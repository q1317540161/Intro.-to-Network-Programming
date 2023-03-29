#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int main(){
	int seed = 0x2aeb0169;
	srand(seed);
	static int len;
	static char buf[64];
	rand();
	rand();
	rand();
	rand();
	rand();
	len  = snprintf(buf,     10, "%x", rand());
	len += snprintf(buf+len, 10, "%x", rand());
	len += snprintf(buf+len, 10, "%x", rand());
	len += snprintf(buf+len, 10, "%x", rand());
	buf[len] = '\0';
	printf("%s\n", buf);
}