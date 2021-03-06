#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "spicsimlink.h"

#include <QDesktopServices>
#include <QImage>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QProcess>
#include <iostream>

enum SKINS {
    SKIN_SPICBOARD = 0,
    SKIN_COFFEE = 1,
    SKIN_TRAFFICLIGHT = 2
};

QString MainWindow::getSaveFileName(const QString & title, const std::map<QString,QString> & filters, const QString defaultSuffix, const QString target){
    QString selectedFilter, filterList;
    if (!filters.empty()) {
        for (auto const& f : filters){
            if (!filterList.isEmpty())
                filterList.append(";;");
            filterList.append(f.second);
            if (f.first == defaultSuffix)
                selectedFilter = f.second;
        }
    }
    QString fileName = QFileDialog::getSaveFileName(this, title, target, filterList, &selectedFilter);
    if (fileName != nullptr){
        QFileInfo fileInfo(fileName);
        if (fileInfo.suffix() == ""){
            for (auto const& f : filters){
                if (selectedFilter == f.second){
                    fileName.append(f.first);
                    break;
                }
            }
        }
    }
    return fileName;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    btnHold(false),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->mainToolBar->insertWidget(ui->actionhelp, spacer);

    // Always on top
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    // Skin SPiCboard
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

    ui->valPoti->setValue(args_info.poti_value_arg);
    ui->adcPoti->setValue(5000 - args_info.poti_value_arg);

    ui->valPhoto->setValue(args_info.photo_value_arg);
    ui->adcPhoto->setValue(args_info.photo_value_arg);


    if (args_info.advanced_given)
        ui->actionAdvanced->toggle();
    if (args_info.vcd_given)
        ui->actionvcdrecord->toggle();

    if (args_info.display_given)
        ui->actiondisplay->toggle();

    // Skin Coffee Machine
    ui->coffeeHeat->setImage(":/skins/coffee-heat.svg");
    ui->coffeeStatusStandby->setColor(QLED::BLUE);
    ui->coffeeStatusStandby->setSize(10);
    ui->coffeeStatusActive->setColor(QLED::GREEN);
    ui->coffeeStatusActive->setSize(10);
    ui->coffeeStatusWater->setColor(QLED::RED);
    ui->coffeeStatusWater->setSize(10);

    // Skin Traffic Light
    ui->trafficLightCarRed0->setColor(QLED::RED);
    ui->trafficLightCarRed1->setColor(QLED::RED);
    ui->trafficLightCarYellow0->setColor(QLED::YELLOW);
    ui->trafficLightCarYellow1->setColor(QLED::YELLOW);
    ui->trafficLightCarGreen0->setColor(QLED::GREEN);
    ui->trafficLightCarGreen1->setColor(QLED::GREEN);

    ui->trafficLightPedRed->setColor(QLED::RED);
    ui->trafficLightPedGreen->setColor(QLED::GREEN);
    ui->trafficLightPedBlue->setColor(QLED::BLUE);
    ui->trafficLightPedBlue->setSize(10);

    ui->trafficLightSeg0->setSize(1, 0, false);
    ui->trafficLightSeg1->setSize(1, 0, false);

    // Status Bar
    QFont font = qApp->font();
    font.setPixelSize(9);
    font.setItalic(true);
    ui->statusBar->setFont(font);

    // General
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(on_update()));
    timer->start(1000 / (args_info.refresh_arg > 1 ? args_info.refresh_arg : 1));

    ui->actionvcdshow->setEnabled(false);
    ui->actionSaveVCD->setEnabled(false);
    if (args_info.vcd_file_given){
        vcd_file = args_info.vcd_file_arg;
    } else {
        QFileInfo fileInfo(spicboard_filepath());
        vcd_file = fileInfo.canonicalPath() + "/" + fileInfo.baseName() + ".vcd";
    }
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_update() {

    switch (ui->skin->currentIndex()){
        case SKIN_TRAFFICLIGHT:
            {
                double carRed = led_lightness(LED_RED0);
                ui->trafficLightCarRed0->setLightness(carRed);
                ui->trafficLightCarRed1->setLightness(carRed);
                double carYellow = led_lightness(LED_YELLOW0);
                ui->trafficLightCarYellow0->setLightness(carYellow);
                ui->trafficLightCarYellow1->setLightness(carYellow);
                double carGreen = led_lightness(LED_GREEN0);
                ui->trafficLightCarGreen0->setLightness(carGreen);
                ui->trafficLightCarGreen1->setLightness(carGreen);

                ui->trafficLightPedRed->setLightness(led_lightness(LED_RED1));
                ui->trafficLightPedGreen->setLightness(led_lightness(LED_GREEN1));
                ui->trafficLightPedBlue->setLightness(led_lightness(LED_BLUE1));

                ui->trafficLightSeg0->setSegment(QSevenSeg::SEGMENT_0, led_lightness(LED_7SEG_0_0));
                ui->trafficLightSeg0->setSegment(QSevenSeg::SEGMENT_1, led_lightness(LED_7SEG_0_1));
                ui->trafficLightSeg0->setSegment(QSevenSeg::SEGMENT_2, led_lightness(LED_7SEG_0_2));
                ui->trafficLightSeg0->setSegment(QSevenSeg::SEGMENT_3, led_lightness(LED_7SEG_0_3));
                ui->trafficLightSeg0->setSegment(QSevenSeg::SEGMENT_4, led_lightness(LED_7SEG_0_4));
                ui->trafficLightSeg0->setSegment(QSevenSeg::SEGMENT_5, led_lightness(LED_7SEG_0_5));
                ui->trafficLightSeg0->setSegment(QSevenSeg::SEGMENT_6, led_lightness(LED_7SEG_0_6));

                ui->trafficLightSeg1->setSegment(QSevenSeg::SEGMENT_0, led_lightness(LED_7SEG_1_0));
                ui->trafficLightSeg1->setSegment(QSevenSeg::SEGMENT_1, led_lightness(LED_7SEG_1_1));
                ui->trafficLightSeg1->setSegment(QSevenSeg::SEGMENT_2, led_lightness(LED_7SEG_1_2));
                ui->trafficLightSeg1->setSegment(QSevenSeg::SEGMENT_3, led_lightness(LED_7SEG_1_3));
                ui->trafficLightSeg1->setSegment(QSevenSeg::SEGMENT_4, led_lightness(LED_7SEG_1_4));
                ui->trafficLightSeg1->setSegment(QSevenSeg::SEGMENT_5, led_lightness(LED_7SEG_1_5));
                ui->trafficLightSeg1->setSegment(QSevenSeg::SEGMENT_6, led_lightness(LED_7SEG_1_6));
            }
            break;

        case SKIN_COFFEE:
            ui->coffeeStatusWater->setLightness(led_lightness(LED_RED0));
            ui->coffeeHeat->setLightness(led_lightness(LED_YELLOW0));
            ui->coffeeStatusActive->setLightness(led_lightness(LED_GREEN0));
            ui->coffeeStatusStandby->setLightness(led_lightness(LED_BLUE0));
            break;

        default: // SKIN_SPICBOARD
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
    }

    ui->actionpause->setChecked(spicboard_is_paused());
    ui->statusBar->showMessage(spicboard_state_string());
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
    ui->valPoti->setValue(5000 - value);
    adc_set(POTI, static_cast<voltage_t>(5000 - value));
}

void MainWindow::on_valPoti_valueChanged(int value) {
    ui->adcPoti->setValue(5000 - value);
}

void MainWindow::on_adcPhoto_valueChanged(int value) {
    ui->valPhoto->setValue(value);
    adc_set(PHOTO, static_cast<voltage_t>(value));
}

void MainWindow::on_valPhoto_valueChanged(int value) {
    ui->adcPhoto->setValue(value);
}

void MainWindow::on_btnHold_clicked(bool checked) {
    btnHold = checked;
    ui->btnUser->setCheckable(checked);
    ui->btn0->setCheckable(checked);
    ui->btn1->setCheckable(checked);
}

void MainWindow::on_actionAdvanced_toggled(bool value) {
    if (value){
        ui->advanced->setEnabled(true);
        ui->advanced->show();
    } else {
        ui->advanced->hide();
        ui->advanced->setEnabled(false);
        btnHold = false;
        ui->btnHold->setChecked(false);
        ui->btnUser->setChecked(false);
        ui->btn0->setChecked(false);
        ui->btn1->setChecked(false);
        ui->btnUser->setCheckable(false);
        ui->btn0->setCheckable(false);
        ui->btn1->setCheckable(false);
    }
}

void MainWindow::on_actiondisplay_toggled(bool value) {
    if (value){
        ui->oledScreenshot->setEnabled(true);
        ui->oled->setEnabled(true);
        ui->oled->show();
    } else {
        ui->oled->hide();
        ui->oled->setEnabled(false);
        ui->oledScreenshot->setEnabled(false);
    }
}

void MainWindow::on_actionload_triggered() {
    QFileInfo fileInfoPrev(spicboard_filepath());
    QFileInfo fileInfo(QFileDialog::getOpenFileName(this, tr("Open Executable"), fileInfoPrev.canonicalPath(), tr("Executables (*.elf *.hex)")));
    if (fileInfo.exists() && fileInfo.isReadable()) {
        spicboard_exit();
        if (!spicboard_load(fileInfo.absoluteFilePath().toStdString().c_str())){
            QMessageBox::warning(this, "Loading Firmware File", "Error while loading firmware file!");
        }
    }
}

void MainWindow::on_actionvcdshow_triggered() {
    QFileInfo fileInfoTmp(args_info.vcd_file_arg);
    if (fileInfoTmp.exists()){
        QStringList arguments;
        arguments << fileInfoTmp.absoluteFilePath();
        arguments << "--rcvar" << "do_initial_zoom_fit 1";

        QProcess process;
        process.setProgram("gtkwave");
        process.setArguments(arguments);
        process.startDetached();
    } else {
        QMessageBox::warning(this, "Value Change Dump View", "No (temporary) value change dump file available!");
    }
}

void MainWindow::on_actionSaveVCD_triggered() {
    std::map<QString,QString> filters={
        { ".vcd", "Value Change Dump (*.vcd)"}
    };
    QFileInfo fileInfoSrc(args_info.vcd_file_arg);
    if (fileInfoSrc.exists()) {
        vcd_file = getSaveFileName("Save SPiCboard Value Change Dump", filters, ".vcd", vcd_file);
        QFileInfo fileInfoDest(vcd_file);
        if (vcd_file != nullptr && vcd_file != args_info.vcd_file_arg){
            QFile::copy(args_info.vcd_file_arg, vcd_file);
        }
    } else {
        QMessageBox::warning(this, "Value Change Dump File", "No temporary file created!");
    }
}

void MainWindow::on_actionhelp_triggered() {
     QDesktopServices::openUrl(QUrl("https://www4.cs.fau.de/Lehre/current/V_SPIC/SPiCboard/spicsim.shtml"));
}

void MainWindow::on_actionexit_triggered() {
    spicboard_exit();
    close();
}

void MainWindow::on_actionreset_triggered() {
    spicboard_reset();
}

void MainWindow::on_oledScreenshot_clicked() {
    std::map<QString,QString> filters={
        { ".png", "PNG (*.png)"},
        { ".bmp", "BMP (*.bmp)" },
        { ".jpg", "JPEG (*.jpg *.jpeg)" }
    };
    QFileInfo fileInfo(spicboard_filepath());
    QString fileName = getSaveFileName("Save OLED-Display Screenshot", filters, ".png", fileInfo.canonicalPath() + "/" + fileInfo.baseName() + "_oled.png");
    if (fileName != nullptr){
        ui->oledGL->grab().save(fileName);
    }
}

void MainWindow::on_actionScreenshot_triggered() {
    std::map<QString,QString> filters={
        { ".png", "PNG (*.png)"},
        { ".bmp", "BMP (*.bmp)" },
        { ".jpg", "JPEG (*.jpg *.jpeg)" }
    };
    QFileInfo fileInfo(spicboard_filepath());
    QString fileName = getSaveFileName("Save SPiCboard Screenshot", filters, ".png", fileInfo.canonicalPath() + "/" + fileInfo.baseName() + ".png");
    if (fileName != nullptr){
        this->grab().save(fileName);
    }
}

void MainWindow::on_actionpause_triggered(bool checked) {
    if (checked) {
        spicboard_pause();
    } else {
        spicboard_run();
    }
}

void MainWindow::on_actionvcdrecord_triggered(bool checked) {
    if (checked) {
        vcd_init();
        if (!vcd_start()) {
            vcd_stop();
            ui->actionvcdrecord->setChecked(false);
            QMessageBox::warning(this, "Value Change Dump Record", "Failed recording Value Change Dump!");
        }
    } else {
        vcd_stop();
    }
    ui->actionvcdshow->setEnabled(!checked);
    ui->actionSaveVCD->setEnabled(!checked);
}

void MainWindow::on_actionstep_triggered(){
    spicboard_step();
}

void MainWindow::on_skin_currentChanged(int index){
    ui->actionAdvanced->setEnabled(index == SKIN_SPICBOARD);
    ui->actiondisplay->setEnabled(index == SKIN_SPICBOARD);
    if (index != SKIN_SPICBOARD && btnHold){
        btnHold = false;
        ui->btnHold->setChecked(false);
        ui->btnUser->setChecked(false);
        if (ui->btn0->isChecked())
            ui->btn0->released();
        if (ui->btn1->isChecked())
            ui->btn1->released();
    }
    if (index == SKIN_COFFEE){
        ui->coffeeWater->setVisible(true);
        if (!ui->coffeeWaterSwitch->isChecked())
            ui->coffeeWaterSwitch->click();
    }
}

void MainWindow::on_trafficLightBtn0_pressed() {
    button_set(BUTTON0, BUTTON_PRESSED);
}

void MainWindow::on_trafficLightBtn0_released() {
    button_set(BUTTON0, BUTTON_RELEASED);
}

void MainWindow::on_trafficLightBtn1_pressed() {
    button_set(BUTTON1, BUTTON_PRESSED);
}

void MainWindow::on_trafficLightBtn1_released() {
    button_set(BUTTON1, BUTTON_RELEASED);
}

void MainWindow::on_coffeeButton_pressed() {
    button_set(BUTTON0, BUTTON_PRESSED);
}

void MainWindow::on_coffeeButton_released() {
    button_set(BUTTON0, BUTTON_RELEASED);
}

void MainWindow::on_coffeeWaterSwitch_clicked(bool checked) {
    ui->coffeeWater->setVisible(checked);
    button_set(BUTTON1, checked ? BUTTON_RELEASED : BUTTON_PRESSED);
}
