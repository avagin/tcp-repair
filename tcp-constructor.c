#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/socket.h>
#include <netinet/tcp.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>

#define pr_perror(fmt, ...) ({ fprintf(stderr, "%s:%d: " fmt " : %m\n", __func__, __LINE__, ##__VA_ARGS__); 1; })

struct tcp {
	char *addr;
	uint32_t port;
	uint32_t seq;
	uint16_t mss_clamp;
	uint16_t wscale;
};

static void usage()
{
	printf(
		"Usage: --addr ADDR -port PORT --seq SEQ --next --addr ADDR -port PORT --seq SEQ -- CMD ...\n"
		"\t Describe a source side of a connection, then set the --next option\n"
		"\t and describe a destination side.\n"
		"\t --reverse - swap source and destination sides\n"
		"\t The idea is that the same command line is execute on both sides,\n"
		"\t but the --reverse is added to one of them.\n"
		"\n"
		"\t CMD ... - a user command to handle a socket, which is the descriptor 3.\n"
		"\n"
		"\t It prints the \"start\" on stdout when a socket is created and\n"
		"\t resumes it when you write \"start\" to stdin.\n"
	);
}

int main(int argc, char **argv)
{
	static const char short_opts[] = "";
	static struct option long_opts[] = {
		{ "addr",	required_argument, 0, 'a' },
		{ "port",	required_argument, 0, 'p' },
		{ "seq",	required_argument, 0, 's' },
		{ "next",	no_argument, 0, 'n'},
		{ "reverse",	no_argument, 0, 'r'},
		{},
	};
	struct tcp tcp[2] = {
				{"localhost", 12345, 5000000, 1460, 7},
				{"localhost", 54321, 6000000, 1460, 7}
			};

	int sk, yes = 1, val, idx, opt, i, src = 0, dst = 1, onr = 0;
	struct tcp_repair_opt opts[4];
	struct sockaddr_in addr;
	char buf[1024];

	i = 0;
	while (1) {
		idx = -1;
		opt = getopt_long(argc, argv, short_opts, long_opts, &idx);
		if (opt == -1)
			break;

		switch (opt) {
		case 'a':
			tcp[i].addr = optarg;
			break;
		case 'p':
			tcp[i].port = atol(optarg);
			break;
		case 's':
			tcp[i].seq = atol(optarg);
			break;
		case 'n':
			i++;
			if (i > 1)
				return pr_perror("--next is used twice or more");
			break;
		case 'r':
			src = 1; dst = 0;
			break;
		default:
			usage();
			return 3;
		}
	}
	if (i != 1)
		return pr_perror("--next is required");

	if (optind == argc) {
		usage();
		return 1;
	}

	for (i = 0; i < 2; i++)
		fprintf(stderr, "%s:%d:%d\n", tcp[i].addr, tcp[i].port, tcp[i].seq);

	sk = socket(AF_INET, SOCK_STREAM, 0);
	if (sk < 0)
		return pr_perror("socket");

	if (setsockopt(sk, SOL_TCP, TCP_REPAIR, &yes, sizeof(yes)))
		return pr_perror("TCP_REPAIR");

	if (setsockopt(sk, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
		return pr_perror("setsockopt");

	/* ============= Restore TCP properties ==================*/
	val = TCP_SEND_QUEUE;
	if (setsockopt(sk, SOL_TCP, TCP_REPAIR_QUEUE, &val, sizeof(val)))
		return pr_perror("TCP_RECV_QUEUE");

	val = tcp[src].seq;
	if (setsockopt(sk, SOL_TCP, TCP_QUEUE_SEQ, &val, sizeof(val)))
		return pr_perror("TCP_QUEUE_SEQ");

	val = TCP_RECV_QUEUE;
	if (setsockopt(sk, SOL_TCP, TCP_REPAIR_QUEUE, &val, sizeof(val)))
		return pr_perror("TCP_SEND_QUEUE");

	val = tcp[dst].seq;
	if (setsockopt(sk, SOL_TCP, TCP_QUEUE_SEQ, &val, sizeof(val)))
		return pr_perror("TCP_QUEUE_SEQ");

	/* ============= Bind and connect ================ */
	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(tcp[src].port);
	if (inet_pton(AF_INET, tcp[src].addr, &(addr.sin_addr)) < 0)
		return pr_perror("inet_pton");

	if (bind(sk, (struct sockaddr *) &addr, sizeof(addr)))
		return pr_perror("bind");

	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(tcp[dst].port);
	if (inet_pton(AF_INET, tcp[dst].addr, &(addr.sin_addr)) < 0)
		return pr_perror("inet_pton");

	if (connect(sk, (struct sockaddr *) &addr, sizeof(addr)))
		return pr_perror("bind");

	opts[onr].opt_code = TCPOPT_WINDOW;
	opts[onr].opt_val = tcp[src].wscale + (tcp[dst].wscale << 16);
	onr++;

	opts[onr].opt_code = TCPOPT_MAXSEG;
	opts[onr].opt_val = tcp[src].mss_clamp;
	onr++;

	if (setsockopt(sk, SOL_TCP, TCP_REPAIR_OPTIONS,
			opts, onr * sizeof(struct tcp_repair_opt)) < 0) {
		return pr_perror("Can't repair options");
	}

	/* Let's go */
	if (write(STDOUT_FILENO, "start", 5) != 5)
		return pr_perror("write");
	if (read(STDIN_FILENO, buf, 5) != 5)
		return pr_perror("read");

	val = 0;
	if (setsockopt(sk, SOL_TCP, TCP_REPAIR, &val, sizeof(val)))
		return pr_perror("TCP_REPAIR");

	execv(argv[optind], argv + optind);

	return pr_perror("Unable to exec %s", argv[optind]);
}
