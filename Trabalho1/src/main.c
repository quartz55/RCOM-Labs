#include "app.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {

    if (argc != 4) {
        printf("Usage: %s [PORT] [MODE] [FILENAME]\n", argv[0]);
        return 1;
    }
    int port, mode;

    port = atoi(argv[1]);
    mode = atoi(argv[2]);

    AppLayer* app;
    if (mode == 0)
        app = AppLayer_constructor(port, CONN_RECEIVER, argv[3]);
    else if (mode == 1)
        app = AppLayer_constructor(port, CONN_TRANSMITTER, argv[3]);
    else {
        printf("Invalid mode! (0 - Receive | 1 - Send)\n");
        return 1;
    }

    if (app != NULL) {
        printf("\n");
        printf("======================\n");
        printf("Connection Established\n");
        printf("======================\n");
        printf("\n");
    }

    int res = AppLayer_start_transfer(app);
    AppLayer_delete(&app);

    if (res != 1) {
        printf("!!!ERROR::CONNECTION - Couldn't close connection\n");
    }
    else {
        printf("\n");
        printf("============\n");
        printf("Disconnected\n");
        printf("============\n");
        printf("\n");
    }

    return 0;
}
