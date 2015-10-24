#ifndef LINK_STRUCT_H
#define LINK_STRUCT_H

#include "../utils.h"

#include <string.h>
#include <termios.h>


typedef struct {
    char port[20];
    int baudRate;
    uint sequenceNumber;
    uint timeout;
    uint numTransmissions;
    ConnectionFlag mode;

    struct termios oldtio, newtio;
} LinkLayer;

static inline void linkLayer_constructor(LinkLayer* l, char port_name[], uint timeout, uint nTrans, ConnectionFlag mode) {
    strcpy(l->port, port_name);
    l->sequenceNumber = 0;
    l->timeout = timeout;
    l->numTransmissions = nTrans;
    l->mode = mode;
}

#endif /* LINK_STRUCT_H */
