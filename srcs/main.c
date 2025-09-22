#include "../includes/ping.h"
#include <poll.h>

int	main(int ac, char **av) {
	int			ret;
	ping_rts_t	rts;
	statistic_t	stat;

	if (ac < 2) {
		exit_error(2, EDESTADDRREQ, "Usage error");
		return 1;
	}

	ret = init_rts(&rts, &stat, av[1]); 
	if (ret < 0) {
		if (IS_GAI_ERR(ret))
			exit_error(2, 0, "%s: %s", av[1], gai_strerror(GET_GAI_CODE(ret)));
		exit_error(2, errno, "System error");
	}

	int					polling;
	char				result[2048];
	struct timespec		now;
	double				next = 0.0;
	double				runtime = 0;
	struct pollfd 		pfd[2];

	pfd[0].fd = rts.sockfd;
	pfd[0].events = POLLIN;
	
	pfd[1].fd = rts.sigfd;
	pfd[1].events = POLLIN;

	printf("PING %s (%s) %d (%d) bytes data\n", rts.src_host, rts.src_ip, DATALEN, ICMP_HEADER_SIZE + PACKET_SIZE);

	for (;;) {
		polling = 0;

		do {
			send_packet(&rts, next);

			clock_gettime(CLOCK_MONOTONIC, &now);
			runtime = get_time_diff(rts.last_send, now);
			next = rts.interval - runtime;
		} while (next < 0);

		int	e = poll(pfd, 2, next);
		if (e < 0) {
			exit_error(2, errno, "poll");
		}
		if (pfd[1].revents & POLLIN) {
			print_statistics(&rts, &stat);
			break ;
		}

		polling = MSG_DONTWAIT;

		for (;;) {
			memset(result, 0, sizeof(result));
			int cc = recvfrom(rts.sockfd, result, sizeof(result), polling, NULL, NULL);

			if (cc < 0) {
				if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
					break ;
				}
				else {
					exit_error(2, errno, "recvfrom");
				}
			}

			if (parse_reply(&rts, result, cc) < 0) {
				perror("parse reply");
				// TODO: 에러난 패킷 처리
			}
		}
	}
	cleanup_rts(&rts);
	return rts.status;
}
