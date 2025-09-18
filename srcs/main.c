#include "../includes/ping.h"
#include <poll.h>

int	main(int ac, char **av) {
	ping_rts_t	rts;
	statistic_t	stat;

	if (ac < 2) {
		printf("Usage\n  ping [options] <destination>\n\n");
		return 0;
	}

	init_rts(&rts, &stat, av[1]);

	// TODO: Check options
	
	int					polling;
	char				result[2048];
	struct timespec		now;
	double				next = 0;
	double				runtime = 0;
	struct sockaddr_in	src;
	socklen_t			src_len;


	printf("PING %s (%s) %d (%d) bytes data\n", rts.src_host, rts.src_ip, DATALEN, ICMP_HEADER_SIZE + PACKET_SIZE);

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
		
		pfd[1].fd = rts.sigfd;
		pfd[1].events = POLLIN;

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
					(struct sockaddr *)&src, (socklen_t *)&src_len);

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
		runtime = get_time_diff(rts.last_send, now);
		next = rts.interval - runtime;
	}
	return 0;
}
