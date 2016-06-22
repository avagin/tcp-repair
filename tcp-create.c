#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/socket.h>
#include <netinet/tcp.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>

#define pr_perror(fmt, ...) do { fprintf(stderr, "%s:%d: " fmt " : %m\n", __func__, __LINE__, ##__VA_ARGS__); return 1; } while (0)

int main(int argc, char **argv)
{
	static const char short_opts[] = "";
	static struct option long_opts[] = {
		{ "saddr",	required_argument, 0, 's' },
		{ "daddr",	required_argument, 0, 'd' },
		{ "sport",	required_argument, 0, 'b' },
		{ "dport",	required_argument, 0, 'p' },
		{ "sseq",	required_argument, 0, 'S' },
		{ "dseq",	required_argument, 0, 'A' },
		{},
	};
	unsigned int seq = 500000, ack = 400000;
	unsigned int src_port = 12345, dst_port = 54321;
	char *src = "localhost", *dst = "localhost";
	struct sockaddr_in addr;
	int sk, yes = 1, val, idx, opt;
	char buf[1024];

	while (1) {
		idx = -1;
		opt = getopt_long(argc, argv, short_opts, long_opts, &idx);
		if (opt == -1)
			break;

		switch (opt) {
		case 's':
			src = optarg;
			break;
		case 'd':
			dst = optarg;
			break;
		case 'b':
			src_port = atol(optarg);
			break;
		case 'p':
			dst_port = atol(optarg);
			break;
		case 'S':
			seq = atol(optarg);
			break;
		case 'A':
			ack = atol(optarg);
			break;
		break;
		}
	}

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

	if (write(STDOUT_FILENO, "start", 5) != 5)
		pr_perror("write");
	if (read(STDIN_FILENO, buf, 5) != 5)
		pr_perror("read");

	val = 0;
	if (setsockopt(sk, SOL_TCP, TCP_REPAIR, &val, sizeof(val)))
		pr_perror("TCP_REPAIR");

	if (write(sk, argv[optind], strlen(argv[optind])) < 0)
		pr_perror("write");

	if (read(sk, buf, sizeof(buf)) < 0)
		pr_perror("read");

	printf("%s\n", buf);

	return 0;
}
