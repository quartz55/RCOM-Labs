#ifndef FTP_CLIENT_H
#define FTP_CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct FTP_t {
    int socket_fd;
    int data_fd;
} FTP;

extern int FTP_Connect(FTP* ftp, const char* ip, int port);
extern int FTP_Download(FTP* ftp, const char* filename, const int fileSize);
extern int FTP_Disconnect(FTP* ftp);
extern int FTP_Quit(FTP* ftp);

extern int FTP_Login(FTP* ftp, const char* user, const char* password);
extern int FTP_CWD(FTP* ftp, const char* path);
extern int FTP_PASSV(FTP* ftp);
extern int FTP_RETR(FTP* ftp, const char* filename);

extern int FTP_Send(FTP* ftp, const char* str, size_t size);
extern int FTP_Read(FTP* ftp, char* str, size_t size);

#endif /* FTP_CLIENT_H */
