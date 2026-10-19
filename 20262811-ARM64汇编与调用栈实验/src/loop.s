.section .data
digit: .ascii "0\n"
.section .text
.global _start
_start:
    mov x19, #1
loop:
    add w20, w19, #48
    adr x1, digit
    strb w20, [x1]
    mov x0, #1
    mov x2, #2
    mov x8, #64
    svc #0
    add x19, x19, #1
    cmp x19, #5
    b.le loop
    mov x0, #0
    mov x8, #93
    svc #0
