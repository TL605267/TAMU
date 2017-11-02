#include <stdio.h>

void main() {
	char ch;
	printf("Please type a character: ");
	ch = getc(stdin);
	printf("The character was '%c'\n", ch);
}
