CFLAGS += -Wall -g

run:
	./local.sh

tcp-constructor: tcp-constructor.c

clean:
	rm -f tcp-constructor

test: tcp-constructor
	python run.py

.PHONY: test

