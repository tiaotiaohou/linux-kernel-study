.text
.p2align 2
.global main
.type main, %function
main:
    stp x29, x30, [sp, #-16]!
    mov x29, sp
    mov w0, #8
    mov w1, #11
    bl c_add
    mov w1, w0
    adrp x0, fmt
    add x0, x0, :lo12:fmt
    bl printf
    mov w0, #0
    ldp x29, x30, [sp], #16
    ret
.size main, . - main
.section .rodata
fmt: .asciz "Student ID: 20262811\nASM calls C: 8 + 11 = %d\n"
.section .note.GNU-stack,"",%progbits
