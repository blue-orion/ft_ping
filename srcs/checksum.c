#include <inttypes.h>

uint16_t checksum(void *packet, int len) {
	uint16_t *buf;
	uint32_t sum = 0;

	buf = packet;
	for (sum = 0; len > 1; len -= 2) {
		sum += *buf++;
	}
	if (len == 1) {
		sum += *(uint8_t *)buf << 8;
	}

	while (sum >> 16) {
	  sum = (sum >> 16) + (sum & 0xFFFF);
	}
	return (~sum);
}
