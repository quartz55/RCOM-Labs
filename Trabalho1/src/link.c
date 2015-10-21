#include "IPA.h"

#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define DEVICE "/dev/pts/"

static struct termios oldtio;

static linkLayer protocol;

void linkLayer_constructor(linkLayer* l, char port_name[], uint timeout, uint nTrans) {
    strcpy(l->port, port_name);
    l->sequenceNumber = 0;
    l->timeout = timeout;
    l->numTransmissions = nTrans;
}

/*
 * OPEN
 */
int llopen(int porta, int flag) {
    if (flag != IPA_TRANSMITTER && flag != IPA_RECEIVER) {
        printf("FLAG not supported (must be either IPA_TRANSMISSER or IPA_RECEIVER)\n");
        return -1;
    }

    char port_name[strlen(DEVICE) + 1];
    sprintf(port_name, "%s%d", DEVICE, porta);

    printf("Opening port : '%s'\n", port_name);

    int fd = open(port_name, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        perror(port_name);
        printf("Could not open port\n");
        return -1;
    }

    linkLayer_constructor(&protocol, port_name, 1, 3);

    /*
     * Port configuration
     */
    printf("Starting port configuration...\n");

    struct termios newtio;
    if (tcgetattr(fd, &oldtio) == -1) {
        perror("tcgetattr");
        return -1;
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = OPOST;

    newtio.c_lflag = 0; // non-canonical
    newtio.c_cc[VTIME]    = 0;   // inter-character timer unused
    newtio.c_cc[VMIN]     = 1;   // blocking read until 5 chars received

    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd,TCSANOW,&newtio) == -1) {
        perror("tcsetattr");
        return -1;
    }

    // Print configuration
    printf("  | Port configurated successfully:\n");
    printf("  |\t"); newtio.c_lflag ? printf("Canonical\n") : printf("Non-canonical\n");
    printf("  |\tVTIME | %d\n", newtio.c_cc[VTIME]);
    printf("  |\tVMIN | %d\n", newtio.c_cc[VMIN]);


    // Transmitter
    if (flag == IPA_TRANSMITTER) {
        return llopen_as_transmitter(fd);
    }
    // Receiver
    else {
        return llopen_as_receiver(fd);
    }
}

int llopen_as_transmitter(int fd) {
    const char SET[] = {FLAG, A_COM_TRANS, C_SET, A_COM_TRANS^C_SET, FLAG};
    const char UA[] = {FLAG, A_ANS_RECEIV, C_UA, A_ANS_RECEIV^C_UA, FLAG};

    printf("Setting port as transmitter...\n");

    printf("Sending SET: ");
    print_frame_i(SET, sizeof(SET));
    printf("\n");
    write(fd, SET, 5*sizeof(char));

    printf("Waiting for UA...\n");
    char buf[5];
    int res = read_frame(fd, buf);

    if (compare_frames(buf, UA, sizeof(UA))) {
        printf("Received UA: ");
        print_frame_i(UA, sizeof(UA));
        printf("\n");
        return fd;
    }

    printf("Not a valid response...\n");
    return -1;
}

int llopen_as_receiver(int fd) {
    const char SET[] = {FLAG, A_COM_TRANS, C_SET, A_COM_TRANS^C_SET, FLAG};
    const char UA[] = {FLAG, A_ANS_RECEIV, C_UA, A_ANS_RECEIV^C_UA, FLAG};

    printf("Setting port as receiver...\n");

    printf("Waiting for SET...\n");
    char buf[5];
    int res = read_frame(fd, buf);

    if (compare_frames(buf, SET, sizeof(SET))) {
        printf("Received SET: ");
        print_frame_i(buf, sizeof(buf));
        printf("\n");

        printf("Sending UA: ");
        print_frame_i(UA, sizeof(UA));
        printf("\n");

        write(fd, UA, sizeof(UA));
        return fd;
    }

    printf("Unnexpected answer\n");
    print_frame_i(buf, sizeof(buf));
    printf("\n");
    return -1;
}

/*
 * OPEN
 */
int llclose(int fd) {
    printf("Restoring old port configuration...\n");
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1 ){
        perror("tcsetattr");
        return -1;
    }

    close(fd);
    return 1;
}

/*
 * Misc
 */
int read_frame(int fd, char* frame) {
    int size = 0, res = -1;

    // Read until a frame is found (FLAG is read)
    char c;
    do {
        res = read(fd, &c, sizeof(c));
        if (res < 0) {
            perror("read");
            return -1;
        }
    } while (c != FLAG);

    // Read until the end of frame (FLAG is read)
    do {
        frame[size++] = c;
        res = read(fd, &c, sizeof(c));
        if (res < 0) {
            perror("read");
            return -1;
        }
        if (c == FLAG) {
            frame[size++] = c;
            break;
        }
    } while (size < MAX_SIZE);

    return size;
}

int compare_frames(const char f1[], const char f2[], unsigned int size) {
    int i;
    for (i=0; i < size; ++i) {
        if (f1[i] != f2[i]) return false;
    }

    return true;
}

int print_frame(const char frame[], unsigned int size) {
    int i;
    printf("| ");
    for (i=0; i < size; i++) printf("%X | ", frame[i]);

    return 1;
}

int print_frame_i(const char frame[], unsigned int size) {
    int i;
    printf("| ");
    for (i=0; i < size; i++) printf("%X | ", frame[i]);

    printf("(%d bytes)", size);

    return 1;
}
