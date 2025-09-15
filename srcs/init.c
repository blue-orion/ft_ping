#include "../includes/ping.h"
#include <sys/signalfd.h>
#include <netdb.h>

void	set_destination(ping_rts_t *rts, char *addr) {
	struct addrinfo	info;
	struct addrinfo	hints;
	struct sockaddr_in	*dst = &rts->dst;

	// getaddrinfo(addr, )
	rts->source = addr;
	dst->sin_family = AF_INET;
	if (inet_pton(AF_INET, addr, &dst->sin_addr) == -1) {
		perror("inet_pton");
		exit(1);
	}
	rts->socklen = sizeof(*dst);
}

struct timespec *init_tsend(ping_rts_t *rts) {
	// 추후 timeout 변경 시 모두 로직 수정해야함.
	rts->interval = INTERVAL;
	rts->timeout = rts->interval * 2;
	
	int	w = rts->timeout / rts->interval + 2;
	int	n = 1;
	while (n < w) {
		n = n << 1;
	}
	rts->t_send = malloc(n * sizeof(struct timespec));
	rts->t_sendsize = n;
	return rts->t_send;
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
