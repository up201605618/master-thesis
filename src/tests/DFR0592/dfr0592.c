#include "dfr0592.h"
#include "dfr0592_addrs.h"

const struct dfr_board * board_init(int i2c_bus, int addr)
{
	char i2c_filename[17];
	snprintf(i2c_filename, 16, "/dev/i2c-%d", i2c_bus);
	int fd = open(i2c_filename, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "Failed to open I2C bus\n");
		perror(i2c_filename);
		return NULL;
	}
	if (ioctl(fd, I2C_SLAVE, addr) < 0) {
		perror("ioctl (set slave address)");
		return NULL;
	}
	int pid = i2c_smbus_read_byte_data(fd, _REG_PID);
	int vid = i2c_smbus_read_byte_data(fd, _REG_PVD);
	if (pid != _REG_DEF_PID || vid != _REG_DEF_VID) {
		fprintf(stderr, "PID/VID does't match\n");
		fprintf(stderr, "PID is 0x%x, should be 0x%x\n", pid, _REG_DEF_PID);
		fprintf(stderr, "VID is 0x%x, should be 0x%x\n", vid, _REG_DEF_VID);
		errno = ENXIO;
		return NULL;
	}
	struct dfr_board *new_board = malloc(sizeof(struct dfr_board));
	struct dfr_board tmp = {
		.i2c_fd = fd,
		.addr = addr,
		.pid = pid,
		.vid = vid};
	*new_board = tmp;
	return new_board;
}

int board_close(const struct dfr_board *board)
{
	int tmp = close(board->i2c_fd);
	if (0 > tmp) {
		fprintf(stderr, "Failed to close file descriptor %d, ", board->i2c_fd);
		perror(NULL);
	}
	return tmp;
}

int board_set_mode(const struct dfr_board *board, enum modes mode)
{
	if (DC == mode) {
		i2c_smbus_write_byte_data(board->i2c_fd, _REG_CTRL_MODE, _CONTROL_MODE_DC_MOTOR);
	} else if (STEPPER == mode) {
		i2c_smbus_write_byte_data(board->i2c_fd, _REG_CTRL_MODE, _CONTROL_MODE_STEPPER);
	} else {
		errno = EINVAL;
		return -1;
	}
	if (i2c_smbus_read_byte_data(board->i2c_fd, _REG_CTRL_MODE) != mode) {
		errno = EIO;
		return -1;
	}
	return 0;
}

int set_pwm_frequency(const struct dfr_board *board, int freq)
{
	if (freq < 100 || freq > 12750) {
		fprintf(stderr, "Requested frequency out of range [100,12750]Hz\n");
		errno = EINVAL;
		return -1;
	}
	int tmp_freq = (int) freq / 50;
	if (0 != freq % 50) {
		fprintf(stderr, "Warning: actual frequency set to the closest multiple of 50Hz");
		fprintf(stderr, "Frequency set to %dHz", tmp_freq * 50);
	}
	int lret = i2c_smbus_write_byte_data(board->i2c_fd, _REG_MOTOR_PWM, tmp_freq);
	int rd_freq = i2c_smbus_read_byte_data(board->i2c_fd, _REG_MOTOR_PWM);
	if (1 != lret || rd_freq < 0 || rd_freq != tmp_freq) {
		errno = EIO;
		fprintf(stderr, "Failed to set frequency");
		return -1;
	}
	return 0;
}
