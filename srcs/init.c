#include "../includes/ping.h"
#include <sys/signalfd.h>
#include <limits.h>

static int	set_destination(ping_rts_t *rts, char *addr);
static int	init_tsend(ping_rts_t *rts);
static int	set_signal();

int	init_rts(ping_rts_t *rts, statistic_t *stat, char *dst) {
	int	saved = 0;
	int	rc = 0;

	memset(rts, 0, sizeof(ping_rts_t));
	rts->sockfd = -1;
	rts->sigfd = -1;

	memset(stat, 0, sizeof(statistic_t));
	stat->min_rtt = INT_MAX;
	rts->stat = stat;
	rts->interval = DFL_INTERVAL;
	rts->msg_type = ICMP_ECHO;
	rts->id = getpid() & 0xFFFF;
	rts->seq = 1;

	rc = set_destination(rts, dst);
	if (rc != 0)
		goto fail;

	rts->sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (rts->sockfd < 0)
		goto fail;

	rts->sigfd = set_signal();
	if (rts->sigfd < 0)
		goto fail;

	if (init_tsend(rts) < 0)
		goto fail;

	return 0;

fail:
	if (IS_GAI_ERR(rc)) return rc;
	saved = errno;
	if (rts->sockfd >= 0) {
		close(rts->sockfd);
		rts->sockfd = -1;
	}
	if (rts->sigfd >= 0) {
		close(rts->sigfd);
		rts->sigfd = -1;
	}
	if (rts->t_send) {
		free(rts->t_send);
		rts->t_send = NULL;
	}
	errno = saved;
	return -1;
}

static int	set_destination(ping_rts_t *rts, char *addr) {
	struct addrinfo	*info;
	struct addrinfo	hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = 0;
	hints.ai_flags = AI_CANONNAME;

	int	rc = getaddrinfo(addr, NULL, &hints, &info);
	if (rc != 0) {
		if (rc == EAI_SYSTEM)
			return -1;
		return MAKE_GAI_ERR(rc);
	}

	if (info->ai_addrlen > sizeof(rts->dst)) {
        freeaddrinfo(info);
        return ping_fail_errno(EOVERFLOW);
    }

	memcpy(&rts->dst, info->ai_addr, info->ai_addrlen);
	rts->socklen = info->ai_addrlen;

	if (info->ai_canonname) {
		strncpy(rts->src_host, info->ai_canonname, sizeof(rts->src_host) - 1);
		rts->src_host[sizeof(rts->src_host) - 1] = '\0';
	}

	struct sockaddr_in	*sa = (struct sockaddr_in *)info->ai_addr;
	if (!inet_ntop(AF_INET, &sa->sin_addr, rts->src_ip, INET_ADDRSTRLEN)) {
		int	e = errno;
		freeaddrinfo(info);
		return ping_fail_errno(e);
	}

	freeaddrinfo(info);
	return 0;
}

static int	init_tsend(ping_rts_t *rts) {
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

static int	set_signal() {
	int					sigfd;
	sigset_t			set;

	sigemptyset(&set);
	sigaddset(&set, SIGINT);

	if (sigprocmask(SIG_BLOCK, &set, NULL) < 0)
		return -1;

	sigfd = signalfd(-1, &set, 0);
	return sigfd;
}
