#include <stdio.h>
#include "URLparser.h"

void errorMessage(char* argv0) {
	printf("Usage Anonymous: %s ftp://<host>/<url-path>\n",argv0);
	printf("Usage Normal: %s ftp://[<user>:<password>@]<host>/<url-path>\n",argv0);
}

int main(int argc, char** argv) {
	if (argc != 2) {
		printf("Wrong number of arguments.\n\n");
		errorMessage(argv[0]);
		return 1;
	}

	/* --------------- URL ------------------- */
	url url;
	initURL(&url);

	if (!URLparser(&url, argv[1]))
		return -1;

	if (!getIp(&url)) {
		printf("ERROR: Cannot find ip to hostname %s.\n", url.host);
		return -1;
	}

	printf("User: %s\n", url.user);
	printf("Password: %s\n", url.pass);
	printf("Host: %s\n", url.host);
	printf("Ip: %s\n", url.ip);
	printf("path: %s\n", url.path);
	printf("Filename: %s\n", url.file);

	return 0;
}