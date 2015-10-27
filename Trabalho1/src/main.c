#include "app.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {

    if (argc != 1 && argc!= 4) {
        printf("Usage: %s [FILENAME] [PORT] [MODE]\n", argv[0]);
        printf("Interactive mode: %s", argv[0]);
        return 1;
    }

    int port, mode, numTrans, timeTrans, maxSize, debug, simul;
    char filename[50];

    if (argc == 1) {
        printf("Interactive mode: \n");
        while (1) {
            printf("Mode:\n");
            printf("  (0) Transmitter\n");
            printf("  (1) Receiver\n");
            printf("> ");
            scanf("%d", &mode);
            if (mode == 0 || mode == 1) break;
        }
        while (1) {
            printf("Port: ");
            scanf("%d", &port);
            if (port >= 0) break;
        }
        printf("Filename: ");
        scanf("%s", filename);
        if (mode == CONN_TRANSMITTER) {
            while (1) {
                printf("Maximum number of retransmissions: ");
                scanf("%d", &numTrans);
                if (numTrans >= 0) break;
            }
            while (1) {
                printf("Time between retransmissions (seconds): ");
                scanf("%d", &timeTrans);
                if (timeTrans > 0) break;
            }
            while (1) {
                printf("Maximum size for data frames (bytes): ");
                scanf("%d", &maxSize);
                if (maxSize > 0) break;
            }
        }
        while (1) {
            printf("Debug:\n");
            printf("  (0) Off\n");
            printf("  (1) On\n");
            printf("> ");
            scanf("%d", &debug);
            if (debug == 0 || debug == 1) break;
        }
        if (mode == CONN_RECEIVER) {
            while (1) {
                printf("Simulate errors:\n");
                printf("  (0) Off\n");
                printf("  (1) On\n");
                printf("> ");
                scanf("%d", &simul);
                if (simul == 0 || simul == 1) break;
            }
        }
    }
    else {
        strcpy(filename, argv[1]);
        port = atoi(argv[2]);
        mode = atoi(argv[3]);
        numTrans = 3;
        timeTrans = 3;
        maxSize = 512;
        debug = 0;
        simul = 0;
    }

    printf("MODE: %d\n", mode);
    if (mode != 0 && mode != 1) {
        printf("Invalid mode! (0 - Receiver | 1 - Transmitter)\n");
        return 1;
    }

    AppLayer* app = AppLayer_constructor(port, mode, filename,
            numTrans, timeTrans, maxSize, debug, simul);

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
