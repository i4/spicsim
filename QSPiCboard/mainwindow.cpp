#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "spicsimlink.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <iostream>

MainWindow::MainWindow(QWidget *parent, int poti, int photo, bool display, bool advanced, bool vcdrecord) :
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

    ui->valPoti->setValue(poti);
    ui->adcPoti->setValue(poti);

    ui->valPhoto->setValue(photo);
    ui->adcPhoto->setValue(photo);

    if (advanced)
        ui->actionAdvanced->toggle();
    if (vcdrecord)
        ui->actionvcdrecord->toggle();

    if (args_info.display_given)
        ui->actiondisplay->toggle();

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(on_update()));
    timer->start(1000 / (args_info.refresh_arg > 1 ? args_info.refresh_arg : 1));
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_update() {
    ui->led0->setLightness(led_lightness(LED_RED0));
    ui->led1->setLightness(led_lightness(LED_YELLOW0));
    ui->led2->setLightness(led_lightness(LED_GREEN0));
    ui->led3->setLightness(led_lightness(LED_BLUE0));
    ui->led4->setLightness(led_lightness(LED_RED1));
    ui->led5->setLightness(led_lightness(LED_YELLOW1));
    ui->led6->setLightness(led_lightness(LED_GREEN1));
    ui->led7->setLightness(led_lightness(LED_BLUE1));

    ui->ledUser->setLightness(led_lightness(LED_USER));

    ui->seg0->setSegment(QSevenSeg::SEGMENT_0, led_lightness(LED_7SEG_0_0));
    ui->seg0->setSegment(QSevenSeg::SEGMENT_1, led_lightness(LED_7SEG_0_1));
    ui->seg0->setSegment(QSevenSeg::SEGMENT_2, led_lightness(LED_7SEG_0_2));
    ui->seg0->setSegment(QSevenSeg::SEGMENT_3, led_lightness(LED_7SEG_0_3));
    ui->seg0->setSegment(QSevenSeg::SEGMENT_4, led_lightness(LED_7SEG_0_4));
    ui->seg0->setSegment(QSevenSeg::SEGMENT_5, led_lightness(LED_7SEG_0_5));
    ui->seg0->setSegment(QSevenSeg::SEGMENT_6, led_lightness(LED_7SEG_0_6));

    ui->seg1->setSegment(QSevenSeg::SEGMENT_0, led_lightness(LED_7SEG_1_0));
    ui->seg1->setSegment(QSevenSeg::SEGMENT_1, led_lightness(LED_7SEG_1_1));
    ui->seg1->setSegment(QSevenSeg::SEGMENT_2, led_lightness(LED_7SEG_1_2));
    ui->seg1->setSegment(QSevenSeg::SEGMENT_3, led_lightness(LED_7SEG_1_3));
    ui->seg1->setSegment(QSevenSeg::SEGMENT_4, led_lightness(LED_7SEG_1_4));
    ui->seg1->setSegment(QSevenSeg::SEGMENT_5, led_lightness(LED_7SEG_1_5));
    ui->seg1->setSegment(QSevenSeg::SEGMENT_6, led_lightness(LED_7SEG_1_6));

    ui->oledGL->update();
    update();
}

void MainWindow::on_btn0_pressed() {
    if (!btnHold)
        button_set(BUTTON0, BUTTON_PRESSED);
}

void MainWindow::on_btn0_released() {
    if (!btnHold)
        button_set(BUTTON0, BUTTON_RELEASED);
}

void MainWindow::on_btn0_clicked(bool checked) {
    if (btnHold)
        button_set(BUTTON0, checked ? BUTTON_PRESSED : BUTTON_RELEASED);
}

void MainWindow::on_btn1_pressed() {
    if (!btnHold)
        button_set(BUTTON1, BUTTON_PRESSED);
}

void MainWindow::on_btn1_released() {
    if (!btnHold)
        button_set(BUTTON1, BUTTON_RELEASED);
}

void MainWindow::on_btn1_clicked(bool checked) {
    if (btnHold)
        button_set(BUTTON1, checked ? BUTTON_PRESSED : BUTTON_RELEASED);
}

void MainWindow::on_btnUser_pressed() {
    if (!btnHold)
        button_set(BUTTON_USER, BUTTON_PRESSED);
}

void MainWindow::on_btnUser_released() {
    if (!btnHold)
        button_set(BUTTON_USER, BUTTON_RELEASED);
}

void MainWindow::on_btnUser_clicked(bool checked) {
    if (btnHold)
        button_set(BUTTON_USER, checked ? BUTTON_PRESSED : BUTTON_RELEASED);
}


void MainWindow::on_adcPoti_valueChanged(int value) {
    ui->valPoti->setValue(value);
    adc_set(POTI, (voltage_t)value);
}

void MainWindow::on_valPoti_valueChanged(int value) {
    ui->adcPoti->setValue(value);
}

void MainWindow::on_adcPhoto_valueChanged(int value) {
    ui->valPhoto->setValue(value);
    adc_set(PHOTO, (voltage_t)value);
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

