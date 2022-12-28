#include <stdio.h>
#include <string.h>

int chrat(char *str, char key) {
	char *pch = strchr(str,key);
	return (int)(pch-str);
}
