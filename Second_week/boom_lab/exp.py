#!/usr/bin/env python
# -*- coding: utf-8 -*-
from pwn import *
import sys
context.log_level = 'debug'
s       = lambda x                  :orda.send(str(x))
sa      = lambda x, y                 :orda.sendafter(str(x),str(y)) 
sl      = lambda x                   :orda.sendline(str(x)) 
sla     = lambda x, y                 :orda.sendlineafter(str(x), str(y)) 
r       = lambda numb=4096          :orda.recv(numb)
rc        = lambda                     :orda.recvall()
ru      = lambda x, drop=True          :orda.recvuntil(x, drop)
rr        = lambda x                    :orda.recvrepeat(x)
irt     = lambda                    :orda.interactive()
uu32    = lambda x   :u32(x.ljust(4, '\x00'))
uu64    = lambda x   :u64(x.ljust(8, '\x00'))
db        = lambda    :raw_input()
def getbase_b64(t):
    pid=proc.pidof(s)[0]
    pie_pwd ='/proc/'+str(pid)+'/maps'
    f_pie=open(pie_pwd)
    return f_pie.read()[:12]
if len(sys.argv) > 1:
    s = ""
    host = s.split(":")[0]
    port = int(s.split(":")[1])
    orda = remote(host,port)
else:
    orda = process("./bomb")


sla("hich to blow yourself up. Have a nice day!\n","Border relations with Canada have never been better.")

sla("Phase 1 defused. How about the next one?","1 2 4 8 16 32")

sla("hat's number 2.  Keep going!",'1 311')

sla("alfway there!",'7 0DrEvil')

gdb.attach(orda,'b *0x40126E')
raw_input()
sla("So you got that one.  Try this one.",'9?>567')
sla("Good work!  On to the next...",'4 3 2 1 6 5')
sla("But finding it and solving it are quite different...",22)

irt()
