#include "../includes/ping.h"

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <inttypes.h>
#include <string.h>

#define PACKET_SIZE 64
#define ECHO_REQUEST 8

struct icmp_header {
	uint8_t		type;
	uint8_t		code;
	uint16_t	checksum;
	uint32_t	payload;
};

int	checksum(void *packet, int len);

int	main(int ac, char **av) {
	if (ac < 2) {
		printf("Usage\n  ping [options] <destination>\n\n");
	}

	int	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockfd == -1) {
		perror("socket");
		return 1;
	}

	struct icmp_header	header;
	struct sockaddr_in	addr;
	char	packet[PACKET_SIZE];

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	if (inet_pton(AF_INET, av[1], &addr.sin_addr) != 1) {
		perror("inet_pton");
		return 1;
	}

	memset(&header, 0, sizeof(header));
	header.type = ECHO_REQUEST;

	memset(packet, 0, sizeof(packet));
	memcpy(packet, &header, sizeof(header));
	((struct icmp_header *)packet)->checksum = checksum(packet, sizeof(packet));

	if (sendto(sockfd, packet, sizeof(packet), 0, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		perror("sendto");
		return 1;
	}

	char	result[PACKET_SIZE];
	socklen_t	len = sizeof(addr);
	if (recvfrom(sockfd, result, sizeof(result), 0, (struct sockaddr *)&addr, &len) == -1) {
		perror("recvfrom");
		return 1;
	}

	printf("result: %s\n", result);
	return 0;
}

int	checksum(void *packet, int len) {
	uint16_t	*buf;
	int			sum;

	buf = packet;
	for (sum = 0; len > 1; len -= 2) {
		sum += *buf++;
	}
	if (len == 1) {
		sum += *(uint8_t *)buf;
	}
	uint8_t	carry;
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	return (~sum);
}
