#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include "serialAdapter/serialadapter.h"
#include "helperObjects/channel/channel.h"
#include "helperObjects/mathComponent/mathchannelcomponent.h"
#include "helperObjects/dataMultiplexer/datamultiplexer.h"
#include <QLabel>
#include <vector>

#include <QTimer>
#include <QCheckBox>
#include <QSettings>

namespace Ui {
class MainWindow;
}

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static void clearLayout(QLayout* layout, bool deleteWidgets = true);

public Q_SLOTS:
    void logLine(const QString &line);
    void toggleConnection();
    void refreshUI();

public slots:
    void UpdateAvailMathCh();
    void on_delete_updateMathComp(uint8_t id);

private slots:
    void on_channelNumber_valueChanged(int arg1);

    void on_addMathComp_clicked();

    void on_add3D_clicked();

    void on_addScatter_clicked();

    void on_fileloggingEnabled_stateChanged(int arg1);

    void on_logfilePathDialog_clicked();

    void on_addLine_clicked();

private:
    void LoadSettings();

    void RegisterMathChannel(uint8_t chID);

    Ui::MainWindow *ui;
    SerialAdapter *dataAdapter;

    QSettings   *settings;
    QTimer      *mainTimer;
    DataMultiplexer *mux;

    //  Dynamically created/destroyed UI elements
    std::vector<Channel*>ch;
    std::vector<UIMathChannelComponent*>mathComp;

    std::vector<QCheckBox*>mathChEnabled;
    std::vector<QLineEdit*>mathChName;
    std::vector<Q3DScatter*>plots;
};

#endif // MAINWINDOW_H
