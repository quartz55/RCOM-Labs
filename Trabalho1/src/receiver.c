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
    fd = llopen(port, LL_RECEIVER);
    if (fd > 0) {
        printf("======================\n");
        printf("Connection established\n");
        printf("======================\n");
        return llclose(fd);
        while (1) {
            char buf[MAX_SIZE];
            read_frame(fd, buf);

            if (compare_frames(buf, DISC_T, sizeof(DISC_T))) {
                write(fd, DISC_R, sizeof(DISC_R));

                read_frame_timeout(fd, buf, DISC_R, sizeof(DISC_R));

                if (compare_frames(buf, UA, sizeof(UA))) {
                    break;
                }
            }
        }

        llclose(fd);
    }
    else printf("Not successful\n");

    return 0;
}
