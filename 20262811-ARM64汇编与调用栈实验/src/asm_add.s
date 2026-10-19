.text
.p2align 2
.global asm_add
.type asm_add, %function
asm_add:
    add w0, w0, w1
    ret
.size asm_add, . - asm_add
.section .note.GNU-stack,"",%progbits
