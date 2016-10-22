#include <stdio.h>

#include <sys/socket.h>
#include <unistd.h>

#include "man_bomber.h"
#include "man_bomber_config.h"

int check_magic(int fd, int magic)
{
	int val;
	char *msg;
	recv(fd, &val, sizeof(int), 0);
	if (val == magic)
		return 0;
	else {
		switch (magic) {
		case PLA:
			msg = "should have been PLA";
			break;
		case BOM:
			msg = "should have been BOM";
			break;
		case WAL:
			msg = "should have been WAL";
			break;
		default:
			msg = "Unknown magic";
			break;
		}
		fprintf(stderr, "(Slave: %d) Magic check fail: %s", getpid(), msg);
		return 1;
	}
}
