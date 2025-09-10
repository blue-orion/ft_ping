#include "../includes/ping.h"

void	send_packet(int sockfd, struct sockaddr_in addr) {
	struct icmphdr	icmp_hdr;

	icmp_hdr.type = ICMP_ECHO;
	icmp_hdr.un.echo.id = getpid() & 0xFFFF;
	icmp_hdr.un.echo.sequence = 0;
	
}
