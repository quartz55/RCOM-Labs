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

    switch(app->status) {
    case CONN_RECEIVER: {
        int res = AppLayer_receive(app);
        llread(app->fd, NULL);
        return res;
        break;
    }
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
    int filesize = getFileSize(file);
    printf("Opened file %s to send (%d bytes)\n", app->filename, filesize);

    CtrlPackage* start = CtrlPackage_create(PKG_START, filesize, app->filename);
    llwrite(app->fd, start->buffer, start->bufSize);
    CtrlPackage_delete(&start);

    int res = 1;
    int read = 0, written = 0;
    uint i = 0;

    char buffer[MAX_SIZE];
    while((read = fread(buffer, sizeof(char), MAX_SIZE, file)) > 0) {
        printf("Read from file: %s\n", buffer);
        DataPackage* pkg = DataPackage_create(i%255, buffer, read);
        int wr = llwrite(app->fd, pkg->buffer, pkg->dataSize+4);
        DataPackage_delete(&pkg);

        if (wr < 0) {
            printf("!!!ERROR::IO - Could not send package\n");
            res = -1;
            break;
        }
        written += wr;

        printf("%d/%d (%d\%%)\n", written, filesize, (written/filesize)*100);
    }

    if (fclose(file) != 0) {
        printf("!!!ERROR::FILE - Couldn't close file '%s'\n", app->filename);
        res = -2;
    }

    CtrlPackage* end = CtrlPackage_create(PKG_END, filesize, app->filename);
    llwrite(app->fd, end->buffer, end->bufSize);
    CtrlPackage_delete(&end);

    if (res > 0) printf("\n#\t#\t# File sent #\t#\t#\n\n");

    return res;
}

int AppLayer_receive(AppLayer* app) {
    int res = 0;

    char* pkg_buf;
    int pkgSize;

    // Get START Package
    pkgSize = llread(app->fd, &pkg_buf);
    CtrlPackage* pkg = CtrlPackage_from_buf(pkg_buf, pkgSize);

    if (pkg->type != PKG_START) {
        printf("!!!ERROR::CTRL_PKG - Not START package\n");
        CtrlPackage_delete(&pkg);
        free(pkg_buf);
        return -1;
    }

    uint expectedFileSize = atoi(pkg->params[CP_PARAM_SIZE].value);
    char* filename = pkg->params[CP_PARAM_NAME].value;

    // Open output file
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
    free(pkg_buf);

    uint currFileSize = 0;
    while (currFileSize != expectedFileSize) {
        pkgSize = llread(app->fd, &pkg_buf);
        if (pkgSize < 0) printf("!!!ERROR::IO - Could not receive package\n");

        pkg = CtrlPackage_from_buf(pkg_buf, pkgSize);
        if (pkg != NULL) { // If a control package
            if (pkg->type == PKG_END) {
                printf("!!!ERROR::CTRL_PKG - Unnexpected END package\n");
                free(pkg_buf);
                CtrlPackage_delete(&pkg);
                res = -1;
                break;
            }
        }
        CtrlPackage_delete(&pkg);

        DataPackage* data = DataPackage_from_buf(pkg_buf, pkgSize);
        if (data == NULL){
            printf("!!!ERROR::DATA_PKG - Expected DATA package\n");
            DataPackage_delete(&data);
            free(pkg_buf);
            res = -1;
            break;
        }
        fwrite(&data->buffer[4], sizeof(char), data->dataSize, file);
        /* printf("Wrote to file: \n"); */
        /* fwrite(&data->buffer[4], sizeof(char), data->dataSize, stdout); */

        currFileSize += data->dataSize;

        printf("%d/%d (%d\%%)\n", currFileSize, expectedFileSize, (currFileSize/expectedFileSize)*100);

        DataPackage_delete(&data);
        free(pkg_buf);
    }

    pkgSize = llread(app->fd, &pkg_buf);
    pkg = CtrlPackage_from_buf(pkg_buf, pkgSize);
    if (pkg == NULL || pkg->type != PKG_END) {
        printf("!!!ERROR::CTRL_PKG - Expected END package\n");
        res = -1;
    }
    CtrlPackage_delete(&pkg);
    free(pkg_buf);

    fclose(file);

    if (res > 0) printf("\n#\t#\t# File received #\t#\t#\n\n");

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

CtrlPackage* CtrlPackage_from_buf(const char* buf, uint size) {
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
        return;
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


DataPackage* DataPackage_create(int N, const char* data, uint size) {
    DataPackage* pkg = (DataPackage*) malloc(sizeof(DataPackage));
    pkg->type = PKG_DATA;
    pkg->N = N;
    pkg->L2 = size / 256;
    pkg->L1 = size % 256;

    pkg->buffer = (char*) malloc((size+4)*sizeof(char));
    pkg->buffer[0] = PKG_DATA; pkg->buffer[1] = N; pkg->buffer[2] = pkg->L2; pkg->buffer[3] = pkg->L1;
    memcpy(&pkg->buffer[4], data, size);

    pkg->dataSize = size;

    /* DataPackage_print(pkg); */
    return pkg;
}

DataPackage* DataPackage_from_buf(const char* buf, uint size) {
    if (buf[0] != PKG_DATA) return NULL;

    DataPackage* pkg = (DataPackage*) malloc(sizeof(DataPackage));

    pkg->type = buf[0];
    pkg->N = buf[1];
    pkg->L2 = buf[2];
    pkg->L1 = buf[3];

    uint dataSize = 256 * pkg->L2 + pkg->L1;

    pkg->buffer = (char*) malloc((dataSize+4)*sizeof(char));
    memcpy(pkg->buffer, buf, dataSize+4);

    pkg->dataSize = dataSize;

    /* DataPackage_print(pkg); */
    return pkg;
}

void DataPackage_delete(DataPackage** pkg) {
    if (*pkg == NULL) return;

    free((*pkg)->buffer);
    (*pkg)->buffer = NULL;

    free(*pkg);
    *pkg=NULL;
}

void DataPackage_print(DataPackage* pkg) {
    printf("------------------\n");
    switch(pkg->type) {
    case PKG_DATA:
        printf("DATA PKG\n");
        break;
    default:
        return;
    }

    printf("%d | %d | %d\n", pkg->N, pkg->L2, pkg->L1);
    fwrite(&pkg->buffer[4], sizeof(char), pkg->dataSize, stdout);

    printf("------------------\n");
}

int getFileSize(FILE* f) {
    int currPos = ftell(f);

    fseek(f, 0, SEEK_END);
    int size = ftell(f);
    fseek(f, 0, currPos);

    return size;
}
