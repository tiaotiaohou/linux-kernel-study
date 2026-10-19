.section .rodata
msg_pos: .ascii "positive\n"
.equ POS_LEN, . - msg_pos
msg_nonpos: .ascii "non-positive\n"
.equ NONPOS_LEN, . - msg_nonpos
.section .text
.global _start
_start:
    mov x9, #VALUE
    cmp x9, #0
    b.gt positive
    adr x1, msg_nonpos
    mov x2, #NONPOS_LEN
    b print
positive:
    adr x1, msg_pos
    mov x2, #POS_LEN
print:
    mov x0, #1
    mov x8, #64
    svc #0
    mov x0, #0
    mov x8, #93
    svc #0
