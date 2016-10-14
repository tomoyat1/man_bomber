#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "man_bomber.h"
#include "man_bomber_config.h"

int srv_init()
{
	FILE *pidfile;
	int logfd, errfd;
	/* pid file */
	pidfile = fopen("/var/tmp/manbomber-srv.pid", "w+");
	fprintf(pidfile, "%d\n", getpid());
	fflush(pidfile);
	fclose(pidfile);

	/* fd handling */
	logfd = open("/var/tmp/manbomber-srv.log",
	     O_CREAT|O_WRONLY|O_APPEND, 0755);
	errfd = open("/var/tmp/manbomber-srv.error",
	     O_CREAT|O_WRONLY|O_APPEND, 0755);
	close(2);
	dup(errfd);
	close(1);
	dup(logfd);
	close(0);
}

int srv_loop()
{
	int i;
	FILE *log, *err;
	time_t start_time;
	struct tm *tmptr;
	char fmt_time[64];
	log = fdopen(1, "a");
	err = fdopen(2, "a");
	/* Log server startup */
	start_time = time(NULL);
	tmptr = localtime(&start_time);
	strftime(fmt_time, sizeof(fmt_time), "%F %H:%M", tmptr);

	fprintf(stdout, "%s\n", fmt_time);
	for (i = 0; 1; i++) {
		fprintf(stdout, "log: %d\n", i);
		fprintf(stderr, "err: %d\n", i);
		sleep(3);
	}
	return 0;
}

int main(int argc, char **argv)
{
	int srv_loop_ret;
	pid_t pid, sid;
	printf("Start $ man bomber server version %d.%d\n",
	    man_bomber_VERSION_MAJOR,
	    man_bomber_VERSION_MINOR);
	pid = fork();
	if (pid == 0) {
		/* Start server loop */
		umask(0);
		srv_init();
		sid = setsid();
		srv_loop_ret = srv_loop();
		exit(srv_loop_ret);
	} else {
		printf("Child pid: %d\n", pid);
		sleep(1);
		execlp("man", "man", "man", NULL);
		exit(0);
	}
}
