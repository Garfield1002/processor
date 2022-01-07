#include <stdio.h>

int main(int argc, char **argv)
{
   volatile int x;
	printf("hello\n");
        *(volatile unsigned int *)0x9000 = 0xdeadbeef;
        for (int i = 0; i < 10; i++) x=i;
        *(volatile unsigned int *)0xfffffff8 = 0x00ff3333;
}
