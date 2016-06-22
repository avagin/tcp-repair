#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/socket.h>
#include <netinet/tcp.h>
#include <string.h>

#define pr_perror(fmt, ...) do { fprintf(stderr, "%s:%d: " fmt " : %m\n", __func__, __LINE__, ##__VA_ARGS__); return 1; } while (0)

int main(int argc, char **argv)
{
	unsigned int seq, ack, src_port, dst_port;
	char src[128], dst[128];
	struct sockaddr_in addr;
	int sk, yes = 1, val, ret;
	char buf[1024];

	ret = sscanf(argv[1], "%s %d %d %s %d %d", src, &src_port, &seq, dst, &dst_port, &ack);
	if (ret != 6)
		pr_perror("scanf -> %d %s", ret, src);

	sk = socket(AF_INET, SOCK_STREAM, 0);
	if (sk < 0)
		pr_perror("socket");

	if (setsockopt(sk, SOL_TCP, TCP_REPAIR, &yes, sizeof(yes)))
		pr_perror("TCP_REPAIR");

	if (setsockopt(sk, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
		pr_perror("setsockopt");

	val = TCP_RECV_QUEUE;
	if (setsockopt(sk, SOL_TCP, TCP_REPAIR_QUEUE, &val, sizeof(val)))
		pr_perror("TCP_RECV_QUEUE");

	val = seq;
	if (setsockopt(sk, SOL_TCP, TCP_QUEUE_SEQ, &val, sizeof(val)))
		pr_perror("TCP_QUEUE_SEQ");

	val = TCP_SEND_QUEUE;
	if (setsockopt(sk, SOL_TCP, TCP_REPAIR_QUEUE, &val, sizeof(val)))
		pr_perror("TCP_SEND_QUEUE");

	val = ack;
	if (setsockopt(sk, SOL_TCP, TCP_QUEUE_SEQ, &val, sizeof(val)))
		pr_perror("TCP_QUEUE_SEQ");

	addr.sin_family = AF_INET;
	addr.sin_port = src_port;
	inet_pton(AF_INET, src, &(addr.sin_addr));

	if (bind(sk, (struct sockaddr *) &addr, sizeof(addr)))
		pr_perror("bind");

	addr.sin_family = AF_INET;
	addr.sin_port = dst_port;
	inet_pton(AF_INET, dst, &(addr.sin_addr));

	if (connect(sk, (struct sockaddr *) &addr, sizeof(addr)))
		pr_perror("bind");

	sleep(5);
	val = 0;
	if (setsockopt(sk, SOL_TCP, TCP_REPAIR, &val, sizeof(val)))
		pr_perror("TCP_REPAIR");

	if (write(sk, argv[2], strlen(argv[2])) < 0)
		pr_perror("write");

	if (read(sk, buf, sizeof(buf)) < 0)
		pr_perror("read");

	printf("%s\n", buf);

	return 0;
}
