#include "FTPclient.h"

#include <netdb.h>
#include <regex.h>
#include <errno.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include <netinet/in.h>

#define PROGRESS_BAR

static void printProgressBar(int a, int b);
static int connectSocket(const char* ip, int port);

int FTP_Connect(FTP* ftp, const char* ip, int port)
{
    int socket = connectSocket(ip, port);
    char ftpMessage[1024];

    if (socket < 0) {
        printf("!!!ERROR!!! Could not establish connection to socket.\n");
        return 1;
    }

    ftp->socket_fd = socket;
    ftp->data_fd = 0;

    if (FTP_Read(ftp, ftpMessage, sizeof(ftpMessage))) {
        printf("!!!ERROR!!! Didn't get response (connect).\n");
        return 1;
    }

    return 0;
}

int FTP_Download(FTP* ftp, const char* filename, const int fileSize)
{
    FILE* file;
    int bytes;

    if (!(file = fopen(filename, "w"))) {
        printf("!!!ERROR!!! Cannot open file to write.\n");
        return 1;
    }

    char buf[1024];
    int downloadedBytes = 0;
    printf("\n\t\t---- Downloading %s ----\n", filename);
    while ((bytes = read(ftp->data_fd, buf, sizeof(buf)))) {
        downloadedBytes += bytes;
#ifdef PROGRESS_BAR
        printProgressBar(downloadedBytes, fileSize);
#endif
        if (bytes < 0) {
            printf("!!!ERROR!!! Could not receive file data.\n");
            return 1;
        }

        if ((bytes = fwrite(buf, bytes, 1, file)) < 0) {
            printf("!!!ERROR!!! Could not write data to file.\n");
            return 1;
        }
    }
    printf("\n\n");

    fclose(file);
    close(ftp->data_fd);

    return 0;
}

int FTP_Disconnect(FTP* ftp)
{
    char buf[1024];

    if (FTP_Read(ftp, buf, sizeof(buf))) {
        printf("!!!ERROR!!! Could not get confirmation.\n");
        return 1;
    }

    sprintf(buf, "QUIT\r\n");
    if (FTP_Send(ftp, buf, strlen(buf))) {
        printf("!!!ERROR!!! Could not send QUIT.\n");
        return 1;
    }

    if (ftp->socket_fd) close(ftp->socket_fd);

    return 0;
}

int FTP_Login(FTP* ftp, const char* user, const char* password)
{
    char sd[1024];

    // username
    sprintf(sd, "USER %s\r\n", user);
    if (FTP_Send(ftp, sd, strlen(sd))) {
        printf("!!!ERROR!!! Could not send USER.\n");
        return 1;
    }

    if (FTP_Read(ftp, sd, sizeof(sd))) {
        printf("!!!ERROR!!! Access denied (Bad username).\n");
        return 1;
    }

    // cleaning buffer
    memset(sd, 0, sizeof(sd));

    // password
    sprintf(sd, "PASS %s\r\n", password);
    if (FTP_Send(ftp, sd, strlen(sd))) {
        printf("!!!ERROR!!! Could not send PASS.\n");
        return 1;
    }

    if (FTP_Read(ftp, sd, sizeof(sd))) {
        printf("!!!ERROR!!! Access denied (Bad password).\n");
        return 1;
    }

    return 0;
}

int FTP_CWD(FTP* ftp, const char* path)
{
    char cwd[1024];

    sprintf(cwd, "CWD %s\r\n", path);
    if (FTP_Send(ftp, cwd, strlen(cwd))) {
        printf("!!!ERROR!!! Could not send path to CWD.\n");
        return 1;
    }

    if (FTP_Read(ftp, cwd, sizeof(cwd))) {
        printf("!!!ERROR!!! Could not change working directory.\n");
        return 1;
    }

    return 0;
}

int FTP_PASSV(FTP* ftp)
{
    char pasv[1024] = "PASV\r\n";
    if (FTP_Send(ftp, pasv, strlen(pasv))) {
        printf("!!!ERROR!!! Could not enter in passive mode\n");
        return 1;
    }

    if (FTP_Read(ftp, pasv, sizeof(pasv))) {
        printf(
            "!!!ERROR!!! Did not receive any response (passive mode).\n");
        return 1;
    }

    int ipPart1, ipPart2, ipPart3, ipPart4;
    int port1, port2;
    if ((sscanf(pasv, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &ipPart1,
                &ipPart2, &ipPart3, &ipPart4, &port1, &port2)) < 0) {
        printf("!!!ERROR!!! Could not get IP and PORT.\n");
        return 1;
    }

    memset(pasv, 0, sizeof(pasv));

    if ((sprintf(pasv, "%d.%d.%d.%d", ipPart1, ipPart2, ipPart3, ipPart4)) <
        0) {
        printf("!!!ERROR!!! Could not create IP address.\n");
        return 1;
    }

    int portResult = port1 * 256 + port2;

#ifdef DEBUG
    printf("IP: %s\n", pasv);
    printf("PORT: %d\n", portResult);
#endif

    if ((ftp->data_fd = connectSocket(pasv, portResult)) < 0) {
        printf(
            "!!!ERROR!!! Incorrect file descriptor associated to ftp data "
            "socket "
            "fd.\n");
        return 1;
    }

    return 0;
}

int FTP_RETR(FTP* ftp, const char* filename)
{
    char retr[1024];

    sprintf(retr, "RETR %s\r\n", filename);
    if (FTP_Send(ftp, retr, strlen(retr))) {
        printf("!!!ERROR!!! Could not send filename.\n");
        return -1;
    }

    if (FTP_Read(ftp, retr, sizeof(retr))) {
        printf("!!!ERROR!!! Didn't get response (filename).\n");
        return -1;
    }

    char* beg = strchr(retr, '(') + 1;
    char* end = strchr(beg, ' ');
    int numBytes = (end - retr) - (beg - retr);
    char size_str[numBytes];
    strncpy(size_str, beg, numBytes);
    int size = atoi(size_str);

    return size;
}

int FTP_Send(FTP* ftp, const char* str, size_t size)
{
    int bytes;

    if ((bytes = write(ftp->socket_fd, str, size)) <= 0) {
        printf("##WARNING## Nothing was sent.\n");
        return 1;
    }

#ifdef DEBUG
    char msg[size - 1];
    strncpy(msg, str, size - 2);
    msg[size - 2] = '\0';
    printf("\n\tSent: '%s' (%d bytes)\n\n", msg, bytes);
#endif

    return 0;
}

int FTP_Read(FTP* ftp, char* str, size_t size)
{
    FILE* fp = fdopen(ftp->socket_fd, "r");

    do {
        memset(str, 0, size);
        str = fgets(str, size, fp);
#ifdef DEBUG
        printf("%s", str);
#endif
    } while (!('1' <= str[0] && str[0] <= '5') || str[3] != ' ');

    return 0;
}

/* Utils */

static int connectSocket(const char* ip, int port)
{
    int sockfd;
    struct sockaddr_in server_addr;

    // server address handling
    bzero((char*)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    // 32 bit Internet address network byte ordered
    server_addr.sin_addr.s_addr = inet_addr(ip);
    // server TCP port must be network byte ordered
    server_addr.sin_port = htons(port);

    // open an TCP socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        return -1;
    }

    // connect to the server
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) <
        0) {
        perror("connect()");
        return -1;
    }

    return sockfd;
}

static void printProgressBar(int a, int b)
{
    int barSize = 50;
    float perc = (float)a / b;
    printf("\r[");
    uint i;
    for (i = 0; i < barSize; ++i) {
        if (i < (int)(perc * barSize))
            printf("=");
        else if (i == (int)(perc * barSize))
            printf(">");
        else
            printf(" ");
    }

    printf("] %.0f%% %d/%d (bytes)", perc * 100, a, b);
    fflush(stdout);
}
