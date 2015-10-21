#include "link.h"

#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

static struct termios oldtio;
static LinkLayer protocol;
static uint currTrans = 0;

/*
 * OPEN
 */
int llopen(int porta, LL_FLAG flag) {
    if (flag != LL_TRANSMITTER && flag != LL_RECEIVER) {
        printf("FLAG not supported (must be either LL_TRANSMISSER or LL_RECEIVER)\n");
        return -1;
    }

    /*
     * Open port
     */
    char port_name[strlen(DEVICE) + 1];
    sprintf(port_name, "%s%d", DEVICE, porta);

    printf("Opening port : '%s'\n", port_name);

    int fd = open(port_name, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        perror(port_name);
        printf("Could not open port\n");
        return -1;
    }


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
    newtio.c_cc[VMIN]     = 0;   // blocking read until 5 chars received

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
    if (flag == LL_TRANSMITTER) {
        linkLayer_constructor(&protocol, port_name, 1, 3, LL_TRANSMITTER);
        return llopen_as_transmitter(fd);
    }
    // Receiver
    else {
        linkLayer_constructor(&protocol, port_name, 1, 3, LL_RECEIVER);
        return llopen_as_receiver(fd);
    }
}

int llopen_as_transmitter(int fd) {

    printf("Setting port as transmitter...\n");

    print_frame(SET, sizeof(SET), "Sending SET:");
    write(fd, SET, sizeof(SET));

    print_frame(UA, sizeof(UA), "Waiting for UA");

    char buf[5];
    int res = read_frame_timeout(fd, buf, SET, sizeof(SET));

    if (compare_frames(buf, UA, res)) {
        print_frame(buf, res, "Received UA:");
        return fd;
    }

    printf("Not a valid response\n");
    return -1;
}

int llopen_as_receiver(int fd) {
    printf("Setting port as receiver...\n");

    print_frame(SET, sizeof(SET), "Waiting for SET");

    char buf[5];
    int res = read_frame(fd, buf);

    if (compare_frames(buf, SET, sizeof(SET))) {
        print_frame(buf, res, "Received SET:");

        print_frame(UA, sizeof(UA), "Sending UA:");

        write(fd, UA, sizeof(UA));
        return fd;
    }

    print_frame(buf, sizeof(buf), "Unnexpected answer");
    return -1;
}

/*
 * CLOSE
 */
int llclose(int fd) {
    if (protocol.mode == LL_TRANSMITTER) {
        print_frame(DISC_T, sizeof(DISC_T), "Sending DISC:");

        char buf[5];
        int res = read_frame_timeout(fd, buf, DISC_T, sizeof(DISC_T));
        if (res < 0) {
            goto close;
        }

        if (compare_frames(buf, DISC_R, res)) {
            print_frame(buf, res, "DISC received:");
            print_frame(UA, sizeof(UA), "Sending UA:");

            write(fd, UA, sizeof(UA));
        }
    }

close:
    printf("Restoring old port configuration...\n");
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1 ){
        perror("tcsetattr");
        return -1;
    }

    printf("Closing port...\n");
    printf("============\n");
    printf("Disconnected\n");
    printf("============\n");
    close(fd);
    return 1;
}

/*
 * Misc
 */
int read_frame_timeout(int fd, char* frame, const char msg[], unsigned long msg_size) {
    write(fd, msg, msg_size);

    int size = 0, res = -1;

    (void) signal(SIGALRM, timeout);
    alarm(protocol.timeout);
    currTrans = 0;
    uint trasTracker = currTrans;
    uint maxTrans = protocol.numTransmissions;
    // Read until a frame is found (FLAG is read)
    char c;
    do {
        res = read(fd, &c, sizeof(c));
        if (res < 0) {
            perror("read");
            return -1;
        }
        if (trasTracker != currTrans) {
            trasTracker = currTrans;
            if (currTrans > maxTrans) {
                printf("Could not get answer...\n");
                return -1;
            }
            printf("!!!TIMEOUT!!! Retrying... (%dx)\n", currTrans);
            write(fd, msg, msg_size);
            alarm(protocol.timeout);
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

bool compare_frames(const char f1[], const char f2[], unsigned int size) {
    int i;
    for (i=0; i < size; ++i) {
        if (f1[i] != f2[i]) return false;
    }

    return true;
}

int print_frame(const char frame[], unsigned int size, char msg[]) {
    printf("%s\t", msg);
    int i;
    printf("| ");
    for (i=0; i < size; i++) printf("%X | ", frame[i]);

    printf("(%d bytes)\n", size);

    return 1;
}

void timeout() {
    currTrans++;
}
