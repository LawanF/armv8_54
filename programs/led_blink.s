@ Loading values into registers.
movz w0, #0x0 @ Zero.
movz w1, #0x3f20, lsl #16 @ GPFSEL memory location.
movz w2, #0x8000, lsl #12 @ To set pin 9 as output.
add w3, w1, #0x1c @ GPSET0 memory location.
add w4, w1, #0x28 @ GPCLR0 memory location.
movz w5, #0x100 @ To set GPSET and GPCLR for pin 9.
movz w6, #0x7e00, lsl #16 @ System timer base address.
add w6, w6, #0x3000
movz w10, #0x2710 @ Epsilon for time calculation.

@ Setting GPIO pin settings.
str w2, [w0, w1] @ Set pin 9 as output.

@ Waiting procedure.
wait1:
ldr w7, [w6, #0x4] @ Load current time.
add w8, w7, #0xf424, lsl #4 @ Time offset of one million.
wait1_loop:
ldr w7, [w6, #0x4] @ Update current time.
sub w9, w7, w8 @ Calculate time difference.
cmp w10, w9
b.ge wait1_loop

@ GPSET
str w5, [w0, w3] @ Set GPSET for pin 9.

@ Waiting procedure.
wait2:
ldr w7, [w6, #0x4] @ Load current time.
add w8, w7, #0xf424, lsl #4 @ Time offset of one million.
wait2_loop:
ldr w7, [w6, #0x4] @ Update current time.
sub w9, w7, w8 @ Calculate time difference.
cmp w10, w9
b.ge wait2_loop

str w5, [w, w4] @ Set GPCLR for pin 9.
br wait1 @ Repeat loop.
