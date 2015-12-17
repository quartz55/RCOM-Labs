
production:
	gcc -Wall -O2 -o ftpdownload downloader.c URLparser.c FTPclient.c

all: production debug

debug:
	gcc -Wall -O2 -D DEBUG -o ftpdownload_debug downloader.c URLparser.c FTPclient.c

clean:
	rm -rf download
