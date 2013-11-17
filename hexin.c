#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <string.h>

#define ZBUFSIZ 1024

#define PS1 "PS1=\\[\\033[1;30m\\][\\[\\033[0;32m\\]\\u\\[\\033[1;32m\\]@\\[\\033[0;32m\\]\\h \\[\\033[1;37m\\]\\W\\[\\033[1;30m\\]]\\[\\033[0m\\]# "
struct  termios oldterm, newterm, tmpterm;

// restore terminal configuration and exit
void sig_die(int n) {
	tcsetattr(0, TCSAFLUSH, &oldterm);
	exit(1);
}


int main(int argc, char **argv){

	printf("Hexin Terminal v0.2 - z0nKT1g3r & whats\n");

	// initialize
	char *buf = malloc(ZBUFSIZ);
	int tty, pty;

	// used to get the tty
	extern char *ptsname();
	pty = open("/dev/ptmx", O_RDWR);
	grantpt(pty);
	unlockpt(pty);
	tty = open(ptsname(pty), O_RDWR);

	// child
	if(!fork()) {
		//close(pty);
		// new session to be used with bash
		setsid();
		ioctl(tty, TIOCSCTTY, NULL);
		// start using the new tty
		dup2(tty, 0);
		dup2(tty, 1);
		dup2(tty, 2);
		close(tty);

		// reset the signals
		int i;
		for (i = 1; i < 64; i++)
			signal(i, SIG_DFL);
		
		// overwrite the PS1 to know that you are in "hexin mode"
		putenv(PS1);
		execl("/bin/bash", "bash", "--norc", "-i", NULL);
		
		printf("despres\n");
		// we should not to be here 
		exit(1);
	}

	// parent
	else{
		// save our terminal configuration
		tcgetattr(0, &oldterm);
		// whait for a second
		sleep(1);
		// set new terminal configuracion
		newterm = oldterm;
		newterm.c_lflag &= ~(ICANON | ECHO | ISIG);
		newterm.c_iflag &= ~(IXON | IXOFF);
		tcsetattr(0, TCSAFLUSH, &newterm);
		
		// overwrite the signals to restore our terminal configuration
		signal(SIGCHLD, sig_die);
		signal(SIGINT, sig_die);
		signal(SIGQUIT, sig_die);
		signal(SIGILL, sig_die);
		signal(SIGABRT, sig_die);
		signal(SIGSEGV, sig_die);
		signal(SIGTERM, sig_die);

		int state = 0;
		char hex[2];
		// main while
		while (1) {
				fd_set  fds;
				int count;
		
				// put the fd to watch
				FD_ZERO(&fds);
				FD_SET(0, &fds);
				FD_SET(pty, &fds);
		
				if (select(pty + 1, &fds, NULL, NULL, NULL) < 0) {
						if (errno == EINTR)
								continue;
						break;
				}
		
				/* stdin => shell */
				if (FD_ISSET(0, &fds)) {
					if (state == 0) {
						count = read(0, buf, 1);
						if ((count <= 0) && (errno != EINTR))
								break;
						/* START TO MODIFY */
						if (buf[0] == '\\') {
							printf("\\");fflush(stdout);
							state = 1;
							// set the terminal to only send the string to the shell, when the \n is received
						    tmpterm = oldterm;
					        tmpterm.c_lflag &= ~ISIG | ~ECHO | ICANON;
					        tmpterm.c_iflag &= ~(IXON | IXOFF);	
							tcsetattr(0, TCSAFLUSH, &tmpterm);
						} else {
							if ((write(pty, buf, count) < 0) && (errno != EINTR))
								break;
						}
					} else {
						buf[0] = '\\';
						count = read(0, buf+1, ZBUFSIZ);
						if ((count <= 0) && (errno != EINTR))
								break;
						//if ((write(1, buf, count) < 0) && (errno != EINTR))
//					    tmpterm.c_lflag &= ~ECHO;
//						tcsetattr(0, TCSAFLUSH, &tmpterm);
						//		break;
						filter(buf, count);
						if ((write(pty, buf, count) < 0) && (errno != EINTR))
								break;
						//state = 0;
						// set the terminal to send every char immediatly to the shell
						tcsetattr(0, TCSAFLUSH, &newterm);
						memset(buf, 0, ZBUFSIZ);
					}
						/* END TO MODIFY */
				/* shell => stdout */
				} else if (FD_ISSET(pty, &fds)) {
						count = read(pty, buf, ZBUFSIZ);
						if ((count <= 0) && (errno != EINTR))
								break;
						if (state == 0) write(1, buf, count);
						else state = 0;
				}
		}
		// restore terminal configuration
		tcsetattr(0, TCSAFLUSH, &oldterm);
	}

	//Thank you for using Hexin
	exit(0);

}//end of main: hexin

//Processes \x**
int filter(char *buf, int size){
	int i;
	int offset;
	char *buf2;
	char *ptr;
	char ch;
	char hex[2];

	buf2=malloc(ZBUFSIZ);

	for(i=0,offset=0;i<size;i++,offset++){
		if(buf[i]=='\\'){
			switch(buf[i+1]){
				case '\\':
					buf2[offset]='\\';
					break;
				case 'n':
					buf2[offset]='\n';
					break;
				case 'r':
					buf2[offset]='\r';
					break;
				case 't':
					buf2[offset]='\t';
					break;
				case 'x':
					hex[0]=buf[i+2];
					hex[1]=buf[i+3];
					buf2[offset]=strtol(hex,&ptr,16);
					i+=2;
					break;
				default:
					buf2[offset]='\\';
					buf2[++offset]=buf[i+1];
					break;
			}
			i++;
		}
		else{
			buf2[offset]=buf[i];
		}
	}
	buf2[offset] = '\n';

	memset(buf,0,ZBUFSIZ);
	memcpy(buf, buf2, ZBUFSIZ);

	free(buf2);

	return offset;

}//end of filter

