#include "../includes/ping.h"
#include <inttypes.h>

uint16_t checksum(void *packet, int len) {
	uint16_t *buf;
	uint32_t sum = 0;

	buf = packet;
	for (sum = 0; len > 1; len -= 2) {
		sum += *buf++;
	}
	if (len == 1) {
		sum += *(uint8_t *)buf << 8;
	}

	while (sum >> 16) {
	  sum = (sum >> 16) + (sum & 0xFFFF);
	}
	return (~sum);
}

void	parse_option(ping_rts_t *rts, int ac, char **av, char *opt) {
	int	c;
	
	while (1) {
		c = getopt(ac, av, opt);
		if (c == -1)
			break ;

		switch (c) {
		case 'v':
			rts->opt_verbose = 1;
			break ;
		default:
			print_help();
			exit(2);
		}
	}
}

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
	diff += (end.tv_nsec - start.tv_nsec) / 1000000.0;
	return diff;
}

void	cleanup_rts(ping_rts_t	*rts) {
	close(rts->sockfd);
	close(rts->sigfd);
	free(rts->t_send);
}
