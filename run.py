#!/usr/bin/env python2

import sys
from subprocess import Popen, PIPE

str1 = "test test test"
str2 = "Test Test Test"

p1 = Popen(["./tcp-create", "--saddr", "localhost", "--sport", "12342", "--sseq", "555", "--daddr", "localhost", "--dport", "54322", "--dseq", "666", "--", "./test.py", str1], stdout = PIPE, stdin = PIPE)

p2 = Popen(["./tcp-create", "--saddr", "localhost", "--sport", "54322", "--sseq",  "666", "--daddr", "localhost", "--dport", "12342", "--dseq", "555", "--", "./test.py", str2], stdout = PIPE, stdin = PIPE)

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
