#!/usr/bin/env python2

import sys
from subprocess import Popen, PIPE

str1 = "test test test"
str2 = "Test Test Test"

args = ["./tcp-create", "--addr", "localhost", "--port", "12342", "--seq", "555", "--next", "--addr", "localhost", "--port", "54322", "--seq", "666", "--reverse", "--", "./tcp-test.py"]

p1 = Popen(args + [str1], stdout = PIPE, stdin = PIPE)

args.remove("--reverse");

p2 = Popen(args + [str2], stdout = PIPE, stdin = PIPE)

p1.stdout.read(5)
p2.stdout.read(5)
p1.stdin.write("start")
p2.stdin.write("start")

if p1.wait():
    sys.exit(1)
if p2.wait():
    p2.wait()

s = p1.stdout.read()
if str2 + "\n" != s:
    print "FAIL", repr(str2), repr(s)
    sys.exit(5);
s = p2.stdout.read()
if str1 + "\n" != s:
    print "FAIL", repr(str1), repr(s)
    sys.exit(5);
print "PASS"
