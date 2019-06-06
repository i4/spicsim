#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->mainToolBar->insertWidget(ui->actionhelp, spacer);

    ui->oled->hide();
    ui->advanced->hide();
    ui->ledUser->setColor(QLED::YELLOW);
    ui->led0->setColor(QLED::RED);
    ui->led1->setColor(QLED::YELLOW);
    ui->led2->setColor(QLED::GREEN);
    ui->led3->setColor(QLED::BLUE);
    ui->led4->setColor(QLED::RED);
    ui->led5->setColor(QLED::YELLOW);
    ui->led6->setColor(QLED::GREEN);
    ui->led7->setColor(QLED::BLUE);
    btnHold = false;
}

MainWindow::~MainWindow() {
    delete ui;
}


void MainWindow::on_btn1_pressed() {
    if (!btnHold)
        std::cout << "BTN1 pressed " << std::endl;
}

void MainWindow::on_btn1_released() {
    if (!btnHold)
        std::cout << "BTN1 released" << std::endl;
}

void MainWindow::on_btn0_pressed() {
    if (!btnHold)
        std::cout << "BTN0 pressed " << std::endl;
}

void MainWindow::on_btn0_released() {
    if (!btnHold)
        std::cout << "BTN0 released" << std::endl;
}

void MainWindow::on_btn0_clicked(bool checked) {
    if (btnHold)
        std::cout << "BTN0 " << checked << std::endl;
}

void MainWindow::on_btn1_clicked(bool checked) {
    if (btnHold)
        std::cout << "BTN1 " << checked << std::endl;
}

void MainWindow::on_valPoti_valueChanged(int value) {
    ui->adcPoti->setValue(value);
}

void MainWindow::on_adcPoti_valueChanged(int value) {
    ui->valPoti->setValue(value);
    ui->led0->setLightness(value * 1.0 / 5000.0);
    ui->led1->setLightness(value * 1.0 / 5000.0);
    ui->led2->setLightness(value * 1.0 / 5000.0);
    ui->led3->setLightness(value * 1.0 / 5000.0);
    ui->led4->setLightness(value * 1.0 / 5000.0);
    ui->led5->setLightness(value * 1.0 / 5000.0);
    ui->led6->setLightness(value * 1.0 / 5000.0);
    ui->led7->setLightness(value * 1.0 / 5000.0);
    ui->seg0->setSegment(QSevenSeg::SEGMENT_4, value * 1.0 / 5000.0);
}

void MainWindow::on_adcPhoto_valueChanged(int value) {
    ui->valPhoto->setValue(value);
}

void MainWindow::on_valPhoto_valueChanged(int value) {
    ui->adcPhoto->setValue(value);
}

void MainWindow::on_actionexit_triggered() {
    close();
}

void MainWindow::on_btnHold_clicked(bool checked) {
    btnHold = checked;
    ui->btnUser->setCheckable(checked);
    ui->btn0->setCheckable(checked);
    ui->btn1->setCheckable(checked);
}

void MainWindow::on_actionAdvanced_toggled(bool value) {
    if (value){
        ui->advanced->show();
        ui->advanced->setEnabled(true);
    } else {
        ui->advanced->setEnabled(false);
        ui->advanced->hide();
        btnHold = false;
        ui->btnHold->setChecked(false);
        ui->btnUser->setChecked(false);
        ui->btn0->setChecked(false);
        ui->btn1->setChecked(false);
        ui->btnUser->setCheckable(false);
        ui->btn0->setCheckable(false);
        ui->btn1->setCheckable(false);
        ui->btnUser->update();
        ui->btn0->update();
        ui->btn1->update();
    }
}

void MainWindow::on_actiondisplay_toggled(bool value) {
    if (value){
        ui->oled->show();
        ui->oled->setEnabled(true);
    } else {
        ui->oled->setEnabled(false);
        ui->oled->hide();
    }
}

void MainWindow::on_actionload_triggered() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Executable"), ".", tr("Executables (*.elf *.hex)"));
    std::cout << "Selected File " <<  fileName.toStdString() << std::endl;

}

void MainWindow::on_actionSaveVCD_triggered() {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Value Change Dump"), ".", tr("Value Change Dumps (*.vcd)"));
    std::cout << "Save as " <<  fileName.toStdString() << std::endl;
}

void MainWindow::on_actionhelp_triggered() {
     QDesktopServices::openUrl(QUrl("https://www4.cs.fau.de/Lehre/current/V_SPIC/SPiCboard/spicsim.shtml"));
}
