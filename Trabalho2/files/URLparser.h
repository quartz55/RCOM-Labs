#ifndef URLPARSER_H
#define URLPARSER_H

#include <regex.h>

typedef char urlBuf[256];

typedef struct URL_t {
    urlBuf user;
    urlBuf pass;
    urlBuf host;
    urlBuf ip;
    urlBuf path;
    urlBuf file;
    int port;
} URL;

extern void initURL(URL* url);
extern int URLparser(URL* url, const char* str);
extern int validateURL(const char* URL, int* userMode, char* currentRegex,
                       size_t nmatch, regmatch_t* pmatch);
extern char* processURLContent(char* str, char chr);
extern int getIp(URL* url);

#endif /* URLPARSER_H */
