#include "../includes/ping.h"
#include <netinet/ip_icmp.h>

int	validate_ip4_hdr(struct iphdr *ih);
int	validate_icmp(ping_rts_t *rts, struct icmphdr *ih, char *payload, int payload_len);

int	validate(ping_rts_t *rts, reply_t *reply, int cc) {
	int			type;
	uint16_t	check;

	// 패킷 총 길이와 실제 수신 받은 크기 비교
	if (ntohs(reply->ip4_hdr.tot_len) != cc) {
		return -1;
	}

	if (validate_ip4_hdr(&reply->ip4_hdr)) {
		return -1;
	}

	// Skip ECHO REQUEST PACKET in localhost
	if (reply->icmp_hdr.type == ICMP_ECHO) {
		return 1;
	}

	if (reply->icmp_hdr.type != ICMP_ECHOREPLY || reply->icmp_hdr.code != 0) {
		return reply->icmp_hdr.type;
	}

	if (validate_icmp(rts, &reply->icmp_hdr, reply->payload, reply->payload_len)) {
		return -1;
	}

	check = checksum(reply->packet + reply->ip4_len, cc - reply->ip4_len);
	if (check != 0) {
		return -1;
	}

	return 0;
}

int	validate_ip4_hdr(struct iphdr *ih) {
	if (ih->version != 4 ||
		ih->protocol != 1 ||
		ih->ihl < 5)
		return 1;
	return 0;
}

int	validate_icmp(ping_rts_t *rts, struct icmphdr *ih, char *payload, int payload_len) {
	// Validate ICMP Header
	if (rts->id != ntohs(ih->un.echo.id)) {
		return -1;
	}
	if (ntohs(ih->un.echo.sequence) > rts->seq) {
		return -1;
	}

	// Validate ICMP Payload
	struct timespec	tp;
	struct timespec	send_time = get_send_time(rts, ntohs(ih->un.echo.sequence));

	memcpy(&tp, payload, sizeof(tp));
	if (memcmp(&tp, &send_time, sizeof(tp))) {
		return 1;
	}
	for (int i = sizeof(struct timespec); i < payload_len; i++) {
		if (payload[i] != 'a') {
			return 1;
		}
	}
	return 0;
}
