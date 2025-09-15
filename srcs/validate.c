#include "../includes/ping.h"

int	validate_ip4_hdr(struct iphdr *ih);
int	validate_icmp(ping_rts_t *rts, struct icmphdr *ih, char *payload, int payload_len);

int	validate(ping_rts_t *rts, reply_t *reply, int cc) {
	// 패킷 총 길이와 실제 수신 받은 크기 비교

	if (validate_ip4_hdr(&reply->ip4_hdr)) {
		printf("[DEBUG] invalid ip4 header\n");
		return 1;
	}
	if (validate_icmp(rts, &reply->icmp_hdr, reply->payload, reply->payload_len)) {
		return 1;
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
	// Skip ECHO REQUEST PACKET in localhost
	if (ih->type == ICMP_ECHO) {
		return 1;
	}
	if (ih->type != ICMP_ECHOREPLY || ih->code != 0) {
		// printf("[DEBUG] Invalid icmp type %d\n", ih->type);
		return ih->type;
	}
	if (rts->id != ntohs(ih->un.echo.id)) {
		// printf("[DEBUG] Invalid reply id %d, %d\n", rts->id, ntohs(ih->un.echo.id));
		return 1;
	}
	if (ntohs(ih->un.echo.sequence) > rts->seq) {
		// printf("[DEBUG] Invalid reply seq %d, %d\n", rts->seq, ih->un.echo.sequence);
		return 1;
	}

	// Validate ICMP Payload
	struct timespec	tp;
	struct timespec	send_time = get_send_time(rts, ntohs(ih->un.echo.sequence));

	memcpy(&tp, payload, sizeof(tp));
	if (memcmp(&tp, &send_time, sizeof(tp))) {
		// printf("[DEBUG] sequence num = %d\n", ntohs(ih->un.echo.sequence));
		// printf("[DEBUG] Invalid payload\nsend time %.2f, payload %.2f\n", get_ms_time(tp), get_ms_time(send_time));
		return 1;
	}
	for (int i = sizeof(struct timespec); i < payload_len; i++) {
		if (payload[i] != 'a') {
			return 1;
		}
	}
	return 0;
}
