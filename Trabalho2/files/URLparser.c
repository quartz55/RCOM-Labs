#include "URLparser.h"

#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <pcre.h>
#include <stdio.h>
#include <stdlib.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/socket.h>

const char* regExp =
    "ftp://([([A-Za-z0-9])*:([A-Za-z0-9])*@])*([A-Za-z0-9.~-])+/([[A-Za-z0-9/"
    "~._-])+";

const char* regExpUserAnonymous =
    "ftp://([A-Za-z0-9.~-])+/([[A-Za-z0-9/~._-])+";

void initURL(URL* url)
{
    memset(url->user, 0, sizeof(urlBuf));
    memset(url->pass, 0, sizeof(urlBuf));
    memset(url->host, 0, sizeof(urlBuf));
    memset(url->ip, 0, sizeof(urlBuf));
    memset(url->path, 0, sizeof(urlBuf));
    memset(url->file, 0, sizeof(urlBuf));
    url->port = 21;
}

int URLparser(URL* url, const char* urlStr)
{
    char *tempURL, *URLcontent, *currentRegex = NULL;
    int userMode;
    size_t nmatch = strlen(urlStr);
    regmatch_t pmatch[nmatch];

    URLcontent = (char*)malloc(strlen(urlStr));
    tempURL = (char*)malloc(strlen(urlStr));

    memcpy(tempURL, urlStr, strlen(urlStr));

    validateURL(tempURL, &userMode, currentRegex, nmatch, pmatch);

    // remove ftp:// from string
    strcpy(tempURL, tempURL + 6);

    if (userMode) {
        // remove [ from string
        strcpy(tempURL, tempURL + 1);

        // save username
        strcpy(URLcontent, processURLContent(tempURL, ':'));
        memcpy(url->user, URLcontent, strlen(URLcontent));

        // save password
        strcpy(URLcontent, processURLContent(tempURL, '@'));
        memcpy(url->pass, URLcontent, strlen(URLcontent));

        // remove ] from string
        strcpy(tempURL, tempURL + 1);
    }

    // save host
    strcpy(URLcontent, processURLContent(tempURL, '/'));
    memcpy(url->host, URLcontent, strlen(URLcontent));

    // save url path
    char* path = (char*)malloc(strlen(tempURL));
    while (strchr(tempURL, '/') != NULL) {
        URLcontent = processURLContent(tempURL, '/');

        strcat(path, URLcontent);

        strcat(path, "/");
    }
    memcpy(url->path, path, strlen(path));

    // save filename
    memcpy(url->file, tempURL, strlen(tempURL));

    free(tempURL);
    free(URLcontent);

    return 1;
}

int validateURL(const char* URL, int* userMode, char* currentRegex,
                size_t nmatch, regmatch_t* pmatch)
{
    if (URL[6] == '[') {
        *userMode = 1;
        currentRegex = (char*)regExp;
    } else {
        *userMode = 0;
        currentRegex = (char*)regExpUserAnonymous;
    }

    regex_t* regex = (regex_t*)malloc(sizeof(regex_t));

    int reti;
    if ((reti = regcomp(regex, currentRegex, REG_EXTENDED)) != 0) {
        perror("URL format is wrong.");
        return 0;
    }

    if ((reti = regexec(regex, URL, nmatch, pmatch, REG_EXTENDED)) != 0) {
        perror("URL could not execute.");
        return 0;
    }

    free(regex);
    return 1;
}

char* processURLContent(char* str, char chr)
{
    char* tempStr = (char*)malloc(strlen(str));
    int end = strlen(str) - strlen(strcpy(tempStr, strchr(str, chr)));

    int i;
    for (i = 0; i < end; i++) {
        tempStr[i] = str[i];
    }

    // termination char
    tempStr[end] = '\0';

    strcpy(str, str + strlen(tempStr) + 1);

    return tempStr;
}

int getIp(URL* url)
{
    struct hostent* h;

    if ((h = gethostbyname(url->host)) == NULL) {
        herror("gethostbyname");
        return 0;
    }

    char* ip = inet_ntoa(*((struct in_addr*)h->h_addr));
    strcpy(url->ip, ip);

    return 1;
}
