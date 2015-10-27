#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

static int DEBUG = 1;

typedef unsigned int uint;
typedef enum {false, true} bool;

typedef enum {
    CONN_TRANSMITTER,
    CONN_RECEIVER
} ConnectionFlag;

inline void printProgressBar(int a, int b) {
    int barSize = 50;
    float perc = (float)a/b;
    printf("%d/%d [", a, b);
    uint i;
    for (i = 0; i < 50; ++i) {
        if (i < perc*barSize) printf("=");
        else printf(" ");
    }

    printf("] %.0f%%\r", perc*100);
    fflush(stdout);
}

#endif /* UTILS_H */
