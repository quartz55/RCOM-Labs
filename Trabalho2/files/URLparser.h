#ifndef __URLparser
#define __URLparser

#include <regex.h>

typedef char urlBuf[256];

typedef struct URL
{
	urlBuf user;
	urlBuf pass;
	urlBuf host;
	urlBuf ip;
	urlBuf path;
	urlBuf file;
	int port;
	
} url;

extern void initURL(url* url);
extern int URLparser(url* url, const char* str);
extern int validateURL(const char *URL,int *userMode,char *currentRegex,size_t nmatch,regmatch_t *pmatch);
extern char* processURLContent(char* str, char chr); 
extern int getIp(url* url); 
#endif
