#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "man_bomber_config.h"


int main(int argc, char **argv)
{
	printf("Start $ man bomber server version %d.%d\n",
	    man_bomber_VERSION_MAJOR,
	    man_bomber_VERSION_MINOR);
	sleep(1);
	execlp("man", "man", "man", NULL);
	exit(0);
}
