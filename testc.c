#include <stdio.h>
#include <string.h>

int chrat(char *str, char key) {
	char *pch = strchr(str,key);
	return (int)(pch-str);
}

void main() {
	char tmp[] = "test=123";
	int x = chrat(tmp,'=');
	//char *e = strchr(tmp,'=');
	//int x = (int)(e-tmp);
	printf("hola! pos: %i\n",x);
}
