#include "mainwindow.h"
#include "ui_mainwindow.h"


#include "scatter/scatterwindow.h"
#include "orientation_3d/orientationwindow.h"
#include "line/lineplot.h"
#include "helperObjects/graphHeaderWidget/graphheaderwidget.h"


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
#include <QMdiSubWindow>
#include <QFileDialog>

#include <QSerialPortInfo>

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindow),
    _pendingDeletion(false),
    _loggingInitialized(false)
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

    //  Initialize objects
    dataAdapter = new SerialAdapter();
    mainTimer = new QTimer();
    mux = DataMultiplexer::GetP();
    settings = new QSettings(QString("config.ini"), QSettings::IniFormat);

    //void response(const QString &s);
    connect(dataAdapter, &SerialAdapter::response, mux, &DataMultiplexer::ReceiveSerialData);
    connect(mux, &DataMultiplexer::logLine, this, &MainWindow::logLine);
    logLine("Starting up..");

    /**
      * Check presence of OpenGL drivers
      */
    Q3DScatter *graph = new Q3DScatter();
    //  Check that OpenGL exists
    if (!graph->hasContext()) {
        QMessageBox msgBox;
        msgBox.setText("Couldn't initialize the OpenGL context.");
        msgBox.exec();
        return;
    }
    delete graph;

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
    QObject::connect(ui->connectButton, &QPushButton::clicked,
                     this, &MainWindow::toggleConnection);
    //  Serial adapter objects logs data into a MainWindow logger
    QObject::connect(dataAdapter, &SerialAdapter::logLine,
                     this, &MainWindow::logLine);

    //  Connect channel enable signals to slot
    for (uint8_t i = 0; i < mathChEnabled.size(); i++)
        QObject::connect(mathChEnabled[i], &QCheckBox::clicked,
                         this, &MainWindow::UpdateAvailMathCh);

    //TODO: Add data bits, parity and flow control fields
    //  For now assume 8bits, no parity, no flow control, 1 stop bit
    //  Connect/disconnect button
    logLine("Start-up completed, loading settings...");
    LoadSettings();

    //  Timer ticks at 1s, refreshing certain UI elements
    mainTimer->setInterval(1000);
    QObject::connect(mainTimer, &QTimer::timeout, this, &MainWindow::refreshUI);
    mainTimer->start();

    dataAdapter->RegisterMux(mux);
}

/**
 * @brief Clean up and backup settings on exit
 * When exiting the window, save all settings and clean up
 */
MainWindow::~MainWindow()
{
    logLine("Deleting main window");
    _pendingDeletion = true;
    //  Save port options
    settings->setValue("port/name",
                       ui->portSelector->itemText(ui->portSelector->currentIndex()));
    settings->setValue("port/baud",
                       ui->portBaud->itemText(ui->portBaud->currentIndex()));

    settings->setValue("channel/startChar", ui->frameStartCh->text());
    settings->setValue("channel/separator", ui->frameChSeparator->text());
    settings->setValue("channel/endChar", ui->frameEndSep->text());
    settings->setValue("channel/endCharR", ui->termSlashR->isChecked());
    settings->setValue("channel/endCharN", ui->termSlashN->isChecked());

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


        settings->setValue("math/component"+id_str+"inCh",
                           mathComp[i]->GetInCh());
        settings->setValue("math/component"+id_str+"mathCh",
                           mathComp[i]->GetMathCh());
        settings->setValue("math/component"+id_str+"math",
                           mathComp[i]->GetMath());
    }
    settings->setValue("math/componentCount", (uint8_t)mathComp.size());

    //  Save math channel labels
    for (uint8_t i = 0; i < mathChEnabled.size(); i++)
        if (mathChEnabled[i]->isChecked())
            settings->setValue("math/channel"+QString::number(i+1)+"name",
                               mathChName[i]->text());

    //  Save file logging settings to file
    settings->setValue("fileLogging/append", ui->appendButton->isChecked());
    settings->setValue("fileLogging/folder", ui->logfilePath->text());
    settings->setValue("fileLogging/fileName", ui->logfileName->text());
    settings->setValue("fileLogging/channelSeparator", ui->logfileChSep->text());

    //  Save currently open page
    settings->setValue("ui/startPage", ui->tabWidget->currentIndex());

    settings->sync();

    for (Channel *X : ch)
        delete X;
    ch.clear();

    for (UIMathChannelComponent *X : mathComp)
        delete X;
    mathComp.clear();

    //  Clean-up for program run log
    _loggingInitialized = false;
    _logFile->close();
    delete _logFileStream;
    _logFile->deleteLater();

    delete ui;
}

/**
 * @brief Load settings from the configuration file
 */
void MainWindow::LoadSettings()
{
    //  Data frame settings
    ui->frameStartCh->setText(
                settings->value("channel/startChar","").toString());
    ui->frameChSeparator->setText(
                settings->value("channel/separator","").toString());
    ui->frameEndSep->setText(
                settings->value("channel/endChar","").toString());
    ui->termSlashN->setChecked(
                settings->value("channel/endCharN","false").toBool());
    ui->termSlashR->setChecked(
                settings->value("channel/endCharR","false").toBool());

    //  Number of data channels
    ui->channelNumber->setValue(
                settings->value("channel/numOfChannels","0").toInt());

    //  Read mask of enabled math channels and apply UI changes
    uint8_t mathChMask = settings->value("math/channelMask","0").toUInt();
    for (uint8_t i = 0; i < mathChEnabled.size(); i++)
        if ( mathChMask & (uint8_t)(1<<(i+1)) )
        {
            mathChEnabled[i]->setChecked(true);
        }

    //  Load number of math components
    uint8_t mathComponentCount = settings->value("math/componentCount","0").toUInt();
    for (uint8_t i = 0; i < mathComponentCount; i++)
        on_addMathComp_clicked();

    //  Load file logging settings
    ui->appendButton->setChecked(
                settings->value("fileLogging/append","true").toBool());
    ui->overwriteButton->setChecked(
                !settings->value("fileLogging/append","true").toBool());
    ui->logfilePath->setText(
                settings->value("fileLogging/folder","").toString());
    ui->logfileName->setText(
                settings->value("fileLogging/fileName","").toString());
    ui->logfileChSep->setText(
                settings->value("fileLogging/channelSeparator",",").toString());

    //  Restore last opened tab
    ui->tabWidget->setCurrentIndex(settings->value("ui/startPage","0").toUInt());
}

/**
 * @brief Trigger refresh of various UI elements
 * Usually called periodically every 1s from a main timer
 */
void MainWindow::refreshUI()
{
    ui->timestamp->setText(QDateTime::currentDateTime().time().toString());

    //  Refresh port selector if nothing is selected
    if (ui->portSelector->currentText() == "")
    {
        const auto infos = QSerialPortInfo::availablePorts();
        for (const QSerialPortInfo &info : infos)
            ui->portSelector->addItem(info.portName());
    }
}

/**
 * @brief Toggle a connection to data adapter
 * Turn a connection to the underlying data adapter on or off
 */
void MainWindow::toggleConnection()
{

    if (ui->connectButton->text() == "Connect")
    {
        //  Update frame information in MUX
        QString termSeq = ui->frameEndSep->text();
        if (ui->termSlashR->isChecked())
            termSeq += QChar(0x000D);
        if (ui->termSlashN->isChecked())
            termSeq += QChar(0x000A);

        mux->SetSerialFrameFormat(ui->frameStartCh->text(), \
                ui->frameChSeparator->text(), \
                termSeq);

        //  Collect channel labels and register them in the mux
        QString *chLabels = new QString[ch.size()];
        for (uint8_t i = 0; i < ch.size(); i++)
        {
            chLabels[i] = ch[i]->GetName();
        }
        mux->RegisterSerialCh(ch.size(), chLabels);
        //  Clean up before exit
        delete[] chLabels;

        //  Configure serial port
        dataAdapter->updatePort(ui->portSelector->itemText(
                                    ui->portSelector->currentIndex()), \
                                ui->portBaud->itemText(ui->portBaud->currentIndex()));
        //  Prevent edits to serial port while connection is open
        ui->serialGroup->setEnabled(false);
        //  Rename the button
        ui->connectButton->setText("Disconnect");

        dataAdapter->startThread();
        //TODO: How to handle a case where function is called, but results in an error?
    }
    else if (ui->connectButton->text() == "Disconnect")
    {
        ui->serialGroup->setEnabled(true);
        ui->connectButton->setText("Connect");

        dataAdapter->stopThread();
    }

}

/**
 * @brief [Slot function] Log a line to UI and an external run log file
 * @param line
 */
void MainWindow::logLine(const QString &line)
{
    QString time = QDateTime::currentDateTime().time().toString();

    //  Handle opening and rotating logs between the program launches. On
    //  every launch, increments the log descriptor and open new logfile to
    //  write to.
    if (!_loggingInitialized && !_pendingDeletion)
    {
        //  Load logfile info
        uint8_t lastLogIndex = settings->value("appLog/index","0").toUInt();
        const uint8_t maxLogIndex = settings->value("appLog/maxIndex","3").toUInt();

        uint8_t currentLogIndex = (lastLogIndex + 1) % maxLogIndex;
        _logFile = new QFile("datadashboard_run"+QString::number(currentLogIndex)+".log");

        if (!_logFile->open(QIODevice::WriteOnly | QIODevice::Text))
        {
            ui->logLine->setText(time + ": " + "Error opening log file");
            return;
        }

        _logFileStream = new QTextStream(_logFile);
        settings->setValue("appLog/index", currentLogIndex);
        _loggingInitialized = true;
    }

    //  If UI has not been deleted, log to UI
    if (!_pendingDeletion)
        ui->logLine->setText(time + ": " + line);

    //  If log file is initialized, append a line in there as well
    if (_loggingInitialized)
        (*_logFileStream) <<  time + ": " + line << Qt::endl;
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
    settings->setValue("channel/numOfChannels", arg1);
    settings->sync();

    logLine("Channel number changed to "\
            +QString::number(arg1)+", reconstructing UI");

    //  Clear existing list of channels
    clearLayout(ui->channelList, true);
    //  Clear old list with channel elements
    ch.clear();

    //  Reconstruct part of UI to offer requested number of channels
    for (uint8_t i = 0; i < arg1; i++)
    {
        QString chID_str =  QString::number(i);
        QHBoxLayout *entry = new QHBoxLayout();

        //  Extract settings for this channel from config file, if existing
        int chID = settings->value("channel/channel"+chID_str+"ID", chID_str).toInt();
        QString chName = settings->value("channel/channel" + chID_str + "name","Serial "+ chID_str).toString();

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
            {
                widget->deleteLater();
            }
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
 * @brief Register a math channel with the given ID in the mux
 * @param chID channel Id to be registered
 */
void MainWindow::RegisterMathChannel(uint8_t chID)
{
    MathChannel *mc = new MathChannel();
    logLine("Attempting to register math channel. Looking up components...");

    mc->SetLabel(mathChName[chID-1]->text());
    //  Look up all the components of this channel ID
    for (UIMathChannelComponent *X : mathComp)
    {
        //  T
        if (X->GetMathCh() == (chID))
        {
            mc->AddComponent(static_cast<MathOperation>(X->GetMath()), X->GetInCh());
        }
    }

    logLine("Registering math channel "+QString::number(chID)+" with "\
            +QString::number(mc->_component.size())+" components");

    mux->RegisterMathChannel(chID, mc);
}

/**
 * @brief [Slot function] Add new math component to the scroll list
 */
void MainWindow::on_addMathComp_clicked()
{
    //  Convert current id to string for easier manipulation
    QString id_str = QString::number(mathComp.size());

    logLine("Adding math component "+id_str);

    //  Construct component for channel math and save it to global variable
    UIMathChannelComponent *tmp = new UIMathChannelComponent((uint8_t)mathComp.size());
    mathComp.push_back(tmp);

    //  Add new component to the UI
    ui->mathCompLayout->addLayout(tmp->GetLayout());
    //  Update available math channels in the component

    logLine("Math component "+id_str+" added to UI");

    UpdateAvailMathCh();
    //  Set values from settings, if exist, otherwise load defaults
    tmp->SetInCh(settings->value("math/component"+id_str+"inCh","0").toInt());
    tmp->SetMathCh(settings->value("math/component"+id_str+"mathCh","1").toInt());
    tmp->SetMath(settings->value("math/component"+id_str+"math","0").toInt());

    connect(tmp, &UIMathChannelComponent::deleteRequested, \
            this, &MainWindow::on_delete_updateMathComp);

    logLine("Saved math component "+id_str);
}

/**
 * @brief [Slot function] Update a list of available math channels used by
 *      other components. Function called whenever a match channel checkbox
 *      has been clicked
 */
void MainWindow::UpdateAvailMathCh()
{
    //  List of channels currently enabled, to be passed to QComboBoxes
    //  in math components list -> not used for now
    int mathCh[6] = {0};
    int count = 0;

    //  Collapse all enabled channels into a binary mask,
    //  save mask into a config file
    static uint8_t  chMask = 0;

    logLine("Updating available math channels in UI...");

    //  Loop through all QCheckBox elements and configure UI look based on
    //  whether they are checked or not
    for (uint8_t i = 0; i < mathChEnabled.size(); i++)
    {
        QString id_str = QString::number(i+1);
        if (mathChEnabled[i]->isChecked())
        {
            mathCh[count++] = (i+1);
            mathChName[i]->setEnabled(true);
            //  Load channel name from settings, if it exists and there
            //  isn't one already set
            if (mathChName[i]->text() == "")
                mathChName[i]->setText(
                            settings->value("math/channel"+id_str+"name",
                                            "Math "+id_str).toString());
            logLine("Channel "+QString::number(i+1)+" enabled in UI");
        }
        else
            mathChName[i]->setEnabled(false);

        //  If it's enabled, but it wasn't in the last call
        if (mathChEnabled[i]->isChecked() && !(((1<<(i+1)) & chMask) > 0))
        {
            logLine("Channel "+QString::number(i+1)+" has been enabled");
            chMask |= (1<<(i+1));
            RegisterMathChannel(i+1);
        }
        //  If it's disabled, but it was enabled in the last call
        else if (!mathChEnabled[i]->isChecked() && (((1<<(i+1)) & chMask) > 0))
        {
            chMask &= ~(1<<(i+1));
            mux->UnegisterMathChannel(i+1);
            logLine("Channel "+QString::number(i+1)+" has been disabled");
        }
    }

    //  Save channel mask
    settings->setValue("math/channelMask", chMask);
    settings->sync();

    //  Go through existing math components list and update QComboBox with new
    //  available math channels
//    for (MathChannelComponent* X : mathComp)
//        X->UpdateMathCh(mathCh, count);

}

/**
 * @brief [Slot function] Called by MathChannelComponent class when the delete
 *  button has been pressed. It handles deletion in UI and clean up in backend
 * @param id ID of MathChannelComponent::_id to be deleted
 */
void MainWindow::on_delete_updateMathComp(uint8_t id)
{
    //  Math component got destroyed
    logLine("Requested to delete math channel "+QString::number(id));
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
 * @brief [Slot] Create new 3D orientation graph
 */
void MainWindow::on_add3D_clicked()
{
    logLine("UI: Creating 3D plot");
    static uint8_t _3DgraphCount = 0;
    QString winID = QString::number(_3DgraphCount);

    OrientationWindow *orient3DWindow = new OrientationWindow(this);
    orient3DWindow->setObjectName("orientationWindow_"+winID);
    QObject::connect(orient3DWindow, &OrientationWindow::logLine,
                     this, &MainWindow::logLine);
    QMdiSubWindow *plotWindow = ui->mdiArea->addSubWindow(orient3DWindow);

    plotWindow->setWindowFlags(Qt::WindowCloseButtonHint);
    plotWindow->setAttribute(Qt::WA_DeleteOnClose, true);
    plotWindow->setWindowTitle("3D Orientation " + winID);

    plotWindow->show();
    _3DgraphCount++;
}

/**
 * @brief [Slot] Create new scatter graph
 */
void MainWindow::on_addScatter_clicked()
{
    logLine("UI: Creating scatter plot");
    static uint8_t _ScatterCount = 0;
    QString winID = QString::number(_ScatterCount);

    //  Create scatter plot
    ScatterWindow *scatterWindow = new ScatterWindow();
    scatterWindow->setObjectName("scatterWindow_"+winID);
    QObject::connect(scatterWindow, &ScatterWindow::logLine,
                     this, &MainWindow::logLine);
    QMdiSubWindow *plotWindow = ui->mdiArea->addSubWindow(scatterWindow);

    plotWindow->setWindowFlags(Qt::WindowCloseButtonHint);
    plotWindow->setAttribute(Qt::WA_DeleteOnClose, true);
    plotWindow->setWindowTitle("Scatter " + winID);

    plotWindow->show();
    _ScatterCount++;
}

/**
 * @brief [Slot] Create new line plot
 */
void MainWindow::on_addLine_clicked()
{
    logLine("UI: Creating line plot");
    static uint8_t _LineCount = 0;
    QString winID = QString::number(_LineCount);

    //  Create line plot
    LinePlot *lineplotWindow = new LinePlot();
    lineplotWindow->setObjectName("lineWindow_"+winID);
    QObject::connect(lineplotWindow, &LinePlot::logLine,
                     this, &MainWindow::logLine);
    QMdiSubWindow *plotWindow = ui->mdiArea->addSubWindow(lineplotWindow);

    plotWindow->setWindowFlags(Qt::WindowCloseButtonHint);
    plotWindow->setAttribute(Qt::WA_DeleteOnClose, true);
    plotWindow->setWindowTitle("Line plot " + winID);

    plotWindow->show();
    _LineCount++;
}


///////////////////////////////////////////////////////////////////////////////
////
///     Logging channel data to file
///
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief [Slot] Open file dialog when 'Path' button has been click
 *      Once the path has been selected, update textbox in UI
 */
void MainWindow::on_logfilePathDialog_clicked()
{
    QString saveDir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                 "/home",
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
    ui->logfilePath->setText(saveDir);
}

/**
 * @brief [Slot] Handle enabling/disabling logging to file
 *      Handles click events to the 'Enable' checkbox which toggles logging to
 *      the file. Updates UI and communicates with the MUX
 * @param arg1  State of the checkbox
 */
void MainWindow::on_fileloggingEnabled_stateChanged(int arg1)
{
    //  Checked can have PartiallyChecked and Checked states, so instead
    //  evaluate if Unchecked
    if (arg1 == Qt::Unchecked)
    {
        ui->overwriteButton->setEnabled(true);
        ui->appendButton->setEnabled(true);
        ui->logfilePathDialog->setEnabled(true);
        ui->logfilePath->setEnabled(true);
        ui->logfileName->setEnabled(true);
        ui->logfileChSep->setEnabled(true);
        //  Disable file logging in mux
        mux->DisableFileLogging();
    }
    else
    {
        ui->overwriteButton->setEnabled(false);
        ui->appendButton->setEnabled(false);
        ui->logfilePathDialog->setEnabled(false);
        ui->logfilePath->setEnabled(false);
        ui->logfileName->setEnabled(false);
        ui->logfileChSep->setEnabled(false);
        //  Save file logging settings to file
        settings->setValue("fileLogging/append",  ui->appendButton->isChecked());
        settings->setValue("fileLogging/folder",  ui->logfilePath->text());
        settings->setValue("fileLogging/fileName",  ui->logfileName->text());
        settings->setValue("fileLogging/channelSeparator",  ui->logfileChSep->text());
        //  Enable file logging in mux
        QString logPath = ui->logfilePath->text()+'/'+ui->logfileName->text();

        int retVal = mux->EnableFileLogging(logPath,
                               ui->appendButton->isChecked(),
                               ui->logfileChSep->text()[0].cell());
        if (retVal != 0)
            ui->fileloggingEnabled->setChecked(false);
    }
}
