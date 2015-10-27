#ifndef APP_H
#define APP_H

#include "utils.h"
#include <stdio.h>

typedef enum {
    PKG_DATA, PKG_START, PKG_END
} PackageType;

#define CP_NUM_PARAMS 2
typedef enum {
    CP_PARAM_SIZE, CP_PARAM_NAME
} CP_ParamType;

typedef struct {
    int fd;
    ConnectionFlag status;
    char filename[100];
} AppLayer;

typedef struct {
    PackageType type;

    struct CP_Param {
        CP_ParamType type;
        uint length;
        char* value;
    } params[CP_NUM_PARAMS];

    char* buffer;
    uint bufSize;

} CtrlPackage;

typedef struct {
    PackageType type;
    char N, L2, L1;
    uint dataSize;
    char* buffer;
} DataPackage;

extern AppLayer* AppLayer_constructor(int port, ConnectionFlag status, char* filename);
extern int AppLayer_start_transfer(AppLayer* app);
extern void AppLayer_delete(AppLayer** app);

extern int AppLayer_receive(AppLayer* app);
extern int AppLayer_send(AppLayer* app);


extern CtrlPackage* CtrlPackage_create(PackageType type, int filesize, char* filename);
extern CtrlPackage* CtrlPackage_from_buf(const char* buf, uint size);
extern void CtrlPackage_delete(CtrlPackage** pkg);
extern void CtrlPackage_print(CtrlPackage* pkg);

extern DataPackage* DataPackage_create(int N, const char* data, uint size);
extern DataPackage* DataPackage_from_buf(const char* buf, uint size);
extern void DataPackage_delete(DataPackage** pkg);
extern void DataPackage_print(DataPackage* pkg);

extern int getFileSize(FILE* f);

#endif /* APP_H */
