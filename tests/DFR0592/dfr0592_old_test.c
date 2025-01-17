#include "dfr0592.h"
#include <stdio.h>
#include <errno.h>
#include <time.h>

int main(int argc, char **argv) {
	if(argc != 1) {
		printf("%s takes no arguments.\n", argv[0]);
		return 1;
	}
	const struct dfr_board *b = board_init(1, 0x10);
	if (NULL == b) {
		fprintf(stderr, "board_init(%d, %d) failed\n", 1, 16);
		return errno;
	}
	if (0 > board_set_mode(b, DC)) {
		perror("board_set_mode()");
		goto ret_err;
	}
	if (0 > set_pwm_frequency(b, 1000)) {
		perror("set_pwm_frequency()");
		goto ret_err;
	}
	if (0 > encoder_enable(b, 1)) {
		perror("encoder_enable(1)");
		goto ret_err;
	}
	if (0 > encoder_disable(b, 2)) {
		perror("encoder_disable(2)");
		goto ret_err;
	}
	// Error expected
	if (0 == encoder_disable(b, 3)) {
		fprintf(stderr, "encoder_disable(3) should have failed, but didn't!");
		goto ret_err;
	}
	fputs("^^ This error is expected ^^\n", stderr);

	if (0 > encoder_set_ratio(b,1,30)) {
		perror("encoder_set_ratio(1,1)");
		goto ret_err;
	}

	if (0 > motor_set_speed(b, 1, 50)) {
		perror("motor_set_speed(1, 50)");
		goto ret_err;
	}
	int speed = 0;
	for (int i=0; i<10; i++) {
		sleep(1);
		encoder_get_speed(b, 1, &speed);
		printf("Speed: %d RPM\n", speed);
	}
	if (0 > motor_stop(b, 1)) {
		perror("motor_stop(1)");
		goto ret_err;
	}
	sleep(1);
	if (0 > motor_set_speed(b, 1, -50)) {
		perror("motor_set_speed(1, -50)");
		goto ret_err;
	}
	for (int i=0; i<10; i++) {
		sleep(1);
		encoder_get_speed(b, 1, &speed);
		printf("Speed: %d RPM\n", speed);
	}
	if (0 > motor_stop(b, 1)) {
		perror("motor_stop(1)");
		goto ret_err;
	}
	if (0 > board_close(b)) {
		perror("board_close()");
		goto ret_err;
	}
	free((void *) b);
	puts("Everything OK");
	return 0;
ret_err:
	free((void *) b);
	return -1;
}
