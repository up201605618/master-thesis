#include "p_v_calc.h"

#define	OUTPUT_PPR	(p_v_task->encoder_ppr * p_v_task->gearbox_ratio)
#define	delta(t1, t2)	(t2.tv_sec - t1.tv_sec) + ((t2.tv_nsec - t1.tv_nsec) / 1000000000.0)

int p_v_task_keep_running = 1;

void set_encoder_ppr (struct p_v_task_s *p_v_task, long ppr)
{
	p_v_task->encoder_ppr = ppr;
}

void set_gearbox_ratio (struct p_v_task_s *p_v_task, double ratio)
{
	p_v_task->gearbox_ratio = ratio;
}

int calc_velocity (struct p_v_task_s *p_v_task, long enc_count, double delta_t)
{
	static double prev_revs = 0.0;
	static double revs = 0.0;
	if (0.0 == OUTPUT_PPR)
		return -1;

	revs = (double) enc_count / OUTPUT_PPR;
	p_v_task->output_w = (revs - prev_revs) / delta_t * 60.0;

	prev_revs = revs;

	return 0;
}

int calc_position (struct p_v_task_s *p_v_task, long enc_count)
{
	if (0.0 == OUTPUT_PPR)
		return -1;

	p_v_task->output_p = (double) enc_count * 360.0 / OUTPUT_PPR;

	return 0;
}

void * p_v_task (void *args)
{
	struct p_v_task_s *p_v_task = (struct p_v_task_s *) args;
	struct sched_param sched_params = {.sched_priority = p_v_task->priority};
	if (0 != sched_setscheduler(0, SCHED_FIFO, &sched_params))
		pthread_exit((void *) -1);
	pthread_mutex_init(&p_v_task->velocity_mutex, NULL);
	pthread_mutex_init(&p_v_task->position_mutex, NULL);

	long count;
	double delta_t;
	struct timespec prev_time, cur_time;
	clock_gettime(CLOCK_MONOTONIC, &prev_time);

	while(p_v_task_keep_running) {
		usleep(p_v_task->period);
		clock_gettime(CLOCK_MONOTONIC, &cur_time);

		// Calculate velocity and position
		count = encoder_task_get_count(p_v_task->enc_task);
		delta_t = delta(prev_time, cur_time);
		calc_position(p_v_task, count);
		calc_velocity(p_v_task, count, delta_t);

		prev_time = cur_time;
	}

	pthread_mutex_destroy(&p_v_task->velocity_mutex);
	pthread_mutex_destroy(&p_v_task->position_mutex);
	pthread_exit((void *) 0);
}

double get_velocity (struct p_v_task_s *p_v_task)
{
	return -1;
}

double get_position (struct p_v_task_s *p_v_task)
{
	return -1;
}
