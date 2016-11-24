CFLAGS += -Wall -g -I../criu/
LDFLAGS += -L../criu/soccr -lsoccr ../criu/soccr/libsoccr.a -lnet

RUN ?= tcp-constructor

run:
	./local.sh

tcp-constructor: tcp-constructor.c ../criu/soccr/libsoccr.a
	$(CC) $(CFLAGS) tcp-constructor.c -o tcp-constructor $(LDFLAGS)

clean:
	rm -f tcp-constructor

test: tcp-constructor
	python run.py ./$(RUN)

.PHONY: test

