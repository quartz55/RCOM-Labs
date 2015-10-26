#include "app.h"
#include "link.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

AppLayer* AppLayer_constructor(int port, ConnectionFlag status, char* filename) {
    int fd = llopen(port, status);
    if (fd < 0) {
        printf("!!!ERROR::CONNECTION - Couldn't establish connection\n");
        return NULL;
    }

    AppLayer* app = (AppLayer*) malloc(sizeof(AppLayer));
    app->fd = fd;
    strcpy(app->filename, filename);
    app->status = status;

    return app;
}

int AppLayer_start_transfer(AppLayer* app) {
    if (app == NULL){
        printf("Something went wrong\n");
        return -1;
    }
    printf("Filename: %s\n", app->filename);
    if (strlen(app->filename) == 0) {
        printf("Invalid filename\n");
        return -1;
    }

    switch(app->status) {
    case CONN_RECEIVER:
        return AppLayer_receive(app);
        break;
    case CONN_TRANSMITTER:
        return AppLayer_send(app);
        break;
    }

    return 0;
}

void AppLayer_delete(AppLayer** app) {
    if (*app == NULL) return;
    llclose((*app)->fd);

    free(*app);
    *app = NULL;
}

int AppLayer_send(AppLayer* app) {
    FILE* file = fopen(app->filename, "rb");
    if (file == NULL) {
        printf("!!!ERROR::FILE - Could not open file '%s'\n", app->filename);
        return -2;
    }
    printf("Opened file %s to send\n", app->filename);

    CtrlPackage* start = CtrlPackage_create(PKG_START, 1000, app->filename);
    llwrite(app->fd, start->buffer, start->bufSize);
    CtrlPackage_delete(&start);

    CtrlPackage* end = CtrlPackage_create(PKG_END, 1000, app->filename);
    llwrite(app->fd, end->buffer, end->bufSize);
    CtrlPackage_delete(&end);

    return 1;
}

int AppLayer_receive(AppLayer* app) {

    int res = 0;

    char* pkg_buf;
    uint pkgSize = llread(app->fd, &pkg_buf);

    CtrlPackage* pkg = CtrlPackage_from_buf(pkg_buf, pkgSize);
    free(pkg_buf);

    uint expectedFileSize = atoi(pkg->params[CP_PARAM_SIZE].value);
    char* filename = pkg->params[CP_PARAM_NAME].value;

    if (pkg->type != PKG_START) {
        printf("!!!ERROR::CTRL_PKG - Not START package\n");
        return -1;
    }


    printf("peidou\n");
    FILE* file = fopen(app->filename, "wb");
    if (file == NULL) {
        printf("!!!ERROR::FILE - Could not create file '%s'\n", app->filename);
        return -2;
    }

    printf("----------------------------\n");
    printf("Receiving file '%s'(%d bytes)\n", filename, expectedFileSize);
    printf("Saving as '%s'\n", app->filename);
    printf("----------------------------\n");

    CtrlPackage_delete(&pkg);

    uint currFileSize = 0;

    while (currFileSize != expectedFileSize) {
        pkgSize = llread(app->fd, &pkg_buf);

        pkg = CtrlPackage_from_buf(pkg_buf, pkgSize);
        if (pkg != NULL) {
            if (pkg->type == PKG_END) {
                printf("!!!ERROR::CTR_PKG - Unnexpected END package\n");
                free(pkg_buf);
                CtrlPackage_delete(&pkg);
                res = -1;
                break;
            }
        }
    }

    fclose(file);

    return res;
}

CtrlPackage* CtrlPackage_create(PackageType type, int filesize, char* filename) {
    if (type == PKG_DATA) return NULL;
    CtrlPackage* pkg = (CtrlPackage*) malloc(sizeof(CtrlPackage));
    pkg->type = type;

    int filesize_str_size = snprintf(NULL, 0, "%d", filesize) + 1;
    char filesize_str[filesize_str_size];
    snprintf(filesize_str, filesize_str_size, "%d", filesize);

    pkg->params[CP_PARAM_SIZE].type = CP_PARAM_SIZE;
    pkg->params[CP_PARAM_SIZE].length = filesize_str_size;
    pkg->params[CP_PARAM_SIZE].value = (char*) malloc(filesize_str_size*sizeof(char));
    strcpy(pkg->params[CP_PARAM_SIZE].value, filesize_str);

    pkg->params[CP_PARAM_NAME].type = CP_PARAM_NAME;
    pkg->params[CP_PARAM_NAME].length = strlen(filename);
    pkg->params[CP_PARAM_NAME].value = (char*) malloc(strlen(filename)*sizeof(char));
    strcpy(pkg->params[CP_PARAM_NAME].value, filename);

    uint buf_i = 0;
    uint bufSize = 1+(CP_NUM_PARAMS*2)+filesize_str_size+strlen(filename);
    pkg->buffer = (char*) malloc(bufSize*sizeof(char));
    pkg->buffer[buf_i++] = type;

    uint i;
    for (i = 0; i < CP_NUM_PARAMS; ++i) {
        pkg->buffer[buf_i++] = pkg->params[i].type;
        pkg->buffer[buf_i++] = pkg->params[i].length;
        memcpy(&pkg->buffer[buf_i], pkg->params[i].value, pkg->params[i].length);
        buf_i += pkg->params[i].length;
    }

    pkg->bufSize = bufSize;

    CtrlPackage_print(pkg);
    return pkg;
}

CtrlPackage* CtrlPackage_from_buf(char* buf, uint size) {
    if (buf[0] != PKG_START && buf[0] != PKG_END) return NULL;

    CtrlPackage* pkg = (CtrlPackage*) malloc(sizeof(CtrlPackage));
    pkg->type = buf[0];
    pkg->buffer = (char*) malloc(size*sizeof(char));
    strcpy(pkg->buffer, buf);
    pkg->bufSize = size;

    uint i, param_i = 0;
    for (i = 1; i < size; ) {
        pkg->params[param_i].type = (CP_ParamType) buf[i++];
        uint length = pkg->params[param_i].length = (uint) buf[i++];

        pkg->params[param_i].value = (char*) malloc(length*sizeof(char));
        memcpy(pkg->params[param_i].value, &buf[i], length);

        i+=length;
        ++param_i;
    }

    CtrlPackage_print(pkg);
    return pkg;
}

void CtrlPackage_delete(CtrlPackage** pkg) {
    if (*pkg == NULL) return;

    uint i;
    for (i = 0; i < CP_NUM_PARAMS; ++i) {
        free((*pkg)->params[i].value);
        (*pkg)->params[i].value = NULL;
    }

    free((*pkg)->buffer);
    (*pkg)->buffer = NULL;

    free(*pkg);
    *pkg = NULL;
}

void CtrlPackage_print(CtrlPackage* pkg) {
    printf("------------------\n");
    switch(pkg->type) {
    case PKG_START:
        printf("START PKG\n");
        break;
    case PKG_END:
        printf("END PKG\n");
        break;
    default:
        printf("fodasse\n");
        return;
        break;
    }

    uint i;
    for (i = 0; i < CP_NUM_PARAMS; ++i) {
        switch(pkg->params[i].type) {
        case CP_PARAM_SIZE:
            printf("Size: %s (%d bytes)\n", pkg->params[i].value, pkg->params[i].length);
            break;
        case CP_PARAM_NAME:
            printf("Name: %s (%d bytes)\n", pkg->params[i].value, pkg->params[i].length);
            break;
        }
    }

    /* printf("Buffer: %s (%d/%d bytes)\n", pkg->buffer, pkg->bufSize, strlen(pkg->buffer)); */
    printf("------------------\n");
}
