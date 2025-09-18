#include "../includes/ping.h"
#include <sys/signalfd.h>
#include <limits.h>

int	init_rts(ping_rts_t *rts, statistic_t *stat, char *dst) {
	memset(rts, 0, sizeof(ping_rts_t));
	memset(stat, 0, sizeof(statistic_t));
	stat->min_rtt = INT_MAX;
	rts->stat = stat;
	rts->interval = DFL_INTERVAL;
	rts->msg_type = ICMP_ECHO;
	rts->id = getpid() & 0xFFFF;
	rts->seq = 1;

	rts->sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (rts->sockfd < 0) {
		perror("socket");
		return -1;
	}

	rts->sigfd = set_signal();
	if (rts->sigfd < 0) {
		perror("set signal");
		return -1;
	}

	set_destination(rts, dst);

	if (init_tsend(rts) < 0) {
		perror("malloc");
		return -1;
	}
	return 0;
}

void	set_destination(ping_rts_t *rts, char *addr) {
	struct addrinfo	*info;
	struct addrinfo	hints;

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_flags = AI_CANONNAME;

	int	rc = getaddrinfo(addr, NULL, &hints, &info);
	if (rc < 0) {
		printf("getaddrinfo: %s\n", gai_strerror(rc));
		exit(1);
	}

	memcpy(&rts->dst, info->ai_addr, info->ai_addrlen);
	rts->src_host = strdup(info->ai_canonname);
	struct sockaddr_in	*sa_in = (struct sockaddr_in *)info->ai_addr;
	if (inet_ntop(AF_INET, &sa_in->sin_addr, rts->src_ip, INET_ADDRSTRLEN) < 0) {
		perror("inet_ntop");
		exit(1);
	}
	rts->socklen = info->ai_addrlen;
	freeaddrinfo(info);
}

int	init_tsend(ping_rts_t *rts) {
	// 추후 timeout 변경 시 모두 로직 수정해야함.
	rts->interval = DFL_INTERVAL;
	rts->timeout = rts->interval * 2;
	
	int	w = rts->timeout / rts->interval + 2;
	int	n = 1;
	while (n < w) {
		n = n << 1;
	}
	rts->t_send = malloc(n * sizeof(struct timespec));
	if (!rts->t_send)
		return -1;
	rts->t_sendsize = n;
	return 0;
}

int	set_signal() {
	int					sigfd;
	sigset_t			set;

	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	sigprocmask(SIG_BLOCK, &set, NULL);

	sigfd = signalfd(-1, &set, 0);
	return sigfd;
}
