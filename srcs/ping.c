#include "../includes/ping.h"

void	print_reply_result(reply_t *reply, int cc, struct timespec *recv_time);
void	statistic_rtt(ping_rts_t *rts, char *payload, struct timespec *recv_time);

void	send_packet(ping_rts_t *rts) {
	char			packet[PACKET_SIZE];
	struct icmphdr	icmp_hdr;

	memset(&icmp_hdr, 0, sizeof(icmp_hdr));
	icmp_hdr.type = rts->msg_type; // ICMP_ECHO
	icmp_hdr.code = 0;
	icmp_hdr.un.echo.id = htons(rts->id);
	icmp_hdr.un.echo.sequence = htons(rts->seq);

	memset(packet, 0, sizeof(packet));
	memcpy(packet, &icmp_hdr, sizeof(icmp_hdr));

	struct timespec	tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	memcpy(packet + sizeof(icmp_hdr), &tp, sizeof(tp));
	rts->t_send[rts->seq & (rts->t_sendsize - 1)] = tp;
	rts->last_send = tp;

	int		icmplen = sizeof(icmp_hdr) + sizeof(tp);
	memset(packet + icmplen, 'a', sizeof(packet) - icmplen);
	((struct icmphdr *)packet)->checksum = checksum(packet, sizeof(packet));

	int	i = sendto(rts->sockfd, packet, sizeof(packet), 0, (struct sockaddr *)&rts->dst, rts->socklen);

	if (i < 0) {
		perror("sendto");
		exit(1);
	}
}

int	parse_reply(ping_rts_t *rts, char *packet, int cc) {
	reply_t	reply;

	memcpy(&reply.ip4_hdr, packet, sizeof(reply.ip4_hdr));
	reply.ip4_len = reply.ip4_hdr.ihl * 4;
	memcpy(&reply.icmp_hdr, packet + reply.ip4_len, sizeof(reply.icmp_hdr));
	reply.payload = packet + reply.ip4_len + sizeof(reply.icmp_hdr);
	reply.payload_len = cc - reply.ip4_len - sizeof(reply.icmp_hdr);

	if (validate(rts, &reply, cc) != 0) {
		// printf("Error packet\n");
		return 1;
	}

	uint16_t check = checksum(packet + reply.ip4_len, cc - reply.ip4_len);
	if (check != 0) {
		return -1;
	}

	struct timespec	tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);

	print_reply_result(&reply, cc, &tp);
	statistic_rtt(rts, reply.payload, &tp);

	return 0;
}

void	statistic_rtt(ping_rts_t *rts, char *payload, struct timespec *recv_time) {
	statistic_t	*stat = rts->stat;
	double	rtt;
	struct timespec send_time;

	stat->nrecved++;
	memcpy(&send_time, payload, sizeof(send_time));

	rtt = (recv_time->tv_sec - send_time.tv_sec) * 1000.0;
	rtt += (recv_time->tv_nsec - send_time.tv_nsec) / 1000000.0;

	stat->rtt_sum += rtt;
	stat->rtt_sum2 += rtt * rtt;
	if (rtt > stat->max_rtt)
		stat->max_rtt = rtt;
	if (rtt < stat->min_rtt)
		stat->min_rtt = rtt;
}
