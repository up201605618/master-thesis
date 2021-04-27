#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

#include <gpiod.h>

#include "encoder.h"

volatile int keep_running = 1;

void sighandler (int signum)
{
	if (SIGINT == signum) {
		keep_running = 0;
	}
}

int main (int argc, char *argv[])
{
	int ret = 0;
	signal(SIGINT, sighandler);
	struct encoder enc = {0};
	if (-1 == encoder_init(&enc, 0, 17, 18)) {
		perror("Failed to initialize 'encoder'");
		ret = -1;
		goto end;
	}
	puts("Encoder open OK");
	if (-1 == encoder_start(&enc)) {
		ret = -1;
		perror("Failed to request events for the inputs");
		goto end;
	}
	while (keep_running) {
		if (1 == encoder_wait(&enc)) {
			printf("\e[1;1H\e[2J"); // clear console
			printf("Line A: %d\n", gpiod_line_get_value(enc.a_line));
			printf("Line B: %d\n", gpiod_line_get_value(enc.b_line));
		}
	}

end:	
	printf("\33[2K\n");
	encoder_end(&enc);
	return ret;
}
