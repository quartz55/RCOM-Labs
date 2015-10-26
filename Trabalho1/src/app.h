#ifndef APP_H
#define APP_H

#include "utils.h"

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
} APP_DataPackage;

extern AppLayer* AppLayer_constructor(int port, ConnectionFlag status, char* filename);
extern int AppLayer_start_transfer(AppLayer* app);
extern void AppLayer_delete(AppLayer** app);

extern int AppLayer_receive(AppLayer* app);
extern int AppLayer_send(AppLayer* app);


extern CtrlPackage* CtrlPackage_create(PackageType type, int filesize, char* filename);
extern CtrlPackage* CtrlPackage_from_buf(char* buf, uint size);
extern void CtrlPackage_delete(CtrlPackage** pkg);
extern void CtrlPackage_print(CtrlPackage* pkg);

#endif /* APP_H */
