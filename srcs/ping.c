#include "../includes/ping.h"

void	statistic_rtt(ping_rts_t *rts, char *payload, struct timespec *recv_time);

void	send_packet(ping_rts_t *rts, double next) {
	char			packet[PACKET_SIZE];
	struct icmphdr	icmp_hdr;
	int				icmp_len;
	struct timespec	tp;

	// for polling
	if (next > 0) {
		return ;
	}
	memset(&icmp_hdr, 0, sizeof(icmp_hdr));
	icmp_hdr.type = rts->msg_type; // ICMP_ECHO
	icmp_hdr.code = 0;
	icmp_hdr.un.echo.id = htons(rts->id);
	icmp_hdr.un.echo.sequence = htons(rts->seq);

	memset(packet, 0, sizeof(packet));
	memcpy(packet, &icmp_hdr, sizeof(icmp_hdr));

	icmp_len = sizeof(icmp_hdr) + sizeof(tp);
	memset(packet + icmp_len, 'a', sizeof(packet) - icmp_len);
	((struct icmphdr *)packet)->checksum = checksum(packet, sizeof(packet));

	if (sendto(rts->sockfd, packet, sizeof(packet), 0, \
			(struct sockaddr *)&rts->dst, rts->socklen) < 0) {
		cleanup_rts(rts);
		exit_error(2, errno, "sendto");
	}

	clock_gettime(CLOCK_MONOTONIC, &tp);
	if (rts->stat->st.tv_sec == 0) {
		rts->stat->st = tp;
	}
	memcpy(packet + sizeof(icmp_hdr), &tp, sizeof(tp));
	rts->t_send[seq_to_index(rts->seq, rts->t_sendsize)] = tp;
	rts->last_send = tp;

	rts->stat->ntransmitted++;
	rts->seq++;
}

int	parse_reply(ping_rts_t *rts, char *packet, int cc) {
	int				ret;
	reply_t			reply;
	struct timespec	tp;

	// 원본 보존
	reply.packet = packet;

	// ip4_hdr 복원
	memcpy(&reply.ip4_hdr, packet, sizeof(reply.ip4_hdr));
	reply.ip4_len = reply.ip4_hdr.ihl * 4;

	// icmp_hdr 및 payload 복원
	memcpy(&reply.icmp_hdr, packet + reply.ip4_len, sizeof(reply.icmp_hdr));
	reply.payload = packet + reply.ip4_len + sizeof(reply.icmp_hdr);
	reply.payload_len = cc - reply.ip4_len - sizeof(reply.icmp_hdr);

	struct timespec	send_time;
	memcpy(&send_time, reply.payload, sizeof(send_time));

	clock_gettime(CLOCK_MONOTONIC, &tp);
	reply.rtt = (tp.tv_sec - send_time.tv_sec) * 1000.0;
	reply.rtt += (tp.tv_nsec - send_time.tv_nsec) / 1000000.0;

	ret = validate(rts, &reply, cc);
	if (ret == 1) // Skip ICMP_ECHO
		return 0;
	if (ret > 0)
		print_error_result(&reply);
	if (ret != 0) {
		rts->stat->nerrored++;
		return -1;
	}

	print_reply_result(&reply, cc);
	statistic_rtt(rts, reply.payload, &tp);
	return 0;
}

void	statistic_rtt(ping_rts_t *rts, char *payload, struct timespec *recv_time) {
	statistic_t		*stat;
	double			rtt;
	struct timespec	send_time;

	stat = rts->stat;
	stat->nrecved++;
	memcpy(&send_time, payload, sizeof(send_time));

	rtt = get_time_diff(send_time, *recv_time);

	stat->rtt_sum += rtt;
	stat->rtt_sum2 += rtt * rtt;
	if (rtt > stat->max_rtt)
		stat->max_rtt = rtt;
	if (rtt < stat->min_rtt)
		stat->min_rtt = rtt;
}
