#!/usr/bin/env python2

import socket, struct, getopt, sys, os

opts, args = getopt.gnu_getopt(sys.argv, '', ['port=', 'seq=', 'addr=', 'next', 'reverse', "snd="])

class Tcp:
    def __init__(self):
        self.addr = "localhost"
        self.port = "12345"
        self.mss = 1460
        self.wscale = 7
        self.seq = 5000000

tcp = Tcp()
conn = []
reverse = False
snd = ""

for k, v in opts:
    if k == "--addr":
        tcp.addr = v
    if k == "--port":
        tcp.port = int(v)
    if k == "--seq":
        tcp.seq = int(v)
    if k == "--next":
        conn.append(tcp)
        tcp = Tcp()
    if k == "--reverse":
        reverse = True
    if k == "--snd":
        snd = v
conn.append(tcp)

if reverse:
    src = 1
    dst = 0
else:
    src = 0
    dst = 1

TCP_REPAIR          = 19
TCP_REPAIR_QUEUE    = 20
TCP_QUEUE_SEQ       = 21
TCP_REPAIR_OPTIONS  = 22
TCPOPT_MSS = 2
TCPOPT_WINDOW = 3
TCPOPT_TIMESTAMP = 8
TCPOPT_SACK_PERM = 4

TCP_RECV_QUEUE = 1
TCP_SEND_QUEUE = 2

# create reply socket with TCP_REPAIR to put it into the correct state
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_TCP, TCP_REPAIR, 1)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

s.setsockopt(socket.SOL_TCP, TCP_REPAIR_QUEUE, TCP_SEND_QUEUE)
seq = struct.pack('=L', int(conn[src].seq))
s.setsockopt(socket.SOL_TCP, TCP_QUEUE_SEQ, seq)

s.setsockopt(socket.SOL_TCP, TCP_REPAIR_QUEUE, TCP_RECV_QUEUE)
ack = struct.pack('=L', int(conn[dst].seq))
s.setsockopt(socket.SOL_TCP, TCP_QUEUE_SEQ, ack)

s.bind((conn[src].addr, conn[src].port))
s.connect((conn[dst].addr, conn[dst].port))

opt = ''
opt += struct.pack('=LHH', TCPOPT_WINDOW,
                       int(conn[src].wscale), int(conn[dst].wscale))
opt += struct.pack('=LL', TCPOPT_MSS, int(conn[src].mss))

#opt += struct.pack('=LL', TCPOPT_SACK_PERM, 0)
#opt += struct.pack('=LL', TCPOPT_TIMESTAMP, 0)

s.setsockopt(socket.SOL_TCP, TCP_REPAIR_OPTIONS, opt)

if snd:
    s.setsockopt(socket.SOL_TCP, TCP_REPAIR_QUEUE, TCP_SEND_QUEUE)
    s.send(snd)

#if ts:
#    ts = struct.pack('=L', int(ts))
#    s.setsockopt(socket.SOL_TCP, TCP_TIMESTAMP, ts)

s.setsockopt(socket.SOL_TCP, socket.TCP_NODELAY, 1)

sys.stdout.write("start")
sys.stdout.flush()
os.read(0, 5)

# we are now ready to send data
s.setsockopt(socket.SOL_TCP, TCP_REPAIR, 0)

os.execvp(args[1], args[1:])
