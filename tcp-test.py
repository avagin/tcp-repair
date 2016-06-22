#!/usr/bin/env python2

import os, sys, socket

sk = socket.fromfd(3, socket.AF_INET, socket.SOCK_STREAM)

sk.send(sys.argv[1])
sk.shutdown(socket.SHUT_WR)
print sk.recv(1024)
