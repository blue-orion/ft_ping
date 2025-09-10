#ifndef PING_H
# define PING_H

# include <stdio.h>
# include <unistd.h>
# include <inttypes.h>

# include <netinet/in.h>
# include <arpa/inet.h>
# include <netinet/ip_icmp.h>
# include <sys/types.h>
# include <inttypes.h>
# include <string.h>

# define PACKET_SIZE 64
# define DEFDATALEN (64 - 8)

int	checksum(void *packet, int len);

/* ping runtime state */
struct ping_rts {
	int	sockfd;
	int	ttl;
	int	interval;
	int	seq;
	int	id;

	// union {
	// 	struct sockaddr_in	dst;
	// 	struct sockaddr_in6	dst;
	// };

	/* option flag bit field */
	unsigned int
		opt_address:1,
		opt_echo:1,
		opt_mask:1,
		opt_ts:1,
		opt_type:1,
		opt_count:1,
		opt_debug:1,
		opt_interval:1,
		opt_nemeric:1,
		opt_ignore_route:1,
		opt_tos:1,
		opt_ttl:1,
		opt_verbose:1,
		opt_timeout:1,
		opt_linger:1,
		opt_flood:1,
		opt_ip_ts:1,
		opt_quiet:1,
		opt_route:1,
		opt_size:1;
};

typedef struct statistic {
	int		nrecved;
	int		ntransmitted;
	double	runtime;
	double	max_rtt;
	double	min_rtt;
	double	rtt_sum;
	double	mdev_rtt;
	double	avg_rtt;
}	statistic;

#endif
