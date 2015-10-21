#ifndef LINK_STRUCT_H
#define LINK_STRUCT_H

#include "link_data.h"
#include "link_utils.h"

#include <string.h>

typedef enum {
    LL_TRANSMITTER,
    LL_RECEIVER
} LL_FLAG;

typedef struct {
    char port[20];
    int baudRate;
    uint sequenceNumber;
    uint timeout;
    uint numTransmissions;
    LL_FLAG mode;

    char frame[MAX_SIZE];
} LinkLayer;

static inline void linkLayer_constructor(LinkLayer* l, char port_name[], uint timeout, uint nTrans, LL_FLAG mode) {
    strcpy(l->port, port_name);
    l->sequenceNumber = 0;
    l->timeout = timeout;
    l->numTransmissions = nTrans;
    l->mode = mode;
}

static inline void linkLayer_clear(LinkLayer* l) {
    bzero(l->frame, MAX_SIZE);
}

#endif /* LINK_STRUCT_H */
