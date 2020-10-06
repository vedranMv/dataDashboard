#include "mainwindow.h"
#include "ui_mainwindow.h"


#include "scatter/scatterdatamodifier.h"
#include "orientation_3d/orientationwidget.h"
#include <QApplication>
#include <QLabel>
#include <QSurfaceFormat>
#include <QDateTime>


#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFontComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMessageBox>
#include <QtGui/QScreen>
#include <QtGui/QFontDatabase>

#include <QSerialPortInfo>

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindow)
{
    //  Setup UI made in designer
    ui->setupUi(this);
    //  Collect handles for checkboxes and names for math channels into arrays
    mathChEnabled.push_back(ui->mathCh1en);
    mathChEnabled.push_back(ui->mathCh2en);
    mathChEnabled.push_back(ui->mathCh3en);
    mathChEnabled.push_back(ui->mathCh4en);
    mathChEnabled.push_back(ui->mathCh5en);
    mathChEnabled.push_back(ui->mathCh6en);

    mathChName.push_back(ui->mathCh1name);
    mathChName.push_back(ui->mathCh2name);
    mathChName.push_back(ui->mathCh3name);
    mathChName.push_back(ui->mathCh4name);
    mathChName.push_back(ui->mathCh5name);
    mathChName.push_back(ui->mathCh6name);

    //  Initialize the variables
    dataAdapter = new SerialAdapter();
    mainTimer = new QTimer();

    LogLine("Starting up..");

    //  Create magnetometer scatter plot
    Q3DScatter *graph = new Q3DScatter();
    QWidget *container = QWidget::createWindowContainer(graph);

    //  Check that OpenGL exists
    if (!graph->hasContext()) {
        QMessageBox msgBox;
        msgBox.setText("Couldn't initialize the OpenGL context.");
        msgBox.exec();
        return;
    }
    //  Configure size policy for the magnetometer data
    QSize screenSize = graph->screen()->size();
    container->setMinimumSize(QSize(400,400));
    container->setMaximumSize(screenSize/4);
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    //  Create 3D orientation widget
    ui->orientation_3d->setMinimumSize(QSize(400,400));
    ui->orientation_3d->setMaximumSize(screenSize);
    ui->orientation_3d->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->orientation_3d->setFocusPolicy(Qt::StrongFocus);

    //  Add second vertical layout with scatter plot, and all plot options
    ui->vLayout->addWidget(new QLabel(QStringLiteral("Magnetometer")));
    ui->vLayout->addWidget(container, 0, Qt::AlignTop);

    //  Pushes data into the magnetometer scatter plot
    ScatterDataModifier *modifier = new ScatterDataModifier(graph);

    /**
     * Configure parameters for serial port
     */
    // Port selector
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos)
        ui->portSelector->addItem(info.portName());
    //  Port baud
    ui->portBaud->addItem("1000000");
    ui->portBaud->addItem("115200");
    ui->portBaud->addItem("9600");
    ui->portBaud->setCurrentIndex(1);

    // 'Connect' button opens serial port connection
    QObject::connect(ui->connectButton, &QPushButton::clicked, this, &MainWindow::toggleConnection);
    //  Serial adapter objects logs data into a MainWindow logger
    QObject::connect(dataAdapter, &SerialAdapter::error, this, &MainWindow::LogLine);

    //  Connect channel enable signals to slot
    for (uint8_t i = 0; i < mathChEnabled.size(); i++)
        QObject::connect(mathChEnabled[i], &QCheckBox::clicked, this, &MainWindow::UpdateAvailMathCh);

    //TODO: Add data bits, parity and flow control fields
    //  For now assume 8bits, no parity, no flow control, 1 stop bit
    //  Connect/disconnect button
    LogLine("Start-up completed, loading settings...");
    LoadSettings();

    //  Timer ticks at 1s, refreshing certain UI elements
    mainTimer->setInterval(1000);
    QObject::connect(mainTimer, &QTimer::timeout, this, &MainWindow::refreshUI);
    mainTimer->start();
}

/**
 * @brief Clean up and backup settings on exit
 * When exiting the window, save all settings and clean up
 */
MainWindow::~MainWindow()
{
    //  Save port options
    settings->setValue("port/name", ui->portSelector->itemText(ui->portSelector->currentIndex()));
    settings->setValue("port/baud", ui->portBaud->itemText(ui->portBaud->currentIndex()));


    //  Save channel settings
    for (uint8_t i = 0; i < ch.size(); i++)
    {
        QString chID_str =  QString::number(i);

        settings->setValue("channel/channel"+chID_str+"ID", ch[i]->GetId());
        settings->setValue("channel/channel"+chID_str+"name", ch[i]->GetName());
    }

    //  Save math channels
    for (uint8_t i = 0; i < mathComp.size(); i++)
    {
        QString id_str = QString::number(i);

        settings->setValue("math/component"+id_str+"inCh", mathComp[i]->GetInCh());
        settings->setValue("math/component"+id_str+"mathCh", mathComp[i]->GetMathCh());
        settings->setValue("math/component"+id_str+"math", mathComp[i]->GetMath());
    }
    settings->setValue("math/componentCount", mathComp.size());

    //  Save math channel labels
    for (uint8_t i = 0; i < mathChEnabled.size(); i++)
        if (mathChEnabled[i]->isEnabled())
            settings->setValue("math/channel"+QString::number(i+1)+"name",mathChName[i]->text());


    settings->sync();

    ch.clear();

    delete ui;
}

/**
 * @brief Load settings from the configuration file
 */
void MainWindow::LoadSettings()
{
    settings = new QSettings(QString("config.ini"), QSettings::IniFormat);

    //  Data frame settings
    ui->frameStartCh->setText(settings->value("channel/startChar","").toString());
    ui->frameChSeparator->setText(settings->value("channel/separator","").toString());
    ui->frameEndSep->setText(settings->value("channel/endChar","").toString());
    //  Number of data channels
    ui->channelNumber->setValue(settings->value("channel/numOfChannels","0").toInt());

    //  Read mask of enabled math channels and apply UI changes
    unsigned int mathChMask = settings->value("math/channelMask","0").toUInt();
    for (uint8_t i = 0; i < mathChEnabled.size(); i++)
        if ( mathChMask & (unsigned int)(1<<(i+1)) )
        {
            mathChEnabled[i]->setChecked(true);
        }

    //  Load number of math components
    unsigned int mathComponentCount = settings->value("math/componentCount","0").toUInt();
    for (uint8_t i = 0; i < mathComponentCount; i++)
        on_addMathComp_clicked();
}

/**
 * @brief Trigger refresh of various UI elements
 * Usually called periodically every 1s from a main timer
 */
void MainWindow::refreshUI()
{
    ui->timestamp->setText(QDateTime::currentDateTime().time().toString());
}

/**
 * @brief Toggle a connection to data adapter
 * Turn a connection to the underlying data adapter on or off
 */
void MainWindow::toggleConnection()
{

    if (ui->connectButton->text() == "Connect")
    {
        dataAdapter->updatePort(ui->portSelector->itemText(ui->portSelector->currentIndex()), \
                                ui->portBaud->itemText(ui->portBaud->currentIndex()));
        ui->portSelector->setEnabled(false);
        ui->portBaud->setEnabled(false);
        ui->connectButton->setText("Disconnect");

        dataAdapter->startThread();
        //TODO: How to handle a case where function is called, but results in an error?
    }
    else if (ui->connectButton->text() == "Disconnect")
    {
        ui->portSelector->setEnabled(true);
        ui->portBaud->setEnabled(true);
        ui->connectButton->setText("Connect");

        dataAdapter->stopThread();
    }

}

/**
 * @brief [Slot function] Log a line to UI and text file
 * @param line
 */
void MainWindow::LogLine(const QString &line)
{
    QString time = QDateTime::currentDateTime().time().toString();
    ui->logLine->setText(time + ": " + line);

    //TODO: Log into a text file as well
}


/**
 * @brief [Slot function] Save starting character as soon as it's edited
 */
void MainWindow::on_frameStartCh_editingFinished()
{
    settings->setValue("channel/startChar", ui->frameStartCh->text());
    settings->sync();
}
/**
 * @brief [Slot function] Save channel separator character as soon as it's edited
 */
void MainWindow::on_frameChSeparator_editingFinished()
{
    settings->setValue("channel/separator", ui->frameChSeparator->text());
    settings->sync();
}
/**
 * @brief [Slot function] Save ending character as soon as it's edited
 */
void MainWindow::on_frameEndSep_editingFinished()
{
    settings->setValue("channel/endChar", ui->frameEndSep->text());
    settings->sync();
}

/**
 * @brief [Slot function] Handles dynamic construction of data channels
 * When called, it destroys all existing channel entries, and constructs
 * a number of new ones corresponding to the argument.
 * At the same time, number of channels is saved into a config file,
 * and existing data from config file is used to populate new fields
 * @param arg1
 */
void MainWindow::on_channelNumber_valueChanged(int arg1)
{
    //static
    settings->setValue("channel/numOfChannels", arg1);
    settings->sync();

    //  Clear existing list of channels
    clearLayout(ui->channelList, true);
    //  Clear old list with channel elements
    ch.clear();

    //  Loop through given number of channels
    for (uint8_t i = 0; i < arg1; i++)
    {
        QString chID_str =  QString::number(i);
        QHBoxLayout *entry = new QHBoxLayout();

        //  Extract settings for this channel from config file, if existing
        int chID = settings->value("channel/channel"+chID_str+"ID","0").toInt();
        QString chName = settings->value("channel/channel" + chID_str + "name","").toString();

        //  Construct UI elements for channel configuration
        Channel *tmp = new Channel("Channel " + chID_str, chID, chName);
        ch.push_back(tmp);

        //  Add UI elements to a layout, then push layout into the UI
        entry->addWidget(tmp->chLabel, 0, Qt::AlignLeft);
        entry->addWidget(tmp->channelId, 0, Qt::AlignLeft);
        entry->addWidget(tmp->channelName, 0, Qt::AlignLeft);
        ui->channelList->addLayout(entry);
    }
}

/**
 * @brief Delete all elements in a layout provided in arguments
 * @param layout Layout to delete objects from
 * @param deleteWidgets If true, delete widgets
 */
void MainWindow::clearLayout(QLayout* layout, bool deleteWidgets)
{
    while (QLayoutItem* item = layout->takeAt(0))
    {
        if (deleteWidgets)
        {
            if (QWidget* widget = item->widget())
                widget->deleteLater();
        }
        if (QLayout* childLayout = item->layout())
            clearLayout(childLayout, deleteWidgets);
        delete item;
    }
}

///////////////////////////////////////////////////////////////////////////////
////
///     Math components UI manipulations
///
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief [Slot function] Add new math component to the scroll list
 */
void MainWindow::on_addMathComp_clicked()
{
    //  Convert current id to string for easier manipulation
    QString id_str = QString::number(mathComp.size());
    //  Construct component for channel math and push it in the vector
    MathChannelComponent *tmp = new MathChannelComponent((uint8_t)mathComp.size());
    mathComp.push_back(tmp);

    //  Add new component to the UI
    ui->mathCompLayout->addLayout(tmp->GetLayout());
    //  Update available math channels in the component
    UpdateAvailMathCh();
    //  Set values from settings, if exist, otherwise load defaults
    tmp->SetInCh(settings->value("math/component"+id_str+"inCh","0").toInt());
    tmp->SetMathCh(settings->value("math/component"+id_str+"mathCh","1").toInt());
    tmp->SetMath(settings->value("math/component"+id_str+"math","0").toInt());
    //  0 - Add
    //  1 - Subtract

    connect(tmp, &MathChannelComponent::deleteRequested, this, &MainWindow::on_delete_updateMathComp);
}

/**
 * @brief [Slot function] Update a list of available math channels used by
 *      math channel entries
 */
void MainWindow::UpdateAvailMathCh()
{
    //  List of channels currently enabled, to be passed to QComboBoxes
    //  in math components list
    int mathCh[6] = {0};
    int count = 0;

    //  Collapse all enabled channels into a binary mask,
    //  save mask into a config file
    unsigned int mathChMask = 0;

    //  Loop through all QCheckBox elements and configure UI look based on
    //  whether they are checked or not
    for (uint8_t i = 0; i < mathChEnabled.size(); i++)
    {
        QString id_str = QString::number(i+1);
        if (mathChEnabled[i]->isChecked())
        {
            mathCh[count++] = (i+1);
            mathChMask |= 1 << (i+1);
            mathChName[i]->setEnabled(true);
            //  Load channel name from settings, if it exists
            mathChName[i]->setText(settings->value("math/channel"+id_str+"name","Math "+id_str).toString());
        }
        else
            mathChName[i]->setEnabled(false);
    }

    //  Save channel mask
    settings->setValue("math/channelMask", mathChMask);
    settings->sync();

    //  Go through existing math components list and update QComboBox with new
    //  available math channels
    for (MathChannelComponent* X : mathComp)
        X->UpdateMathCh(mathCh, count);

}

/**
 * @brief [Slot function] Called by MathChannelComponent class when the delete
 *  button has been pressed. It handles deletion in UI and clean up in backend
 * @param id ID of MathChannelComponent::_id to be deleted
 */
void MainWindow::on_delete_updateMathComp(uint8_t id)
{
    //  Math component got destroyed

    //  Find component in vector
    uint8_t i;
    for (i = 0; i < mathComp.size(); i++)
        if (mathComp[i]->GetID() == id)
            break;
    //  Clear UI elements
    clearLayout(mathComp[i]->GetLayout());
    //  Delete remainder
    delete mathComp[i];
    //  Remove the entry from vector
    mathComp.erase(mathComp.begin()+i);

    //  Update IDs of entries in the vector
    for (i = 0; i < mathComp.size(); i++)
        mathComp[i]->SetID(i);
}

///////////////////////////////////////////////////////////////////////////////
////
///     Dynamic creation of graphs
///
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Create new 3D orientation graph
 */
void MainWindow::on_add3D_clicked()
{
    OrientationWidget *tmp = new OrientationWidget();
    tmp->setMinimumSize(QSize(200,200));
    tmp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    ui->mdiArea->addSubWindow(tmp);
    tmp->parentWidget()->setWindowFlags(Qt::WindowCloseButtonHint);
    tmp->parentWidget()->setAttribute(Qt::WA_DeleteOnClose, true);

    tmp->parentWidget()->show();
}
