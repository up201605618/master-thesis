#include "dfr0592.h"
#include <stdio.h>
#include <errno.h>

int main(int argc, char **argv) {
    if(argc != 1) {
        printf("%s takes no arguments.\n", argv[0]);
        return 1;
    }
    const struct dfr_board *b = board_init(1, 16);
    if (NULL == b) {
	fprintf(stderr, "board_init(%d, %d) failed\n", 1, 16);
	return errno;
    }
    printf("fd = %d and addr = %d\n", b->i2c_fd, b->addr);
    free((void *) b);
    if (0 > board_close(b)) {
	perror("board_close()");
	return errno;
    }
    return 0;
}
