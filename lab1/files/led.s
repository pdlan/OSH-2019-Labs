.section .init
.global _start
_start:
ldr r0, =0x3F200000
ldr r1, =0x3F003000
mov r2, #1
lsl r2, #27
str r2, [r0, #28]
mov r7, #1000
mul r6, r7, r7
ldr r4, [r1, #4]
add r4, r4, r6
add r5, r4, r6
add r6, r6, r6
loop:
ldr r3, [r1, #4]
cmp r3, r4
beq on
cmp r3, r5
beq off
b loop
on:
add r4, r4, r6
mov r3, #0
str r3, [r0, #40]
mov r3, #1
lsl r3, #29
str r3, [r0, #28]
b loop
off:
add r5, r5, r6
mov r3, #0
str r3, [r0, #28]
mov r3, #1
lsl r3, #29
str r3, [r0, #40]
b loop
