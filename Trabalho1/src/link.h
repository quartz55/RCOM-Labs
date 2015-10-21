#ifndef IPA_H
#define IPA_H

#define MAX_SIZE 512
#define BAUDRATE B38400

// Flags
/*
  INFO FRAME

  | F | A | C | BCC1 | DATA --- DATA | BCC2 | F |


  SUPERVISION FRAMES

  | F | A | C | BCC1 | F |

*/
#define FLAG 0x7e
#define A_COM_TRANS 0x03
#define A_ANS_TRANS 0x01
#define A_COM_RECEIV 0x01
#define A_ANS_RECEIV 0x03
#define C_SET 0x07
#define C_DISC 0x0b
#define C_UA 0x03

typedef unsigned int uint;
typedef enum {false, true} bool;

/*
 * Data structures
 */
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

/*
 * Functions
 */

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

int llopen_as_transmitter(int fd);
int llopen_as_receiver(int fd);

int read_frame(int fd, char* frame);
int compare_frames(const char f1[], const char f2[], unsigned int size);
int print_frame(const char frame[], unsigned int size);
int print_frame_i(const char frame[], unsigned int size);

#endif /* IPA_H */
