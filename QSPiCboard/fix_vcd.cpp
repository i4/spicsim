#include "fix_vcd.h"

#include <QFile>

int fix_vcd( const QString& path_in, const QString& path_out ) {

    // Buffer for modifying the VCD file
    QString vcd;

    // Read input file
    {

        QFile file_in( path_in );
        if( !file_in.open( QIODevice::ReadOnly | QIODevice::Text ) ) {
            return -1;
        }

        vcd = file_in.readAll();
        if( vcd.size() == 0 ) {
            return -1;
        }

    }

    // Replace faulty initializations by the realistic values
    vcd.replace( "x!", "0!" ); // INT/EXT_INT0
    vcd.replace( "x\"", "0\"" ); // INT/EXT_INT1
    vcd.replace( "x#", "0#" ); // INT/TIMER0_COMPA
    vcd.replace( "x$", "0$" ); // INT/TIMER0_COMPB
    vcd.replace( "x%", "0%" ); // INT/TIMER0_OVF
    vcd.replace( "x&", "1&" ); // LED0-RED0/PD6
    vcd.replace( "x'", "1'" ); // LED0-YELLOW0/PD5
    vcd.replace( "x(", "1(" ); // LED0-GREEN0/PD4
    vcd.replace( "x)", "1)" ); // LED0-BLUE0/PD7
    vcd.replace( "x*", "1*" ); // LED0-RED1/PB0
    vcd.replace( "x+", "1+" ); // LED0-YELLOW1/PB1
    vcd.replace( "x,", "1," ); // LED0-GREEN1/PC3
    vcd.replace( "x-", "1-" ); // LED0-BLUE1/PC2
    vcd.replace( "x.", "1." ); // BTN0/PD2
    vcd.replace( "x/", "1/" ); // BTN1/EXT/PD3
    vcd.replace( "x0", "00" ); // COM-TX/PD1
    vcd.replace( "x1", "01" ); // COM-RX/PD0
    vcd.replace( "bxxxxxxxxxxxx 2", "b001111111111 2" ); // ADC0-POTI/PC0
    vcd.replace( "bxxxxxxxxxxxx 3", "b011111111111 3" ); // ADC1-PHOTO/PC1
    vcd.replace( "bxxxxxxxx 4", "b00000000 4" ); // HC595-MOSI/SPI0
    vcd.replace( "x5", "05" ); // HC595-LATCH/PB2

    // Write to output file
    {

        QFile file_out( path_out );
        if( !file_out.open( QIODevice::WriteOnly | QIODevice::Text ) ) {
            return -1;
        }

        if( (file_out.write( vcd.toUtf8() ) == -1) || (!file_out.flush()) ) {
            return -1;
        }

    }

    // Signal success
    return 0;

}
