movz w0, #0x0
ldr w1, gpfselmem
ldr w2, gpfsel4
ldr w3, gpset0mem
ldr w4, gpclr0mem
ldr w5, gpsetclr

str w2, [w1]

wait1:
movz w6, #0x0
ldr w7, waittime
wait1loop:
add w6, w6, #0x1
cmp w6, w7
b.ne wait1loop

gpset:
str w5, [w3]

wait2:
movz w6, #0x0
wait2loop:
add w6, w6, #0x1
cmp w6, w7
b.ne wait2loop

str w5, [w4]

b wait1

gpfselmem:
    .int 0x3f200000
gpfsel4:
    .int 0x1000

gpset0mem:
    .int 0x3f20001c

gpclr0mem:
    .int 0x3f200028

gpsetclr:
    .int 0x10

waittime:
    .int 0x500000

