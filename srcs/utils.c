#include "../includes/ping.h"

int	seq_to_index(int seq, int n) {
	return (seq & (n - 1));
}

struct timespec	get_send_time(ping_rts_t *rts, int seq) {
	return rts->t_send[seq & (rts->t_sendsize - 1)];
}

double	get_ms_time(struct timespec tp) {
	return tp.tv_sec * 1000.0 + tp.tv_nsec / 1000000.0;
}

double	get_time_diff(struct timespec start, struct timespec end) {
	double	diff;

	diff = (end.tv_sec - start.tv_sec) * 1000.0;
	diff += (end.tv_nsec - start.tv_sec) / 1000000.0;
	return diff;
}
