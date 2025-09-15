#ifndef PING_H
# define PING_H

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <inttypes.h>

# include <netinet/in.h>
# include <arpa/inet.h>
# include <netinet/ip_icmp.h>
# include <sys/types.h>
# include <inttypes.h>
# include <time.h>
# include <string.h>
# include <signal.h>

# define ICMP_HEADER_SIZE 20
# define PACKET_SIZE 64
# define DEFDATALEN (64 - 8)
# define INTERVAL 1000

typedef struct ping_rts	ping_rts_t;
typedef struct reply	reply_t;
typedef struct statistic statistic_t;

void	print_statistics(ping_rts_t *rts, statistic_t *stat);
int	checksum(void *packet, int len);
int	set_signal();
void	set_destination(ping_rts_t *rts, char *addr);
struct timespec *init_tsend(ping_rts_t *rts);
void	send_packet(ping_rts_t *rts);
int		parse_reply(ping_rts_t *rts, char *packet, int packlen);
int	validate(ping_rts_t *rts, reply_t *reply, int cc);
void	sigint_handler(int signum);

/* reply packet structure */
struct reply {
	struct iphdr	ip4_hdr;
	int				ip4_len;
	struct icmphdr	icmp_hdr;
	char 			*payload;
	int				payload_len;
};

struct statistic {
	int		nrecved;
	int		ntransmitted;
	int		nerrored;
	double	max_rtt;
	double	min_rtt;
	double	rtt_sum;
	double	rtt_sum2;
	double	mdev_rtt;
	double	avg_rtt;
	struct timespec st;
};

/* ping runtime state */
struct ping_rts {
	int	sigfd;
	int	sockfd;
	int	ttl;
	int	interval;
	int	timeout;

	uint16_t	id;
	uint16_t	seq;
	uint8_t		msg_type;

	struct timespec	*t_send;
	int				t_sendsize;
	struct timespec	last_send;

	char					*source;
	struct sockaddr_in		dst;
	struct sockaddr_in6		dst6;
	int						socklen;

	statistic_t	*stat;

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


int	seq_to_index(int seq, int n);
struct timespec	get_send_time(ping_rts_t *rts, int seq);
double	get_ms_time(struct timespec tp);

#endif
