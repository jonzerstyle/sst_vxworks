#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/ioctl.h>
#ifndef LINUX
#include "termios.h"
#include <ioLib.h>
#include <fioLib.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

void randomize(void) {
	srand((int)time(NULL));
}


int maxsst(int a, int b) {
	if (a > b) return a;
	return b;
}

int minsst(int a, int b) {
	if (a < b) return a;
	return b;
}

int // <editor-fold defaultstate="collapsed" desc="comment">
getch// </editor-fold>
(void) {
	char chbuf[1];
        int bytes;
        #ifndef LINUX
        ioctl(STD_IN, FIOSETOPTIONS, OPT_RAW);
        ioctl(STD_IN, FIONBIO, 1);
        while (1) { 
                ioctl(STD_IN, FIONREAD, &bytes);
                if (bytes > 0) {
                        fioRead(STD_IN, chbuf, 1);
                        break;
                }
        }
        ioctl(STD_IN, FIOSETOPTIONS, OPT_TERMINAL);
        ioctl(STD_IN, FIONBIO, 0);
        #else
        struct termios oldstate, newstate;
        fflush(stdout);
	    tcgetattr(0, &oldstate);
	    newstate = oldstate;
	    newstate.c_lflag &= ~ICANON;
	    newstate.c_lflag &= ~ECHO;
	    tcsetattr(0, TCSANOW,  &newstate);
        read(0, &chbuf, 1);
	    tcsetattr(0, TCSANOW, &oldstate);
        #endif
        return chbuf[0];
}
