movz w0, #0x0 @ Zero.
movz w1, #0x3f20, lsl #16 @ GPFSEL memory location.
movz w2, #0x8000, lsl #12 @ To set pin 9 as output.
add w3, w1, #0x1c @ GPSET0 memory location.
add w4, w1, #0x28 @ GPCLR0 memory location.
movz w5, #0x100 @ To set GPSET and GPCLR for pin 9.

str w2, [w0, w1] @ Set pin 9 as output.

wait1:
movz w6, #0x0 @ Set timer to 0.
movz w7, #0x3b9a, lsl #16 @ Timer target.
wait1_loop:
add w6, w6, #0x1
cmp w6, w7
br.ne wait1_loop

str w5, [w0, w3] @ Set GPSET for pin 9.

wait2:
movz w6, #0x0 @ Set timer to 0.
movz w7, #0x3b9a, lsl #16 @ Timer target.
wait2_loop:
add w6, w6, #0x1
cmp w6, w7
br.ne wait2_loop

str w5, [w0, w4] @ Set GPCLR for pin 9.
br wait1 @ Repeat loop.
