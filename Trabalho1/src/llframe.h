#ifndef LLFRAME_H
#define LLFRAME_H

#include "utils.h"

#define MAX_SIZE 512
#define BAUDRATE B38400
#define DEVICE "/dev/pts/"

#define FLAG 0x7e
#define ESC 0x7d

typedef enum {
    C_SET = 0x07, C_DISC = 0x0b, C_UA = 0x03, C_RR = 0x01, C_REJ = 0x05
} LL_C;
typedef enum {
    A_COM_T = 0x03, A_COM_R = 0x01, A_ANS_T = 0x01, A_ANS_R = 0x03
} LL_A;

typedef enum {
    LL_FRAME_COMMAND,
    LL_FRAME_INFO,
    LL_FRAME_INVALID
} LLFrameType;

typedef struct {
    LLFrameType type;

    uint nr;
    uint ns;

    struct data{
        char* message;
        uint size;
    } data;
} LLFrame;

extern LLFrame* LLFrame_create_info(const char* data, uint dataSize, int ns);
extern LLFrame* LLFrame_create_command(LL_A a, LL_C controlField, int nr);
extern LLFrame* LLFrame_from_buf(const char buf[], uint size);
extern LLFrame* LLFrame_from_fd(int fd);
extern bool LLFrame_is_command(LLFrame* frame, LL_C comm);
extern bool LLFrame_is_invalid(LLFrame* frame);
extern void LLFrame_print(LLFrame* frame);
extern void LLFrame_print_msg(LLFrame* frame, const char msg[]);
extern void LLFrame_delete(LLFrame** frame);
extern char getBCC(const char buf[], uint size);

#endif /* LLFRAME_H */
