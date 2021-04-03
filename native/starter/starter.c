#include <stdlib.h>

void start(void);
void print(char *str);
void printLong(long l);
void printInt(unsigned int l);

#define VID ((unsigned char*)0x8000)
#define COLOR ((unsigned char*)0x9000)
#define SCREEN_SIZE 2000

#define SPI ((unsigned char*)0xFF80)
#define SPIDATA SPI
#define SPISTAT (SPI+1)
#define SPIDIV  (SPI+2)
#define SPISEL  (SPI+3)
#define SLAVE_SEL_2 0x04
#define SLAVE_SEL_1 0x02


static char x=0, y=0;

void (*intrVec) (void);


void select(unsigned char sel) {
    *(SPISEL) = sel;
}

void unselect(void) {
    *(SPISEL) = 0;
}

int readSPI(long ofs, char copy) {
    unsigned int len, i;
    long start;
    unsigned char *dest;
    char c;
    print(" OFS ");printLong(ofs);
    select(SLAVE_SEL_2);
    *SPI = 0x3;
    *SPI = (unsigned char)(ofs >>16);
    *SPI = (unsigned char)(ofs >>8);
    *SPI = (unsigned char)(ofs & 0xff);

    c = *SPI;
    start = (long)c<<16;

    c = *SPI;
    start = start | ((long)c<<8);

    c = *SPI;
    start = start | c;

    len = *SPI;
    len = len <<8;
    len = len | *SPI;
    if (copy) {
        dest = (unsigned char *) (start & 0xffff);
        for (i=0;i<len;i++) {
            *dest++ = *SPI;
            /*
            if ((unsigned int)dest > 0xff00) {
                x=0;
                y=10;
                printInt((unsigned int)dest);
                print(" ");
                printInt((unsigned int)*(dest-1));

            }
            */
        }
    }

    unselect();

    print(" START ");printLong(start);
    print(" LEN ");printInt(len);


    return len+5;
}

void print (char *str) {
    select(SLAVE_SEL_1);
    while (*str != 0) {
        *SPI = *str++;
    }
    unselect();
}

void print_vid(char *str) {
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
     for (i=0;i<SCREEN_SIZE;i++) {
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
    long start =  0x0;
    unsigned int tmp;
    clearScreen();
    print("SKIP (STARTER) ");
    start = start + (long)readSPI(start, 0);
    print(" N ");
    printLong(start);
    print(" \r");

    print("READING (BASIC)");
   start = start + readSPI(start, 1);
    print(" N ");
    printLong(start);
    print(" BYTES\r");
/*
    print("READING (Editor)");
   start = start + readSPI(start, 1);
    print(" N ");
    printLong(start);
    print(" BYTES\r");

    print("READING (KERNAL)");
   start = start + readSPI(start, 1);
    print(" N ");
    printLong(start);
    print(" BYTES\r");
*/
    /*
    intrVec = *((unsigned int *)  0xfffe);
printInt(intrVec);
    print(" \r");
    *((unsigned int *)0xfffe) = &intr;
    tmp = *((unsigned int *)  0xfffe);
printInt(tmp);
    print(" \r");
    */
        // store a value to 0xffc0 to disable boot rom
        __asm__ ("STA $FFC0");
        // restore stacj
//        __asm__ ("LDX #$FF");
//        __asm__ ("TXS");
        // jump to reset routine
        __asm__ ("jmp $f2b5");


    while(1);
 

}