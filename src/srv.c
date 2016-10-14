#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int main(int argc, char **argv)
{
	execlp("man", "man", "man", NULL);
	exit(0);
}
