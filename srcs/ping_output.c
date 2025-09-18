#include "../includes/ping.h"
#include <netdb.h>
#include <math.h>

void	print_reply_result(reply_t *reply, int cc, struct timespec *recv_time) {
	double	rtt;
	int		ttl = reply->ip4_hdr.ttl;

	cc = cc - reply->ip4_len;
	struct timespec	send_time;
	memcpy(&send_time, reply->payload, sizeof(send_time));

	rtt = (recv_time->tv_sec - send_time.tv_sec) * 1000.0;
	rtt += (recv_time->tv_nsec - send_time.tv_nsec) / 1000000.0;

	char source[64];
	char	host[64];
	inet_ntop(AF_INET,  &reply->ip4_hdr.saddr, source, INET_ADDRSTRLEN);
	struct sockaddr_in	sa;
	sa.sin_addr.s_addr = reply->ip4_hdr.saddr;
	sa.sin_family = AF_INET;
	getnameinfo((struct sockaddr *)&sa, 	\
			sizeof(sa), host, sizeof(host), NULL, 0, 0);
	int	seq = ntohs(reply->icmp_hdr.un.echo.sequence);

	printf("%d btyes from %s (%s): icmp_seq=%d ttl=%d time=%.3f ms\n", cc, host, source, seq, ttl, rtt);
}

void	print_statistics(ping_rts_t *rts, statistic_t *stat) {
	printf("\n--- %s ping statistics ---\n", rts->src_host);
	printf("%d packets transmitted, ", stat->ntransmitted);
	printf("%d received, ", stat->nrecved);
	if (stat->nerrored) {
		printf("+%d errors, ", stat->nerrored);
	}

	double runtime = (rts->last_send.tv_sec - stat->st.tv_sec) * 1000.0;
	runtime += (rts->last_send.tv_nsec - stat->st.tv_nsec) / 1000000.0;
	double	loss = ((stat->ntransmitted - stat->nrecved) / (double)stat->ntransmitted) * 100.0;
	stat->avg_rtt = stat->rtt_sum / stat->nrecved;
	stat->mdev_rtt = sqrt((stat->rtt_sum2) / stat->nrecved - (stat->avg_rtt * stat->avg_rtt));
	printf("%d%% packet loss, ", (int)loss);
	printf("time %.fms\n", runtime);

	if (stat->nrecved) {
		printf("rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms", 
				stat->min_rtt, stat->avg_rtt, stat->max_rtt, stat->mdev_rtt);
	}
	printf("\n");
}
