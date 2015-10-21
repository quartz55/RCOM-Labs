#ifndef LINK_H
#define LINK_H

#include "link/link_data.h"
#include "link/link_utils.h"
#include "link/link_struct.h"

extern void timeout();

/**
 * Establishes a connection on the port specified
 *
 * @param porta
 * @param flag LL_TRANSMITTER | LL_RECEIVER
 *
 * @return id of connection, -1 if error
 */
extern int llopen(int porta, LL_FLAG flag);

/**
 * Sends information through a connection
 *
 * @param fd File descriptor of connection
 * @param buffer Array of chars to send
 * @param length Size of the array
 *
 * @return Number of chars sent, -1 if error
 */
extern int llwrite(int fd, char* buffer, int length);

/**
 * Reads information from a connection
 *
 * @param fd File descriptor of connection
 * @param buffer Array of chars received
 *
 * @return Number of chars read, -1 if error
 */
extern int llread(int fd, char* buffer);

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

extern int read_frame(int fd, char* frame);
extern int read_frame_timeout(int fd, char* frame, const char msg[], unsigned long msg_size);
extern bool compare_frames(const char f1[], const char f2[], unsigned int size);
extern int print_frame(const char frame[], unsigned int size, char msg[]);

#endif /* LINK_H */
