#include "../includes/ping.h"
#include <asm-generic/errno.h>
#include <poll.h>
#include <sys/socket.h>

void	main_loop(ping_rts_t *rts);

int	main(int ac, char **av) {
	int			ret;
	ping_rts_t	rts;
	statistic_t	stat;

	memset(&rts, 0, sizeof(rts));
	memset(&stat, 0, sizeof(stat));

	parse_option(&rts, ac, av, "v?");
	ac -= optind;
	av += optind;
	if (!ac)
		exit_error(1, EDESTADDRREQ, "usage error");

	ret = init_rts(&rts, &stat, av[ac - 1]); 
	if (ret < 0) {
		if (IS_GAI_ERR(ret))
			exit_error(2, 0, "%s: %s", av[ac - 1], gai_strerror(GET_GAI_CODE(ret)));
		exit_error(2, errno, "System error");
	}

	main_loop(&rts);
	cleanup_rts(&rts);
	return rts.status;
}

void	main_loop(ping_rts_t *rts) {
	char				result[2048];
	struct timespec		now;
	double				next = 0.0;
	struct pollfd 		pfd[2];
	int					cc;

	pfd[0].fd = rts->sockfd;
	pfd[0].events = POLLIN;
	
	pfd[1].fd = rts->sigfd;
	pfd[1].events = POLLIN;

	printf("PING %s (%s) %d (%d) bytes data\n", \
		rts->src_host, rts->src_ip, DATALEN, ICMP_HEADER_SIZE + PACKET_SIZE);
	for (;;) {
		do {
			send_packet(rts, next);

			clock_gettime(CLOCK_MONOTONIC, &now);
			next = rts->interval - get_time_diff(rts->last_send, now);
		} while (next < 0);

		if (poll(pfd, 2, next) < 0) {
			exit_error(2, errno, "poll");
		}
		if (pfd[1].revents & POLLIN) {
			print_statistics(rts);
			break ;
		}

		for (;;) {
			memset(result, 0, sizeof(result));
			cc = recvfrom(rts->sockfd, result, sizeof(result), MSG_DONTWAIT, NULL, NULL);
			if (cc < 0) {
				if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
					break ;
				else
					exit_error(2, errno, "recvfrom");
			}
			parse_reply(rts, result, cc);
		}
	}
}
