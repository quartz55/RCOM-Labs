#ifndef LINK_DATA_H
#define LINK_DATA_H

#define MAX_SIZE 512
#define BAUDRATE B38400
#define DEVICE "/dev/pts/"

/*
  INFO FRAME

  | F | A | C | BCC1 | DATA --- DATA | BCC2 | F |


  SUPERVISION FRAMES

  | F | A | C | BCC1 | F |

*/
#define FLAG 0x7e
#define A_COM_TRANS 0x03
#define A_ANS_TRANS 0x01
#define A_COM_RECEIV 0x01
#define A_ANS_RECEIV 0x03
#define C_SET 0x07
#define C_DISC 0x0b
#define C_UA 0x03

static const char SET[] = {FLAG, A_COM_TRANS, C_SET, A_COM_TRANS^C_SET, FLAG};
static const char DISC_R[] = {FLAG, A_ANS_RECEIV, C_DISC, A_ANS_RECEIV^C_DISC, FLAG};
static const char DISC_T[] = {FLAG, A_COM_TRANS, C_SET, A_COM_TRANS^C_DISC, FLAG};
static const char UA[] = {FLAG, A_ANS_RECEIV, C_UA, A_ANS_RECEIV^C_UA, FLAG};


#endif /* LINK_DATA_H */
