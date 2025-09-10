#include "../includes/ping.h"
#include <sys/socket.h>
#include <sys/time.h>

#define TTL 64
#define INTERVAL 1000

int	main(int ac, char **av) {
	if (ac < 2) {
		printf("Usage\n  ping [options] <destination>\n\n");
		return 0;
	}

	// TODO: Check options
	
	int	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockfd == -1) {
		perror("socket");
		return 1;
	}

	// TODO: init_socket();

	struct ping_rts	rts;
	statistic	stat;

	// TODO: set_address();
	addr.sin_family = AF_INET;
	if (inet_pton(AF_INET, av[1], &addr.sin_addr) != 1) {
		perror("inet_pton");
		return 1;
	}

	struct icmphdr	icmp_hdr;
	char	packet[PACKET_SIZE];
	char		result[PACKET_SIZE];
	socklen_t	len;
	struct timeval	st;
	struct timeval	end;
	double	rtt;

	memset(&icmp_hdr, 0, sizeof(icmp_hdr));
	icmp_hdr.type = ICMP_ECHO;
	icmp_hdr.un.echo.id = getpid() & 0xFFFF;
	icmp_hdr.un.echo.sequence = 1;

	printf("PING %s: %d bytes data\n", av[1], DEFDATALEN);
	for (int i = 0; i < 5; i++) {
		gettimeofday(&st, NULL);
		memset(packet, 0, sizeof(packet));
		memcpy(packet, &icmp_hdr, sizeof(icmp_hdr));
		((struct icmphdr *)packet)->checksum = checksum(packet, sizeof(packet));

		if (sendto(sockfd, packet, sizeof(packet), 0, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
			perror("sendto");
			return 1;
		}

		len = sizeof(addr);
		if (recvfrom(sockfd, result, sizeof(result), 0, (struct sockaddr *)&addr, &len) == -1) {
			perror("recvfrom");
			return 1;
		}
		uint16_t chk = checksum(result, sizeof(result));
		if (chk != 0) {
			printf("socket loss: %d\n", checksum(result, sizeof(result)));
			return 0;
		}
		gettimeofday(&end, NULL);
		
		rtt = (end.tv_sec - st.tv_sec) * 1000.0;
		rtt += (end.tv_usec - st.tv_usec) / 1000.0;

		printf("%ld btyes from %s: icmp_seq=%d ttl=%d time=%.2f ms\n", sizeof(result), av[1], icmp_hdr.un.echo.sequence, TTL, rtt);
		icmp_hdr.un.echo.sequence++;
		usleep(1000000);
	}
	return 0;
}
