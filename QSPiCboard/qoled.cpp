#include "qoled.h"
#include "spicsimlink.h"


QOLED::QOLED(QWidget *parent) : QOpenGLWidget(parent) {

}

QOLED::~QOLED(){

}


void QOLED::initializeGL()
{
    glClearColor(0,0,0,1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
}

void QOLED::paintGL(){

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1, 128, -1, 64, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    ssd1306_set_flag(SSD1306_FLAG_DIRTY, 0);
    const bool vertical = ssd1306_get_flag(SSD1306_FLAG_SEGMENT_REMAP_0) != 0;
    const bool horizontal = ssd1306_get_flag(SSD1306_FLAG_COM_SCAN_NORMAL) == 0;
    const bool invert = ssd1306_get_flag(SSD1306_FLAG_DISPLAY_INVERTED) == 0;
    const bool active = ssd1306_get_flag(SSD1306_FLAG_DISPLAY_ON);
    if (active){
        glColor4d(1.0, 1.0, 1.0, oled.contrast_register / 512.0 + 0.5);
        for (int p = 0; p < ssd1306_pages; p++){
            for (int b = 0; b < 8; b++){
                for (int c = 0; c < ssd1306_cols; c++){
                    if (((oled.vram[p][c] >> b) & 1) == invert){
                        int r = p * 8 + b;
                        int y = horizontal ? (ssd1306_rows - r - 1) : r;
                        int x = vertical ? (ssd1306_cols - c - 1) : c;
                        glRecti(x,y,x+1,y+1);
                    }
                }
            }
        }
    }
}
