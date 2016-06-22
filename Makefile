CFLAGS += -Wall

tcp-create: tcp-create.c

test:
	unshare -Urn bash -c 'ip link set up lo; python run.py'
