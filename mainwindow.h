#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include "serialAdapter/serialadapter.h"
#include "helperObjects/channel/channel.h"
#include <QLabel>
#include <vector>

#include <QTimer>
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


public Q_SLOTS:
    void LogLine(const QString &line);
    void toggleConnection();
    void refreshUI();


private slots:
    void on_frameStartCh_editingFinished();

    void on_frameChSeparator_editingFinished();

    void on_frameEndSep_editingFinished();

    void on_channelNumber_valueChanged(int arg1);

private:
    void LoadSettings();
    void clearLayout(QLayout* layout, bool deleteWidgets = true);

    Ui::MainWindow *ui;
    SerialAdapter *dataAdapter;

    QSettings   *settings;
    QLabel      *time;
    QTimer      *mainTimer;
    std::vector<Channel*>ch;
};

#endif // MAINWINDOW_H
