#define LINKLAYER_GLOBAL
#include "link.h"
#include "alarm.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <time.h>


LinkLayer* ll;

/*
 * OPEN
 */
int llopen(int porta, ConnectionFlag flag) {
    if (flag != CONN_TRANSMITTER && flag != CONN_RECEIVER) {
        printf("FLAG not supported (must be either LL_TRANSMISSER or LL_RECEIVER)\n");
        return -1;
    }

    const char* base_port = "/dev/ttyS";
    char port_name[strlen(base_port) + 2];
    sprintf(port_name, "%s%d", base_port, porta);

    printf("Opening port : '%s'\n", port_name);

    int fd = open(port_name, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        perror(port_name);
        printf("Could not open port\n");
        return -1;
    }

    int res = -1;
    // Transmitter
    if (flag == CONN_TRANSMITTER) {
        linkLayer_constructor(fd, port_name, 1, 3, CONN_TRANSMITTER);
        res = llopen_as_transmitter(fd);
    }
    // Receiver
    else {
        linkLayer_constructor(fd, port_name, 1, 3, CONN_RECEIVER);
        res = llopen_as_receiver(fd);
    }

    srand(time(NULL));
    return res;
}

int llopen_as_transmitter(int fd) {
    printf("! Port set as transmitter !\n");

    printf("Establishing connection...\n");

    LLFrame* set = LLFrame_create_command(A_COM_T, C_SET, 0);
    int res = send_with_retransmission(fd, set, C_UA);

    if (res) return fd;

    return -1;
}

int llopen_as_receiver(int fd) {
    printf("! Port set as receiver !\n");

    char c;
    do {
        read(fd, &c, sizeof(char));
    } while (c != FLAG);

    LLFrame* comm = LLFrame_from_fd(fd);

    if (LLFrame_is_command(comm, C_SET)) {
        LLFrame* ua = LLFrame_create_command(A_ANS_R, C_UA, 0);
        LLFrame_write(ua, fd);
        LLFrame_delete(&ua);
    }
    else {
        LLFrame_print_msg(comm, "Invalid response: ");
        fd = -1;
    }

    LLFrame_delete(&comm);
    return fd;
}

/*
 * WRITE
 */
int llwrite(int fd, const char* buffer, uint length) {
    uint numTries = 0;
    bool transfering = true;
    int written = -1;

    (void) signal(SIGALRM, alarm_handler);

    while (transfering) {
        if (numTries == 0 || alarmRing) {
            if (numTries > ll->numTransmissions) {
                alarm(0);
                alarmRing = false;
                printf("!!!ERROR::RETRANSMISSION - Maximum number of retransmissions exceeded!!!!\n");
                transfering = false;
                written = -1;
                break;
            }

            if (alarmRing)
                printf("!!!ERROR::TIMEOUT - Retrying (%dx)\n", numTries);

            alarmRing = false;
            alarm(ll->timeout);

            LLFrame* data = LLFrame_create_info(buffer, length, ll->sequenceNumber);
            written = LLFrame_write(data, fd);
            LLFrame_delete(&data);

            ++numTries;
        }

        char c;
        read(fd, &c, sizeof(c));
        if (c==FLAG) {
            LLFrame* response = LLFrame_from_fd(fd);
            if (LLFrame_is_command(response, C_RR)) {
                if (ll->sequenceNumber != response->nr)
                    ll->sequenceNumber = response->nr;

                alarm(0);
                alarmRing = false;
                transfering = false;
            }
            else if (LLFrame_is_command(response, C_REJ)) {
                alarm(0);
                alarmRing = false;
                numTries = 0;
            }
            LLFrame_delete(&response);
        }
    }
    alarm(0);
    alarmRing = false;

    return written;
}

/*
 * READ
 */
int llread(int fd, char** buffer) {
    int size = 0;
    bool done = false;
    while (!done) {
        char c;
        do {
            read(fd, &c, sizeof(c));
        }while(c != FLAG);

        LLFrame* buf = LLFrame_from_fd(fd);
        if (LLFrame_is_invalid(buf) && buf != NULL) {
            if (buf->error == LL_ERROR_BCC2) {
            BCC2_ERROR_SIMUL:
                if (ll->sequenceNumber == buf->ns) { // New frame
                    LLFrame* rej = LLFrame_create_command(A_ANS_R, C_REJ, buf->ns);
                    LLFrame_write(rej, fd);
                    LLFrame_delete(&rej);
                }
                else if (ll->sequenceNumber != buf->ns) { // Duplicate
                    LLFrame* rr = LLFrame_create_command(A_ANS_R, C_RR, ll->sequenceNumber);
                    LLFrame_write(rr, fd);
                    LLFrame_delete(&rr);
                }
            }
        }
        else if (LLFrame_is_command(buf, C_DISC)) {
            LLFrame* disc = LLFrame_create_command(A_COM_R, C_DISC, 0);
            send_with_retransmission(fd, disc, C_UA);
            LLFrame_delete(&disc);

            done = true;
        }
        else if (buf->type == LL_FRAME_INFO) {
            if (ll->sequenceNumber == buf->ns) { // New frame

                // Random generation of BCC2 errors
                if (SIMUL_ERROR) {
                    int random_number = rand()%10000 + 1;
                    if (random_number <= 5) {
                        printf("\nSimulating BCC2 error\n");
                        goto BCC2_ERROR_SIMUL;
                    }
                }

                size = LLFrame_get_data(buf, buffer);

                ll->sequenceNumber = !buf->ns;

                LLFrame* rr = LLFrame_create_command(A_ANS_R, C_RR, ll->sequenceNumber);
                LLFrame_write(rr, fd);
                LLFrame_delete(&rr);

                done = true;
            }
            else if (ll->sequenceNumber != buf->ns) { // Duplicate frame
                LLFrame* rr = LLFrame_create_command(A_ANS_R, C_RR, ll->sequenceNumber);
                LLFrame_write(rr, fd);
                LLFrame_delete(&rr);
            }
        }
        LLFrame_delete(&buf);
    }

    return size;
}

/*
 * CLOSE
 */
int llclose(int fd) {
    if (ll->mode == CONN_TRANSMITTER) {
        printf("Trying to close connection...\n");

        LLFrame* disc = LLFrame_create_command(A_COM_T, C_DISC, 0);
        int res = send_with_retransmission(fd, disc, C_DISC);
        LLFrame_delete(&disc);

        if (!res) goto close;

        LLFrame* ua = LLFrame_create_command(A_ANS_T, C_UA, 0);
        LLFrame_write(ua, fd);
        LLFrame_delete(&ua);
    }

close:
    if (DEBUG)
        printf("Restoring old port configuration...\n");
    if (tcsetattr(fd, TCSANOW, &ll->oldtio) == -1 ){
        perror("tcsetattr");
        return -1;
    }

    if (DEBUG)
        printf("Closing port...\n");
    close(fd);

    return 1;
}

int send_with_retransmission(int fd, LLFrame* msg, LL_C answer) {
    uint numTries = 0;
    int res = 0;
    bool done = false;
    (void) signal(SIGALRM, alarm_handler);

    while (!done) {
        if (numTries == 0 || alarmRing) {
            if (numTries > ll->numTransmissions) {
                alarm(0);
                alarmRing = false;
                printf("!!!ERROR::RETRANSMISSION - Maximum number of retransmissions exceeded!!!!\n");
                done = true;
                break;
            }
            if (alarmRing)
                printf("!!!ERROR::TIMEOUT - Retrying (%dx)\n", numTries);

            alarmRing = false;
            alarm(ll->timeout);

            LLFrame_write(msg, fd);

            ++numTries;
        }

        char c;
        read(fd, &c, sizeof(c));
        if (c == FLAG) {
            LLFrame* response = LLFrame_from_fd(fd);
            if (LLFrame_is_command(response, answer)) {
                alarm(0);
                alarmRing = false;
                done = true;
                res = 1;
            }
            LLFrame_delete(&response);
        }
    }
    alarm(0);
    alarmRing = false;

    return res;
}

int configureTermios(int fd, struct termios *t) {
    bzero(t, sizeof(*t));
    t->c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    t->c_iflag = IGNPAR;
    t->c_oflag = OPOST;
    t->c_lflag = 0; // non-canonical

    t->c_cc[VTIME]    = 0;   // inter-character timer unused
    t->c_cc[VMIN]     = 0;   // blocking read until 5 chars received

    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd,TCSANOW,t) == -1) {
        perror("tcsetattr");
        return false;
    }

    return true;
}

int linkLayer_constructor(int fd, char* port_name, uint timeout, uint nTrans, ConnectionFlag mode){
    ll = (LinkLayer*) malloc(sizeof(LinkLayer));
    strcpy(ll->port, port_name);
    ll->sequenceNumber = 0;
    ll->timeout = timeout;
    ll->numTransmissions = nTrans;
    ll->mode = mode;

    printf("Saving previous port configuration...\n");
    if (tcgetattr(fd, &ll->oldtio) == -1) {
        perror("tcgetattr");
        return -1;
    }

    printf("Configuring new port...\n");
    if (!configureTermios(fd, &ll->newtio)) return -1;

    return 1;
}
