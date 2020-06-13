#include "spicsimlink.h"
#include "mainwindow.h"
#include <QApplication>
#include <iostream>
#include <cmath>

struct args_info args_info;
ssd1306_t oled;

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_Use96Dpi);
    QApplication a(argc, argv);
    QFont font = qApp->font();
    font.setPixelSize(12);
    qApp->setFont(font);
    MainWindow w;
    w.show();

    return a.exec();
}

// Stubs for testing

void button_set(enum BUTTON btn, enum BUTTONSTATE state) {
    std::cout << "Button " << btn << " has state " << state << std::endl;
}

void adc_set(enum ADC adc, voltage_t volt){
     std::cout << "ADC" << adc << " has " << volt << " mV" << std::endl;
}

bool vcd_start() {
    std::cout << "VCD starts" << std::endl;
    return true;
}

void vcd_stop() {
    std::cout << "VCD stops" << std::endl;
}

bool vcd_init() {
    std::cout << "VCD initializing" << std::endl;
    return true;
}

double led_lightness(enum LED led){
    double x = (led * 1337.0) / 13.0;
    return x - floor(x);
}

cycles_t spicboard_cycles() {
    return 23;
}

int spicboard_state() {
    return 7;
}

const char * spicboard_state_string(){
    return "Simulation";
}

void spicboard_pause(){
    std::cout << "Pause SPiCboard simulation" << std::endl;
}

bool spicboard_is_paused(){
    return false;
}

void spicboard_step(){
    std::cout << "Step SPiCboard simulation" << std::endl;
}

void spicboard_run(){
    std::cout << "Run SPiCboard simulation" << std::endl;
}

void spicboard_reset(){
    std::cout << "Reset SPiCboard simulation" << std::endl;
}

bool spicboard_stop(){
    std::cout << "Stop SPiCboard simulation" << std::endl;
    return true;
}

void spicboard_exit(){
    std::cout << "Exit SPiCboard simulation" << std::endl;
}

const char * spicboard_filepath() {
    return "/tmp/foo.elf";
}

bool spicboard_load(const char * fname){
    std::cout << "Loading " << fname << std::endl;
    return true;
}
