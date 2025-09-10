#include <arpa/inet.h>
#include <errno.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define PACKET_SIZE 64

unsigned short checksum(void *b, int len) {
  unsigned short *buf = b;
  unsigned int sum = 0;
  unsigned short result;

  for (sum = 0; len > 1; len -= 2)
    sum += *buf++;
  if (len == 1)
    sum += *(unsigned char *)buf;
  sum = (sum >> 16) + (sum & 0xFFFF);
  sum += (sum >> 16);
  result = ~sum;
  return result;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <host>\n", argv[0]);
    return 1;
  }

  int sockfd;
  struct sockaddr_in addr;
  struct icmphdr icmp_hdr;
  char packet[PACKET_SIZE];
  struct timeval start, end;
  double rtt;

  sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (sockfd < 0) {
    perror("socket");
    return 1;
  }

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  if (inet_pton(AF_INET, argv[1], &addr.sin_addr) != 1) {
    perror("inet_pton");
    return 1;
  }

  memset(&icmp_hdr, 0, sizeof(icmp_hdr));
  icmp_hdr.type = ICMP_ECHO;
  icmp_hdr.un.echo.id = getpid() & 0xFFFF;
  icmp_hdr.un.echo.sequence = 1;

  memset(packet, 0, PACKET_SIZE);
  memcpy(packet, &icmp_hdr, sizeof(icmp_hdr));
  ((struct icmphdr *)packet)->checksum = checksum(packet, PACKET_SIZE);

  gettimeofday(&start, NULL);
  if (sendto(sockfd, packet, PACKET_SIZE, 0, (struct sockaddr *)&addr,
             sizeof(addr)) <= 0) {
    perror("sendto");
    return 1;
  }

  socklen_t addrlen = sizeof(addr);
  if (recvfrom(sockfd, packet, PACKET_SIZE, 0, (struct sockaddr *)&addr,
               &addrlen) <= 0) {
    perror("recvfrom");
    return 1;
  }
  gettimeofday(&end, NULL);

  rtt = (end.tv_sec - start.tv_sec) * 1000.0;
  rtt += (end.tv_usec - start.tv_usec) / 1000.0;

  printf("Reply from %s: time=%.2f ms\n", argv[1], rtt);

  close(sockfd);
  return 0;
}
