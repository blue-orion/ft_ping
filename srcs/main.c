#include "../includes/ping.h"
#include <errno.h>
#include <limits.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <poll.h>

#define TTL 64

int	main(int ac, char **av) {
	if (ac < 2) {
		printf("Usage\n  ping [options] <destination>\n\n");
		return 0;
	}

	ping_rts_t	rts;
	statistic_t	stat;
	memset(&stat, 0, sizeof(statistic_t));
	stat.min_rtt = INT_MAX;
	rts.stat = &stat;

	// TODO: Check options
	
	int	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockfd == -1) {
		perror("socket");
		return 1;
	}
	rts.sockfd = sockfd;

	// TODO: init_socket();

	rts.sigfd = set_signal();
	if (rts.sigfd < 0) {
		perror("set signal");
		exit(1);
	}
	set_destination(&rts, av[1]);

	rts.interval = INTERVAL;
	rts.t_send = init_tsend(&rts);
	if (!rts.t_send) {
		perror("malloc");
		exit(1);
	}

	struct icmphdr	icmp_hdr;
	char		result[2048];

	rts.msg_type = ICMP_ECHO;
	rts.id = getpid() & 0xFFFF;
	rts.seq = 1;

	int	polling;
	printf("PING %s: %d(%d) bytes data\n", av[1], DEFDATALEN, ICMP_HEADER_SIZE + PACKET_SIZE);
	struct timespec	st;
	clock_gettime(CLOCK_MONOTONIC, &st);

	struct timespec	now;
	double			next = 0;
	double			rtt = 0;
	struct icmphdr	reply;

	clock_gettime(CLOCK_MONOTONIC, &rts.stat->st);
	for (;;) {
		polling = 0;

		if (next <= 0) {
			send_packet(&rts);
			stat.ntransmitted++;
			rts.seq++;
		}

		struct pollfd pfd[2];
		pfd[0].fd = rts.sockfd;
		pfd[0].events = POLLIN;
		pfd[0].revents = 0;
		
		pfd[1].fd = rts.sigfd;
		pfd[1].events = POLLIN;
		pfd[1].revents = 0;

		int	e = poll(pfd, 2, next);
		if (e < 0) {
			perror("poll");
			exit(1);
		}
		if (pfd[1].revents & POLLIN) {
			print_statistics(&rts, &stat);
			return 0;
		}

		polling = MSG_DONTWAIT;

		for (;;) {
			int cc = recvfrom(rts.sockfd, result, sizeof(result), polling, 
					(struct sockaddr *)&rts.dst, (socklen_t *)&rts.socklen);

			if (cc < 0) {
				if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
					break ;
				}
			}

			if (parse_reply(&rts, result, cc) < 0) {
				perror("parse reply");
				exit(1);
			}
		}

		clock_gettime(CLOCK_MONOTONIC, &now);
		rtt = (now.tv_sec - rts.last_send.tv_sec) * 1000.0;
		rtt += (now.tv_nsec - rts.last_send.tv_nsec) / 1000000.0;
		next = rts.interval - rtt;
	}
	return 0;
}
