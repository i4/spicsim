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
    explicit MainWindow(QWidget *parent = nullptr, int poti = 2000, int photo = 1000, bool display = false, bool advanced = false, bool vcdrecord = false);
    ~MainWindow();

private:
    bool btnHold;

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

private:
    Ui::MainWindow *ui;
    QTimer *timer;
};

#endif // MAINWINDOW_H
