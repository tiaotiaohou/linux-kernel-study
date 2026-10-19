.section .rodata
msg:
    .ascii "Hello World! Student ID: 20262811\n"
.equ msg_len, . - msg

.section .text
.global _start

_start:
    // write(1, msg, msg_len)
    mov x0, #1
    adr x1, msg
    mov x2, #msg_len
    mov x8, #64
    svc #0

    // exit(0)
    mov x0, #0
    mov x8, #93
    svc #0
