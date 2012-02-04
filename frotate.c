// Frotate - File rotation utility for embedded systems
// Copyright (C) 2012 Frederic Hoerni - fhoerni@free.fr
// Licensed under the GNU GENERAL PUBLIC LICENSE Version 3.
// This program comes with ABSOLUTELY NO WARRANTY.
// This is free software, and you are welcome to redistribute it
// under certain conditions.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

const int MAX_PATH = 2048;

void usage()
{
    fprintf(stderr, "Usage:\n"
            "    frotate [ --prefix <p> --size <s> -n <n> ]\n"
            "\n"
            "    Read line from stdin and write to <p>, <p>.1, <p>.2, etc.\n"
            "    The output files are at most <s> bytes.\n"
            "    The number of output files is at most <n>.\n"
            "\n"
            "Default values: frotate --prefix frotate.log --size 524288 -n 3\n" 
            );
    exit(1);
}

int openFile(const char *prefix)
{
    int fd = open(prefix, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR);
    if (fd < 0) {
        fprintf(stderr, "Cannot open %s: %s\n", prefix, strerror(errno));
        exit(1);
    }
    return fd;
}

void buildPath(char *output, int maxSize, const char *prefix, int i) {
    // concatenates prefix "." i
    int n = 0;
    if (0 == i) {
        // do not use ".0"
        n = snprintf(output, maxSize, "%s", prefix);
    } else {
        n = snprintf(output, maxSize, "%s.%d", prefix, i);
    }

    if (n >= MAX_PATH) {
        // not enough space
        fprintf(stderr, "Cannot build path with i=%d, prefix=%s\n", i, prefix);
        exit(1);
    }

}

void rotateFiles(const char *prefix, int maxFiles)
{
    // renames <prefix>.<n-1> -> <prefix>.<n>
    //                          (<prefix>.<n> is therefore lost)
    // ...
    // renames <prefix>.<2> -> <prefix>.<3>
    // renames <prefix>.<1> -> <prefix>.<2>
    // renames <prefix>     -> <prefix>.<1>
    //
    int i = 0;
    char oldpath[MAX_PATH+1];
    char newpath[MAX_PATH+1];

    for (i=maxFiles-1; i>=1; i--) {
        buildPath(oldpath, MAX_PATH, prefix, i-1);
        buildPath(newpath, MAX_PATH, prefix, i);

        int result = rename(oldpath, newpath);
        if (result < 0) {
            // Ignore this error.
            // It is normal at the beginning when the oldpath
            // does not exist yet.
        }
    }
}

int main(int argc, char ** argv)
{
    int i = 0;
    char * prefix = "frotate.log";
    int maxSize = 512*1024; // bytes
    int maxFiles = 3;
    // parse command line
    for (i=1; i<argc; i++) {
        if (0 == strcmp("--prefix", argv[i])) {
            i++; // go to next argument
            if (i >= argc) usage();
            prefix = argv[i];
        } else if (0 == strcmp("--size", argv[i])) {
            i++; // go to next argument
            if (i >= argc) usage();
            maxSize = atoi(argv[i]);
        } else if (0 == strcmp("-n", argv[i])) {
            i++; // go to next argument
            if (i >= argc) usage();
            maxFiles = atoi(argv[i]);
        } else usage();
    }

    printf("%s --prefix %s --size %d -n %d\n", argv[0], prefix, maxSize, maxFiles);

    // check arguments
    if (maxSize <= 0) {
        fprintf(stderr, "Invalid argument --size %d\n", maxSize);
        exit(1);
    }
    if (maxFiles <= 0) {
        fprintf(stderr, "Invalid argument -n %d\n", maxFiles);
        exit(1);
    }
    if (strlen(prefix) == 0) {
        fprintf(stderr, "Invalid empty --prefix\n");
        exit(1);
    }
    if (strlen(prefix) > MAX_PATH-11) {
        // keep space for 10 digits + 1 dot.
        fprintf(stderr, "Prefix too long: must be less than %d (%s).\n",
                MAX_PATH-11, prefix);
        exit(1);
    }

    const int BUF_SIZE = 4096;
    unsigned char buffer[BUF_SIZE];
    
    int fd = openFile(prefix);

    // main loop
    while (1) {
        ssize_t n = read(0, buffer, BUF_SIZE);
        if (n < 0) {
            fprintf(stderr, "Input error: %s\n", strerror(errno));
        } else if (0 == n) {
            // end of file
            close(fd);
            break;
        } else {
            // write to output file
            write(fd, buffer, n);
            off_t size = lseek(fd, 0, SEEK_END);
            if (size >= maxSize) {
                // do rotation
                close(fd);
                rotateFiles(prefix, maxFiles);
                fd = openFile(prefix);
            }
        }
    }



}
