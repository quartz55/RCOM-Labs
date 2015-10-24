#include "llframe.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

LLFrame* LLFrame_create_info(const char* data, uint dataSize, int ns) {
    LLFrame* f = (LLFrame *) malloc(sizeof(LLFrame));

    f->type = LL_FRAME_INFO;
    f->ns = f->nr = -1;

    char* msg = (char *) malloc(dataSize*sizeof(char) + 6*sizeof(char));

    msg[0] = FLAG; msg[1] = A_COM_T; msg[2] = ns << 5;
    msg[3] = msg[1] ^ msg[2];
    memcpy(&msg[4], data, dataSize);
    msg[4 + dataSize] = getBCC(data, dataSize);
    msg[4 + dataSize + 1] = FLAG;

    f->data.message = msg;
    f->data.size = 6 + dataSize;

    LLFrame_print_msg(f, "Info created: ");

    return f;
}

LLFrame* LLFrame_create_command(LL_A a, LL_C controlField, int nr) {
    LLFrame* f = (LLFrame *) malloc(sizeof(LLFrame));

    f->type = LL_FRAME_COMMAND;
    f->ns = f->nr = -1;

    char* msg = (char *) malloc(5*sizeof(char));

    msg[0] = FLAG; msg[1] = a; msg[2] = controlField;

    if (controlField == C_RR || controlField == C_REJ)
        msg[2] |= (nr << 5);

    msg[3] = msg[1] ^ msg[2];
    msg[4] = FLAG;

    f->data.message = msg;
    f->data.size = 5;

    LLFrame_print_msg(f, "Command created: ");

    return f;
}

LLFrame* LLFrame_from_buf(const char buf[], uint size) {
    bool done = false;

    enum State{ A_STATE, C_STATE, BCC_STATE, DATA_STATE, BCC2_STATE, ERROR_STATE };

    enum State state = A_STATE;

    uint index = 0;
    char A, C;
    char* data;
    uint dataIndex = 0;

    LLFrame* frame;
    while (!done) {
        switch (state) {
        case A_STATE:
            A = buf[index++];
            state = C_STATE;
            break;
        case C_STATE:
            C = buf[index++];
            state = BCC_STATE;
            break;
        case BCC_STATE:
            if (buf[index] != (A^C)) {
                printf("!!!ERROR::BCC - Not valid\n");
                state = ERROR_STATE;
            }
            else {
                if(++index >= size) {
                    frame = LLFrame_create_command(A, C, 0);
                    return frame;
                }
                data = (char *) malloc((size-4)*sizeof(char));
                state = DATA_STATE;
            }
            break;
        case DATA_STATE:
            if (index > size-1) {
                state = BCC2_STATE;
            }
            else {
                data[dataIndex++] = buf[index++];
            }
            break;
        case BCC2_STATE:
            if (buf[index] == getBCC(data, dataIndex)) done = true;
            else {
                printf("!!!ERROR::BCC2 - Not valid\n");
                state = ERROR_STATE;
            }
            break;
        case ERROR_STATE: {
            frame = (LLFrame*) malloc(sizeof(LLFrame));
            frame->type = LL_FRAME_INVALID;
            return frame;
            break;
        }
        }
    }

    int ns = C & 0xF0;
    frame = LLFrame_create_info(data, dataIndex, ns);
    return frame;
 }

LLFrame* LLFrame_from_fd(int fd) {
    int res = -1;

    char c;
    read(fd, &c, sizeof(c));
    while (c == FLAG) read(fd, &c, sizeof(c));

    uint size = 0;
    char buf[MAX_SIZE];
    do {
        buf[size] = c;
        res = read(fd, &c, sizeof(c));
        size += res;
    } while (c != FLAG && size < MAX_SIZE);

    LLFrame* frame = LLFrame_from_buf(buf, size);
    return frame;
}

bool LLFrame_is_invalid(LLFrame* frame) {
    if (frame == NULL) return true;
    return frame->type == LL_FRAME_INVALID;
}

bool LLFrame_is_command(LLFrame* frame, LL_C comm) {
    if (!LLFrame_is_invalid(frame))
        return (frame->type == LL_FRAME_COMMAND) && (frame->data.message[2] == comm);
    return false;
}

void LLFrame_print(LLFrame* frame) {
    if (LLFrame_is_invalid(frame)) return;
    if (frame->type == LL_FRAME_COMMAND || frame->type == LL_FRAME_INFO) {
        uint i;
        printf("| ");
        for (i = 0; i < frame->data.size; ++i)
            printf("%X | ", frame->data.message[i]);

        printf("(%d bytes)\n", frame->data.size);
    }
}

void LLFrame_print_msg(LLFrame* frame, const char msg[]) {
    if (LLFrame_is_invalid(frame)) return;
    printf("%s ", msg);
    LLFrame_print(frame);
}

void LLFrame_delete(LLFrame** frame) {
    if (*frame == NULL) return;
    free((*frame)->data.message);
    (*frame)->data.message = NULL;
    free(*frame);
    *frame = NULL;
}

char getBCC(const char buf[], uint size) {
    char BCC = 0;
    uint i;
    for (i = 0; i < size; ++i) {
        BCC ^= buf[i];
    }

    return BCC;
}
