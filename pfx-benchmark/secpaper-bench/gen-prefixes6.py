#!/usr/bin/python

import random
import sys

r = random.Random()
m = set()

def gen_byte(count=1):
    result = []
    for i in range(0,count):
        result.append(int(r.randint(0,65535)))
    return result

def gen_asn():
    return r.randint(0, 65535)

def gen_mask(start, end):
    return r.randint(start, end)


smask=int(sys.argv[2])
emask=int(sys.argv[3])

i = 0
while i < int(sys.argv[1]):
    min_mask = gen_mask(smask, emask)
    max_mask = gen_mask(min_mask, emask)
    bytes = gen_byte(3)
    addr=""
    for a in bytes:
        addr +="%x:" % int(a)
    addr = addr.rstrip(":")
    roa = ("%s:: %s %s %s" % (addr, min_mask, max_mask,gen_asn()))
    if roa not in m:
        m.add((roa, min_mask, max_mask))
        i += 1
        print(roa)
