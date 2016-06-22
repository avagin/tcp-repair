CFLAGS += -Wall -g

tcp-create: tcp-create.c

clean:
	rm -f tcp-create

test: tcp-create
	unshare -Urn bash -c 'ip link set up lo; python run.py'

.PHONY: test

