#include <stdio.h>
extern int asm_add(int a, int b);
int main(void)
{
    int result = asm_add(8, 11);
    printf("Student ID: 20262811\n");
    printf("C calls ASM: 8 + 11 = %d\n", result);
    return 0;
}
