#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

extern int DEBUG;
extern int SIMUL_ERROR;
extern int NUM_TRANS;
extern int TIME_TRANS;
extern int MAX_SIZE;

typedef unsigned int uint;
typedef enum {false, true} bool;

typedef enum {
    CONN_TRANSMITTER,
    CONN_RECEIVER
} ConnectionFlag;

static inline void printProgressBar(int a, int b) {
    int barSize = 50;
    float perc = (float)a/b;
    printf("\r%d/%d (bytes) [", a, b);
    uint i;
    for (i = 0; i < 50; ++i) {
        if (i < perc*barSize) printf("=");
        else printf(" ");
    }

    printf("] %.0f%%", perc*100);
    fflush(stdout);
}

#endif /* UTILS_H */
