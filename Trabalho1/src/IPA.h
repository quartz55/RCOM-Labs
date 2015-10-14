/**
 * @file   IPA.h
 * HEADER file for the IPA api
 *
 */

#ifndef IPA_H
#define IPA_H

#define MAX_SIZE 512

typedef unsigned int uint;
typedef enum {false, true} bool;

typedef enum {
    IPA_TRANSMITTER,
    IPA_RECEIVER
} IPA_FLAG;

typedef struct {
    int fileDescriptor;
    int status;
} applicationLayer;

typedef struct {
    char port[20];
    int baudRate;
    uint sequenceNumber;
    uint timeout;
    uint numTransmissions;

    char frame[MAX_SIZE];
} linkLayer;

/**
 * Establishes a connection on the port specified
 *
 * @param porta
 * @param flag IPA_TRANSMITTER | IPA_RECEIVER
 *
 * @return id of connection, -1 if error
 */
int llopen(int porta, int flag);

/**
 * Sends information through a connection
 *
 * @param fd File descriptor of connection
 * @param buffer Array of chars to send
 * @param length Size of the array
 *
 * @return Number of chars sent, -1 if error
 */
int llwrite(int fd, char* buffer, int length);

/**
 * Reads information from a connection
 *
 * @param fd File descriptor of connection
 * @param buffer Array of chars received
 *
 * @return Number of chars read, -1 if error
 */
int llread(int fd, char* buffer);

/**
 * Closes the connection specified
 *
 * @param fd File descriptor of connection
 *
 * @return 1 if success, -1 is error
 */
int llclose(int fd);

#endif /* IPA_H */
