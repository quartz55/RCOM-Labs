#include "link.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s [PORT NUMBER]\n", argv[0]);
        return -1;
    }
    int port = atoi(argv[1]);
    int fd = -1;
    fd = llopen(port, CONN_RECEIVER);
    if (fd > 0) {
        printf("======================\n");
        printf("Connection established\n");
        printf("======================\n");

        char buf[512];
        llread(fd, buf);

        llclose(fd);
    }
    else printf("Not successful\n");

    return 0;
}
