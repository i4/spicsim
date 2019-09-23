#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    bool btnHold;
    QString vcd_file;
    QString getSaveFileName(const QString & title, const std::map<QString,QString> & filters, const QString defaultSuffix = nullptr, const QString target = nullptr);

private slots:
    void on_btnHold_clicked(bool checked);
    void on_btn1_pressed();
    void on_btn1_released();
    void on_btn1_clicked(bool checked);
    void on_btn0_pressed();
    void on_btn0_released();
    void on_btn0_clicked(bool checked);
    void on_valPoti_valueChanged(int value);
    void on_adcPoti_valueChanged(int value);
    void on_adcPhoto_valueChanged(int value);
    void on_valPhoto_valueChanged(int value);
    void on_actionexit_triggered();
    void on_actionAdvanced_toggled(bool value);
    void on_actiondisplay_toggled(bool value);
    void on_actionload_triggered();
    void on_actionSaveVCD_triggered();
    void on_actionhelp_triggered();
    void on_update();
    void on_btnUser_pressed();
    void on_btnUser_released();
    void on_btnUser_clicked(bool checked);
    void on_actionreset_triggered();
    void on_oledScreenshot_clicked();
    void on_actionScreenshot_triggered();
    void on_actionvcdshow_triggered();
    void on_actionpause_triggered(bool checked);
    void on_actionvcdrecord_triggered(bool checked);
    void on_actionstep_triggered();
    void on_skin_currentChanged(int index);
    void on_trafficLightBtn0_pressed();
    void on_trafficLightBtn0_released();
    void on_trafficLightBtn1_pressed();
    void on_trafficLightBtn1_released();
    void on_coffeeWaterSwitch_clicked(bool checked);

    void on_coffeeButton_pressed();

    void on_coffeeButton_released();

private:
    Ui::MainWindow *ui;
    QTimer *timer;
};

#endif // MAINWINDOW_H
