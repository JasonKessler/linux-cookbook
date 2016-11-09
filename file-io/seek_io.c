/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2016.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/*
  New comments written by: Shyamal S. Chandra, 2016
*/

/* Listing 4-3 */

#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    // size_t: unsigned integer: "Size of an object in bytes"
    size_t len;
    // off_t: signed integer: "File offset or size"
    off_t offset;
    int fd, ap, j;
    char *buf;
    // ssize_t: signed integer: "...Byte count or (negative) error indication..."
    ssize_t numRead, numWritten;

    if (argc < 3 || strcmp(argv[1], "--help") == 0)
        /*
          static void usageError(char *progName) {
            fprintf(stderr, "Usage: %s [-x] file...\n", progName);
            exit(EXIT_FAILURE);
          }
        */
        usageErr("%s file {r<length>|R<length>|w<string>|s<offset>}...\n",
                 argv[0]);

    // open(const char *pathname, int flags, mode_t mode)
    // Source: http://man7.org/linux/man-pages/man2/open.2.html
    fd = open(argv[1], O_RDWR | O_CREAT,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                S_IROTH | S_IWOTH);                     /* rw-rw-rw- */
    if (fd == -1)
        errExit("open");

    for (ap = 2; ap < argc; ap++) {
        switch (argv[ap][0]) {
        case 'r':   /* Display bytes at current offset, as text */
        case 'R':   /* Display bytes at current offset, in hex */

            /*
            long getLong(const char *arg, int flags, const char *name) {
              return getNum("getLong", arg, flags, name);
            }
            GN_ANY_BASE: Can use any base - like strtol(3)
            strtol(3): long int strtol(const char *nptr, char **endptr, int base)
            "...The strtol() function converts the initial part of the string in nptr
            to a long integer value according to the given base, which must be
            between 2 and 36 inclusive, or be the special value 0..."
            Source: http://man7.org/linux/man-pages/man3/strtol.3.html
            */
            len = getLong(&argv[ap][1], GN_ANY_BASE, argv[ap]);

            //"...The malloc() function allocates size bytes and returns a pointer to
            //the allocated memory..."
            // Source: http://man7.org/linux/man-pages/man3/malloc.3.html
            buf = malloc(len);
            if (buf == NULL)
                errExit("malloc");

            /*
              ssize_t read(int fd, void *buf, size_t count);
              Description:
                "...read() attempts to read up to count bytes from file descriptor fd
                into the buffer starting at buf..."
              Source: http://man7.org/linux/man-pages/man2/read.2.html
            */
            numRead = read(fd, buf, len);

            if (numRead == -1)
                errExit("read");

            if (numRead == 0) {
                printf("%s: end-of-file\n", argv[ap]);
            } else {
                printf("%s: ", argv[ap]);
                for (j = 0; j < numRead; j++) {
                    if (argv[ap][0] == 'r')
                        printf("%c", isprint((unsigned char) buf[j]) ?
                                                buf[j] : '?');
                    else
                        printf("%02x ", (unsigned int) buf[j]);
                }
                printf("\n");
            }

            free(buf);
            break;

        case 'w':   /* Write string at current offset */
            numWritten = write(fd, &argv[ap][1], strlen(&argv[ap][1]));
            if (numWritten == -1)
                errExit("write");
            printf("%s: wrote %ld bytes\n", argv[ap], (long) numWritten);
            break;

        case 's':   /* Change file offset */
            offset = getLong(&argv[ap][1], GN_ANY_BASE, argv[ap]);
            if (lseek(fd, offset, SEEK_SET) == -1)
                errExit("lseek");
            printf("%s: seek succeeded\n", argv[ap]);
            break;

        default:
            cmdLineErr("Argument must start with [rRws]: %s\n", argv[ap]);
        }
    }

    if (close(fd) == -1)
        errExit("close");

    exit(EXIT_SUCCESS);
}
