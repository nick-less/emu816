#include <stdlib.h>

void start(void);
void print(char *str);
void printLong(long l);
void printInt(unsigned int l);

#define VID ((unsigned char*)0x0400)
#define COLOR ((unsigned char*)0xD800)

#define SPI ((unsigned char*)0xDE00)
#define SPIDATA SPI
#define SPISTAT (SPI+1)
#define SPIDIV  (SPI+2)
#define SPISEL  (SPI+3)
#define SLAVE_SEL 0x04

static char x=0, y=0;

void (*intrVec) (void);


void select() {
    *(SPISEL) = SLAVE_SEL;
}

void unselect(void) {
    *(SPISEL) = 0;
}

int readSPI(long ofs, char copy) {
    int len, i;
    long start;
    unsigned char *dest;
    char c;
    print(" OFS ");printLong(ofs);
    select();
    *SPI = 0x3;
    *SPI = (unsigned char)(ofs >>16);
    *SPI = (unsigned char)(ofs >>8);
    *SPI = (unsigned char)(ofs & 0xff);

    c = *SPI;
    start = (long)c<<16;
    print(" START ");printLong(start);

    c = *SPI;
    start = start | ((long)c<<8);
        print(" START ");printLong(start);

    c = *SPI;
    start = start | c;
    print(" START ");printLong(start);

    len = *SPI;
    len = len <<8;
    len = len | *SPI;
    print(" LEN ");printInt(len);
    if (copy) {
        dest = (unsigned char *) (start & 0xffff);
        for (i=0;i<len;i++) {
            *dest++ = *SPI;
        }
    }
 
    unselect();
    return len+5;
}


void print(char *str) {
    char *v = VID;
    while (*str != 0) {
        if (*str == '\r') {
            x=0;y++;
        } else {
            v[x+(y*40)] = *(str);
            x++;
            if (x>40) {
                x =0 ;y++;
            }
        }
        str++;
    }
}

void printLong(long l) {
    char buf [20];
    ltoa (l, buf,10);
    print(buf);
}

void printInt(unsigned int l) {
    char buf [20];
    itoa (l, buf,16);
    print(buf);
}



void clearScreen(void) {
    int i;
     for (i=0;i<1000;i++) {
            VID[i] = 0x20;
            COLOR[i] = 1;
        }
}


void intr(void) {
    unsigned char * keyCntPtr = ( unsigned char * ) 0x6c;
    unsigned char * keyBufPtr = ( unsigned char * ) 0x277;
   // *VID = *VID +1;
    intrVec();

}


void main(void) {
    long start =  0x0100;
    unsigned int tmp;
    clearScreen();
    print("READING (LOADER) ");
    start = start + (long)readSPI(start, 0);
    print(" N ");
    printLong(start);
    print(" \r");

    print("READING (BASIC)");
   start = start + readSPI(start, 1);
    print(" N ");
    printLong(start);
    print(" BYTES\r");

    print("READING (KERNAL)");
   start = start + readSPI(start, 1);
    print(" N ");
    printLong(start);
    print(" BYTES\r");
    /*
    intrVec = *((unsigned int *)  0xfffe);
printInt(intrVec);
    print(" \r");
    *((unsigned int *)0xfffe) = &intr;
    tmp = *((unsigned int *)  0xfffe);
printInt(tmp);
    print(" \r");
    */
        __asm__ ("jmp ($FFFC)");


    while(1);
 

}