#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#define PROGNAME	"size-enc"	/* program name for debug output */
#define MAX_SIZE	0x00ffffff	/* biggest value 3 bytes can store */

main(int ac, char** av)
{
    struct stat sbuf;	/* file info */

    if (ac != 2) {
	fprintf(stderr, "usage: %s filename\n", PROGNAME);
	exit(1);
    }

    if (stat(av[1], &sbuf) == -1) {
	perror(av[1]);
	exit(1);
    }

    if (sbuf.st_size > MAX_SIZE) {
	fprintf(stderr, "%s: file size too large to be encoded in three bytes\n", av[1]);
    }

    /* put out the file size in three bytes, most signif. byte first */
    putchar((sbuf.st_size >> 16) & 0xff);
    putchar((sbuf.st_size >>  8) & 0xff);
    putchar((sbuf.st_size >>  0) & 0xff);

    exit(0);
}
