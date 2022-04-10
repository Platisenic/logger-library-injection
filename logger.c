#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define DFT_SO_PATH "./logger.so"

int checkSepExist(int argc, char * argv[]) {
    for(int i=1; i<argc; i++) {
        if(strcmp(argv[i], "--") == 0) {
            return i;
        }
    }
    return 0;
}

int checkFirstArgIsCMD(int argc, char * argv[]) {
    return (argv[1][0] != '-');
}

void printMsg() {
    fprintf(stderr, "usage: ./logger [-o file] [-p sopath] [--] cmd [cmd args ...]\n"
                    "\t-p: set the path to logger.so, default = ./logger.so\n"
                    "\t-o: print output to file, print to \"stderr\" if no file specified\n"
                    "\t--: separate the arguments for logger and for the command\n");
}

int main(int argc, char * argv[]) {
    if (argc < 2) {
        printMsg();
        exit(EXIT_FAILURE);
    }

    char *loggersoPath = DFT_SO_PATH;
    char *outputFilePath = NULL;
    int logfd = STDERR_FILENO;
    int sepPos = checkSepExist(argc, argv);
    int opt;
    opterr = 0;
    if (sepPos == 0) {
        if (!checkFirstArgIsCMD(argc, argv)) {
            sepPos = argc - 2;
            while ((opt = getopt(argc-1, argv, "p:o:")) != -1 ) {
                switch (opt) {
                    case 'p':
                        loggersoPath = optarg;
                        break;
                    case 'o':
                        outputFilePath = optarg;
                        break;
                    default:
                        printMsg();
                        exit(EXIT_FAILURE);
                }
            }
        }
    } else {
        while ((opt = getopt(sepPos, argv, "p:o:")) != -1 ) {
            switch (opt) {
                case 'p':
                    loggersoPath = optarg;
                    break;
                case 'o':
                    outputFilePath = optarg;
                    break;
                default:
                    printMsg();
                    exit(EXIT_FAILURE);
            }
        }
    }

    if (outputFilePath != NULL) {
        logfd = open(outputFilePath, O_CREAT|O_WRONLY|O_TRUNC, 0664);
    } else {
        logfd = dup(STDERR_FILENO);
    }

    if (logfd == -1) {
        printMsg();
        exit(EXIT_FAILURE);
    }

    char logfd_str[5];
    memset(logfd_str, 0, 5);
    sprintf(logfd_str, "%d", logfd);

    setenv("LD_PRELOAD", loggersoPath, 1);
    setenv("LOGGER_FD", logfd_str, 1);
    execvp(argv[sepPos+1], argv+(sepPos+1));
    close(logfd);

    return 0;
}

