# for example
export TCP_SRC="192.168.122.1";
export TCP_DST="192.168.122.142";
export TCP_SSHKEY="/home/avagin/.ssh/id_rsa"
export TCP_SPORT=$((`date +%s` % 1000 + 12000))
export TCP_DPORT=$((`date +%s` % 1000 + 12001))
make test
