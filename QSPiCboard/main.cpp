#include "spicsimlink.h"
#include "mainwindow.h"
#include <QApplication>
#include <iostream>
#include <cmath>

struct args_info args_info;
ssd1306_t oled;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
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

double led_lightness(enum LED led){
    double x = (led * 1337.0) / 13.0;
    return x - floor(x);
}

cycles_t get_cycles() {
    return 23;
}

int get_cpu_state() {
    return 7;
}

const char * get_cpu_state_string() {
    return "Simulation";
}

bool spicboard_load(char * fname){
    std::cout << "Loading " << fname << std::endl;
}
