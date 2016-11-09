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
Comments added by: Shyamal S. Chandra, 2016

File descriptors -- "...refer to all types of open files, including pipes, FIFOs,
sockets, terminals, devices, and regular files."
Source: Kerrisk, Michael. The Linux programming interface. No Starch Press, 2010.
Site: http://man7.org/tlpi/code/
*/

#include <sys/stat.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

#ifndef BUF_SIZE
#define BUF_SIZE 1024
#endif

int main(int argc, char *argv[]) {
  int inputFd, outputFd, openFlags;

  // mode_t: integer: "...File permissions and type..."
  mode_t filePerms;
  // ssize_t: signed integer: "...Byte count or (negative) error indication..."
  ssize_t numRead;
  char buf[BUF_SIZE];

  if (argc != 3 || strcmp(argv[1], "--help") == 0)
    usageError("%s old-file new-file\n", argv[0]);

  /* Open input and output files */

  inputFd = open(argv[1], O_RDONLY);
  if (inputFd == -1) {
    errExit("opening file %s", argv[1]);
  }
  /*
  O_CREAT:
  "...If the file does not exist, it will be created..."
  Source: http://man7.org/linux/man-pages/man2/open.2.html

  O_WRONLY:
  "...Open for writing only..."
  Source: https://linux.die.net/man/3/open

  O_TRUNC:
  "...If the file exists and is a regular file...its length shall be truncated
  to 0, and the mode and owner shall be unchange..."
  Source: http://pubs.opengroup.org/onlinepubs/000095399/functions/open.html
  */
  openFlags = O_CREAT | O_WRONLY | O_TRUNC;

  /*
  S_IRUSR:
  "...Read permission bit for the owner of the file..."
  Source: https://www.gnu.org/software/libc/manual/html_node/Permission-Bits.html

  S_IWUSR:
  "...Write permission bit for the owner of the file..."
  Source: https://www.gnu.org/software/libc/manual/html_node/Permission-Bits.html

  S_IRGRP:
  "...Read permission bit for the group owner of the file. Usually 040..."
  Source: https://www.gnu.org/software/libc/manual/html_node/Permission-Bits.html

  S_IWGRP:
  "...Write permission bit for the group owner of the file. Usually 020...."
  Source: https://www.gnu.org/software/libc/manual/html_node/Permission-Bits.html

  S_IROTH:
  "...Read permission bit for other users. Usually 04..."
  Source: https://www.gnu.org/software/libc/manual/html_node/Permission-Bits.html

  S_ISWOTH:
  Source: https://www.gnu.org/software/libc/manual/html_node/Permission-Bits.html
  */
  filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

  // open(const char *pathname, int flags, mode_t mode)
  // Source: http://man7.org/linux/man-pages/man2/open.2.html
  outputFd = open(argv[2], openFlags, filePerms);

  if (outputFd == -1) {
    /*
    void errExit(const char *format, ...)
    {
      // va_list (Vaiadic function):
      // "...a complete object type suitable for holding the information
      // needed by the macros va_start, va_copy, va_arg, and va_end..."
      // Source: http://en.cppreference.com/w/cpp/utility/variadic/va_list
      va_list argList;
      va_start(argList, format);
      outputError(TRUE, errno, TRUE, format, argList);
      va_end(argList);
      terminate(TRUE);
    }
    Source: http://man7.org/tlpi/code/online/book/lib/error_functions.c.html#errExit

    Q: What is variadic function?
    A: "...A function of indefinite arity,...one which accepts a variable
       number of arguments..."
    Source: https://en.wikipedia.org/wiki/Variadic_function
    */
    errExit("opening file %s", argv[2]);
  }

  /* Transfer data until we encounter end of input or an error */
  /*
    ssize_t read(int fd, void *buf, size_t count);
    Description:
      "...read() attempts to read up to count bytes from file descriptor fd
      into the buffer starting at buf..."
    Source: http://man7.org/linux/man-pages/man2/read.2.html
  */
  while ((numRead = read(inputFd, buf, BUF_SIZE)) > 0) {

    /*
      ssize_t write(int fd, const void *buf, size_t count)
      "...write() writes up to count bytes frmo the buffer pointed buf
      to the file referred to by the file descriptor fd..."
      Source: http://man7.org/linux/man-pages/man2/write.2.html
    */
    if (write(outputFd, buf, numRead) != numRead) {
      /*
        void fatal(const char *format, ...) {
          va_list argList;
          va_start(argList, format);
          outputError(FALSE, 0, TRUE, format, argList);
          va_end(argList);
          terminate(TRUE);
        }
        Source: http://man7.org/tlpi/code/online/book/lib/error_functions.c.html
      */
      fatal("couldn't write whole buffer");
    }
  }

  if (numRead == -1) {
    /*
      "...Print an error message and exit..."
      Source: http://www.ee.ryerson.ca/~courses/cn8800/programs/errexit.c
      void errExit(const char *format, ...) {
          va_list argList;

          va_start(argList, format);
          outputError(TRUE, errno, TRUE, format, argList);
          va_end(argList);

          terminate(TRUE);
      }
      Source: http://man7.org/tlpi/code/online/book/lib/error_functions.c.html
    */
    errExit("read");
  }

  if (close(inputFd) == -1) {
    errExit("close input");
  }
  if (close(outputFd) == -1) {
    errExit("close output");
  }

  exit(EXIT_SUCCESS);

}
