#!/usr/bin/python

import random
import sys
import struct
import socket

r = random.Random()
m = set()

def gen_ip():
    return "%s.%s.%s.%s" % (r.randint(0,255), r.randint(0,255),
            r.randint(0,255), r.randint(0,255))

    return r.randint(0,255)

def gen_asn():
    return r.randint(0, (2**16))

def gen_mask(start, end):
    return r.randint(start, end)

def int2ip( intip ):
    octet = ''
    for exp in [3,2,1,0]:
        octet = octet + str(intip / ( 256 ** exp )) + "."
        intip = intip % ( 256 ** exp )
    return(octet.rstrip('.'))

smask=int(sys.argv[2])
emask=int(sys.argv[3])

i = 0
while i < int(sys.argv[1]):
    min_mask = gen_mask(smask, emask)
    max_mask = gen_mask(min_mask, emask)

    ip = gen_ip()
    ipaddr = struct.unpack('>L',socket.inet_aton(ip))[0]
    ipaddr_masked = ipaddr & (4294967295<<(32-int(min_mask)))
    ip = socket.inet_ntoa(struct.pack('>L',ipaddr_masked))

    roa = ("%s %s %s %s" % (ip, min_mask, max_mask, gen_asn()))
    if roa not in m:
        m.add((ip, min_mask))
        i += 1
        print(roa)
