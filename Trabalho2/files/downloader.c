#include <stdio.h>
#include "URLparser.h"
#include "FTPclient.h"

int main(int argc, char** argv)
{
    if (argc != 2) {
        printf("Usage\n");
        printf("\tAnonymous: %s ftp://<host>/<url-path>\n", argv[0]);
        printf("\tNormal: %s ftp://[<user>:<password>@]<host>/<url-path>\n",
               argv[0]);
        return 1;
    }
    
    /* --------------- URL Parser ------------------- */

    URL url;
    initURL(&url);

    if (!URLparser(&url, argv[1])) return -1;

	printf("Parsed url\n");

    if (!getIp(&url)) {
        printf("!!!ERROR!!! Cannot find IP to hostname %s.\n", url.host);
        return -1;
    }

    char* user = "anonymous";
    if (strlen(url.user) != 0) {
        user = url.user;
    }

    char* password;
    if (strlen(url.pass)) {
        password = url.pass;
    } else {
        printf("You are now entering in *anonymous* mode.\n");
    }

    printf("\n");
    printf("+-----------------------\n");
    printf("| Connection info\n");
    printf("+-----------------------\n");
    if (strcmp(user, "anonymous")) {
        printf("| User: %s\n", url.user);
        printf("| Password: ");
        int i;
        for (i = 0;  i < strlen(url.pass);  i++, printf("%c", '*'));
        printf("\n");
    }
    printf("| Host: %s\n", url.host);
    printf("| Ip: %s\n", url.ip);
    printf("| path: %s\n", url.path);
    printf("| Filename: %s\n", url.file);
    printf("+-----------------------\n");
    printf("\n");

    /*------------------- FTP Client -------------------------*/

    FTP ftp;
    FTP_Connect(&ftp, url.ip, url.port);

    if (FTP_Login(&ftp, user, password)) {
        printf("!!!ERROR!!! Could not login user %s\n", user);
        return -1;
    }

    if (FTP_CWD(&ftp, url.path)) {
        printf("!!!ERROR!!!"
            "Could not change working directory to the folder of %s\n",
            url.file);
        return -1;
    }

    if (FTP_PASSV(&ftp)) {
        printf("!!!ERROR!!! Could not enter in passive mode\n");
        return -1;
    }

    int fileSize = FTP_RETR(&ftp, url.file);
    if (fileSize > 0) {
        FTP_Download(&ftp, url.file, fileSize);
	FTP_Disconnect(&ftp);
    }
    else {
	FTP_Quit(&ftp);
	return 1;
    }

    return 0;
}
