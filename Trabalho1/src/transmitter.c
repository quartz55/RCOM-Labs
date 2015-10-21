#include "IPA.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s [PORT NUMBER]\n", argv[0]);
        return -1;
    }
    int port = atoi(argv[1]);
    int res = -1;
    res = llopen(port, IPA_TRANSMITTER);
    if (res > 0) {
        printf("Connection established\n");
        llclose(res);
    }
    else printf("Not successful\n");

    return 0;
}
