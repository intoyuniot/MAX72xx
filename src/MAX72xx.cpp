
//Adapted for IntoRobot by Robin, Sept 19, 2015
#include "MAX72xx.h"

/*************************************************************************************/

//the opcodes for the MAX7221 and MAX7219
#define OP_NOOP   0
#define OP_DIGIT0 1
#define OP_DIGIT1 2
#define OP_DIGIT2 3
#define OP_DIGIT3 4
#define OP_DIGIT4 5
#define OP_DIGIT5 6
#define OP_DIGIT6 7
#define OP_DIGIT7 8
#define OP_DECODEMODE  9
#define OP_INTENSITY   10
#define OP_SCANLIMIT   11
#define OP_SHUTDOWN    12
#define OP_DISPLAYTEST 15

MAX72xx::MAX72xx(int dataPin, int clkPin, int csPin, int numDevices) {
    SPI_MOSI=dataPin;
    SPI_CLK=clkPin;
    SPI_CS=csPin;
    if(numDevices<=0 || numDevices>8 )
        numDevices=8;
    maxDevices=numDevices;
    pinMode(SPI_MOSI,OUTPUT);
    pinMode(SPI_CLK,OUTPUT);
    pinMode(SPI_CS,OUTPUT);
    digitalWrite(SPI_CS,LOW);
    SPI_MOSI=dataPin;
    for(int i=0;i<64;i++)
        status[i]=0x00;
    for(int i=0;i<maxDevices;i++) {
        spiTransfer(i,OP_DISPLAYTEST,0);
        //scanlimit is set to max on startup
        setScanLimit(i,7);
        //decode is done in source
        spiTransfer(i,OP_DECODEMODE,0);
        setIntensity(i,1);
        clearDisplay(i);
        //we go into shutdown-mode on startup
        shutdown(i,true);
    }
}

int MAX72xx::getDeviceCount() {
    return maxDevices;
}

void MAX72xx::shutdown(int addr, bool b) {
    if(addr<0 || addr>=maxDevices)
        return;
    if(b)
        spiTransfer(addr, OP_SHUTDOWN,0);
    else
        spiTransfer(addr, OP_SHUTDOWN,1);
}

void MAX72xx::setScanLimit(int addr, int limit) {
    if(addr<0 || addr>=maxDevices)
        return;
    if(limit>=0 && limit<8)
        spiTransfer(addr, OP_SCANLIMIT,limit);
}

void MAX72xx::setIntensity(int addr, int intensity) {
    if(addr<0 || addr>=maxDevices)
        return;
    if(intensity>=0 && intensity<16)
        spiTransfer(addr, OP_INTENSITY,intensity);

}

void MAX72xx::clearDisplay(int addr) {
    int offset;

    if(addr<0 || addr>=maxDevices)
        return;
    offset=addr*8;
    for(int i=0;i<8;i++) {
        status[offset+i]=0;
        spiTransfer(addr, i+1,status[offset+i]);
    }
}

void MAX72xx::setLed(int addr, int row, int column, bool state) {
    int offset;
    byte val=0x00;

    if(addr<0 || addr>=maxDevices)
        return;
    if(row<0 || row>7 || column<0 || column>7)
        return;
    offset=addr*8;
    val=128 >> column;
    if(state)
        status[offset+row]=status[offset+row]|val;
    else {
        val=~val;
        status[offset+row]=status[offset+row]&val;
    }
    spiTransfer(addr, row+1,status[offset+row]);
}

void MAX72xx::setRow(int addr, int row, byte value) {
    int offset;
    if(addr<0 || addr>=maxDevices)
        return;
    if(row<0 || row>7)
        return;
    offset=addr*8;
    status[offset+row]=value;
    spiTransfer(addr, row+1,status[offset+row]);
}

void MAX72xx::setColumn(int addr, int col, byte value) {
    byte val;

    if(addr<0 || addr>=maxDevices)
        return;
    if(col<0 || col>7)
        return;
    for(int row=0;row<8;row++) {
        val=value >> (7-row);
        val=val & 0x01;
        setLed(addr,row,col,val);
    }
}

void MAX72xx::setDigit(int addr, int digit, byte value, bool dp) {
    int offset;
    byte v;

    if(addr<0 || addr>=maxDevices)
        return;
    if(digit<0 || digit>7 || value>15)
        return;
    offset=addr*8;
    v=charTable[value];
    if(dp)
        v|=128;
    status[offset+digit]=v;
    spiTransfer(addr, digit+1,v);

}

void MAX72xx::setChar(int addr, int digit, char value, bool dp) {
    int offset;
    byte index,v;

    if(addr<0 || addr>=maxDevices)
        return;
    if(digit<0 || digit>7)
        return;
    offset=addr*8;
    index=(byte)value;
    if(index >127) {
        //nothing define we use the space char
        value=32;
    }
    v=charTable[index];
    if(dp)
        v|=128;
    status[offset+digit]=v;
    spiTransfer(addr, digit+1,v);
}

void MAX72xx::spiTransfer(int addr, volatile byte opcode, volatile byte data) {
    //Create an array with the data to shift out
    int offset=addr*2;
    int maxbytes=maxDevices*2;

    for(int i=0;i<maxbytes;i++)
        spidata[i]=(byte)0;
    //put our device data into the array
    spidata[offset+1]=opcode;
    spidata[offset]=data;
    //enable the line
    digitalWrite(SPI_CS,LOW);
    //Now shift out the data
    for(int i=maxbytes;i>0;i--)
        shiftOut(SPI_MOSI,SPI_CLK,MSBFIRST,spidata[i-1]);
    //latch the data onto the display
    digitalWrite(SPI_CS,HIGH);
}

void MAX72xx::setLetter (int addr, const byte c){
    for (byte col = 0; col < 8; col++)
        setColumn(addr, 7-col, cp437_font [c] [col]);
}  // end of letter

void MAX72xx::tweenLetters (int addr, const byte c1, const byte c2, int speed){
    for (byte step = 0; step < 8; step++){
        for (byte col = 0; col < 8; col++){
            if(col<8-step)
                setColumn(addr, 7-col, cp437_font [c1] [col+step]);
            else
                setColumn(addr, 7-col, cp437_font [c2] [col+step-8]);
        }
        delay(speed);
    }

}


/************************************************************************************************/