CFLAGS += -Wall -g

run:
	./local.sh

tcp-create: tcp-create.c

clean:
	rm -f tcp-create

test: tcp-create
	python run.py

.PHONY: test

