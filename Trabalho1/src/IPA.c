#include "IPA.h"

#include <stdio.h>

int llopen(int porta, int flag) {
    switch(flag) {
    case IPA_TRANSMITTER:
        break;
    case IPA_RECEIVER:
        break;
    default:
        printf("FLAG not supported (must be either IPA_TRANSMISSER or IPA_RECEIVER)\n");
        return -1;
        break;
    }

    int id = 0;

    return id;
}
