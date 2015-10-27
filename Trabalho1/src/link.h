#ifndef LINK_H
#define LINK_H

#include "llframe.h"
#include "utils.h"

#include <termios.h>

#define BAUDRATE B38400

typedef struct {
    char port[20];
    int baudRate;
    uint sequenceNumber;
    uint timeout;
    uint numTransmissions;
    ConnectionFlag mode;

    struct termios oldtio, newtio;
} LinkLayer;

extern LinkLayer* ll;

extern int linkLayer_constructor(int fd, char* port_name, uint timeout, uint nTrans, ConnectionFlag mode);

/**
 * Establishes a connection on the port specified
 *
 * @param porta
 * @param flag LL_TRANSMITTER | LL_RECEIVER
 *
 * @return id of connection, -1 if error
 */
extern int llopen(int porta, ConnectionFlag flag);

/**
 * Sends information through a connection
 *
 * @param fd File descriptor of connection
 * @param buffer Array of chars to send
 * @param length Size of the array
 *
 * @return Number of chars sent, -1 if error
 */
extern int llwrite(int fd, const char* buffer, uint length);

/**
 * Reads information from a connection
 *
 * @param fd File descriptor of connection
 * @param buffer Array of chars received
 *
 * @return Number of chars read, -1 if error
 */
extern int llread(int fd, char** buffer);

/**
 * Closes the connection specified
 *
 * @param fd File descriptor of connection
 *
 * @return 1 if success, -1 is error
 */
extern int llclose(int fd);

extern int llopen_as_transmitter(int fd);
extern int llopen_as_receiver(int fd);

extern int send_with_retransmission(int fd, LLFrame* msg, LL_C answer);
extern int configureTermios(int fd, struct termios *t);

#endif /* LINK_H */
