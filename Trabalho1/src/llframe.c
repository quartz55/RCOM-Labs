#include "llframe.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

LLFrame* LLFrame_create_info(const char* data, uint dataSize, int ns) {
    LLFrame* f = (LLFrame *) malloc(sizeof(LLFrame));

    f->type = LL_FRAME_INFO;
    f->nr = -1;
    f->ns = ns;

    f->data.size = 6 + dataSize;
    char* msg = (char *) malloc(f->data.size * sizeof(char));

    msg[0] = FLAG; msg[1] = A_COM_T; msg[2] = (0x00) | (ns << 5);
    msg[3] = msg[1] ^ msg[2];
    memcpy(&msg[4], data, dataSize);
    msg[4 + dataSize] = getBCC(data, dataSize);
    msg[4 + dataSize + 1] = FLAG;


    f->data.message = msg;

    if (DEBUG) {
        printf("Info %d created\n", f->ns);
    }

    return f;
}

LLFrame* LLFrame_create_command(LL_A a, LL_C controlField, int nr) {
    LLFrame* f = (LLFrame *) malloc(sizeof(LLFrame));

    f->type = LL_FRAME_COMMAND;
    f->ns = -1;
    f->nr = nr;

    char* msg = (char *) malloc(5*sizeof(char));

    msg[0] = FLAG; msg[1] = a; msg[2] = controlField;

    if (controlField == C_RR || controlField == C_REJ)
        msg[2] |= (nr << 5);

    msg[3] = msg[1] ^ msg[2];
    msg[4] = FLAG;

    f->data.message = msg;
    f->data.size = 5;

    if (DEBUG) {
        print_command(controlField);
        LLFrame_print_msg(f, "Command created: ");
    }

    return f;
}

LLFrame* LLFrame_from_buf(const char buf[], uint size) {
    bool done = false;

    enum State{ A_STATE, C_STATE, BCC_STATE, DATA_STATE, BCC2_STATE, ERROR_STATE };

    enum State state = A_STATE;

    uint index = 0;
    char A, C;
    char *data = (char *) malloc((size-4)*sizeof(char));
    uint dataIndex = 0;

    LLFrame* frame;
    LLError err = LL_ERROR_NONE;
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
                err = LL_ERROR_BCC;
                state = ERROR_STATE;
            }
            else {
                if(index+1 >= size) {
                    LL_C ll_c = 0x0F & C;
                    uint nr = C >> 5;
                    frame = LLFrame_create_command(A, ll_c, nr);
                    free(data);
                    return frame;
                }
                state = DATA_STATE;
                ++index;
            }
            break;
        case DATA_STATE:
            if (index+1 >= size) {
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
                err = LL_ERROR_BCC2;
                state = ERROR_STATE;
            }
            break;
        case ERROR_STATE: {
            int ns = C & 0xF0;
            frame = (LLFrame*) malloc(sizeof(LLFrame));
            frame->type = LL_FRAME_INVALID;
            frame->error = err;
            frame->ns = ns;
            free(data);
            return frame;
            break;
        }
        }
    }

    int ns = (C & 0xF0) >> 5;
    frame = LLFrame_create_info(data, dataIndex, ns);
    return frame;
}

LLFrame* LLFrame_from_fd(int fd) {
    int res = -1;

    char c;
    uint size;
    uint size_inc = 1;
    char* buf = (char*) malloc(MAX_SIZE*sizeof(char));

skipflags:
    read(fd, &c, sizeof(c));
    while (c == FLAG) read(fd, &c, sizeof(c));

    size = 0;
    do {
        buf[size] = c;
        res = read(fd, &c, sizeof(c));
        if (res < 0) perror("!!!ERROR::IO");
        size += res;
        if (size >= MAX_SIZE)
            buf = (char*) realloc(buf, ++size_inc*MAX_SIZE*sizeof(char));
    } while (c != FLAG);
    if (size < 3) goto skipflags;

    int destuff_size = destuff_buffer(&buf, size);
    /* printf("Read %d bytes (%d after destuff)\n", size+2, destuff_size+2); */

    LLFrame* frame = LLFrame_from_buf(buf, destuff_size);
    free(buf);

    return frame;
}

int LLFrame_write(LLFrame* frame, int fd) {
    int written = 0;

    char *stuffed = (char*) malloc(frame->data.size*sizeof(char));
    memcpy(stuffed, frame->data.message, frame->data.size);

    int stuffed_size = stuff_buffer(&stuffed, frame->data.size);

    /* LLFrame test; */
    /* test.type = LL_FRAME_COMMAND; */
    /* test.data.message = stuffed; */
    /* test.data.size = stuffed_size; */

    written = write(fd, stuffed, stuffed_size);
    /* printf("Write %d bytes\n", written); */

    free(stuffed);

    if (frame->type == LL_FRAME_COMMAND) return 0;
    else if (frame->type == LL_FRAME_INFO) return frame->data.size - 6;

    return written;
}

int LLFrame_get_data(LLFrame *frame, char** buff) {
    if (frame->type != LL_FRAME_INFO) return -1;

    int dataSize = frame->data.size-6;
    *buff = (char*) malloc(dataSize*sizeof(char));
    memcpy(*buff, &frame->data.message[4], dataSize);

    return dataSize;
}

bool LLFrame_is_invalid(LLFrame* frame) {
    if (frame == NULL) return true;
    return frame->type == LL_FRAME_INVALID;
}

bool LLFrame_is_command(LLFrame* frame, LL_C comm) {
    if (!LLFrame_is_invalid(frame))
        return (frame->type == LL_FRAME_COMMAND) && ((frame->data.message[2] & 0x0F) == comm);
    return false;
}

void LLFrame_print(LLFrame* frame) {
    if (LLFrame_is_invalid(frame)) return;
    if (frame->type == LL_FRAME_COMMAND) {
        uint i;
        printf("| ");
        for (i = 0; i < frame->data.size; ++i)
            printf("%X | ", frame->data.message[i]);

    }
    else if (frame->type == LL_FRAME_INFO) {
        uint i;
        printf("| ");
        for (i = 0; i < 4; ++i)
            printf("%X | ", frame->data.message[i]);

        for (i = 4; i < frame->data.size-2; ++i)
            printf("%c | ", frame->data.message[i]);

        printf("%X | %X | ",
               frame->data.message[frame->data.size-2],
               frame->data.message[frame->data.size-1]);

    }
    printf("(%d bytes)\n", frame->data.size);
}

void LLFrame_print_msg(LLFrame* frame, const char msg[]) {
    if (LLFrame_is_invalid(frame)) return;
    printf("%s ", msg);
    LLFrame_print(frame);
}

void LLFrame_print_data(LLFrame *frame) {
    if (frame->type != LL_FRAME_INFO) return;

    uint i = 0;
    for (i = 4; i < frame->data.size-2; ++i) {
        printf("%c", frame->data.message[i]);
    }
}

void LLFrame_delete(LLFrame** frame) {
    if (*frame == NULL) return;
    free((*frame)->data.message);
    (*frame)->data.message = NULL;
    free(*frame);
    *frame = NULL;
}

int stuff_buffer(char** buffer, uint size) {
    int newSize = size;

    uint i;
    for (i = 1; i < size-1; ++i) {
        if ((*buffer)[i] == FLAG || (*buffer)[i] == ESC) {
            ++newSize;
        }
    }

    (*buffer) = (char*) realloc((*buffer), newSize*sizeof(char));

    for (i = 1; i < size-1; ++i) {
        if ((*buffer)[i] == FLAG || (*buffer)[i] == ESC) {
            memmove((*buffer)+(i+1), (*buffer)+i, size-i);

            (*buffer)[i] = ESC;
            (*buffer)[i+1] ^= 0x20;

            ++size;
        }
    }

    return newSize;
}

int destuff_buffer(char** buffer, uint size) {
    int newSize = size;

    uint i;
    for (i = 1; i < newSize -1; ++i) {
        if ((*buffer)[i] == ESC) {
            memmove((*buffer)+i, (*buffer)+(i+1), newSize-(i+1));

            (*buffer)[i] ^= 0x20;

            --newSize;
        }
    }

    (*buffer) = (char*) realloc((*buffer), newSize);
    return newSize;
}

char getBCC(const char buf[], uint size) {
    char BCC = 0;
    uint i;
    for (i = 0; i < size; ++i) {
        BCC ^= buf[i];
    }

    return BCC;
}

void print_command(LL_C c) {
    switch (c) {
    case C_SET:
        printf("[SET] ");
        break;
    case C_DISC:
        printf("[DISC] ");
        break;
    case C_UA:
        printf("[UA] ");
        break;
    case C_REJ:
        printf("[REJ] ");
        break;
    case C_RR:
        printf("[RR] ");
        break;
    default:
        printf("[INVALID] ");
        break;
    }
}
