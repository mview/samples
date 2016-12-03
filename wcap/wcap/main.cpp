/*
 * Copyright 2016 Joey  <joeyye@foxmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

#define wait3(status, options, rusage) wait4(-1, status, options, rusage)
#define NONE (fd_set *) NULL
#define NEVER (struct timeval *) NULL
#define IGNORE (struct sockaddr *) NULL

int debug = 0;
int httpport=8000;
int s;
fd_set active;
struct sockaddr_in sc_in;
int j;

void openLuaService();
void runLua(int socket,char *buffer);

void generateSettings(const char *arch)
{
	char buf[256];
	FILE *fp;
	sprintf(buf,"var settings={ \"arch\": \"%s\" }",arch);
	fp=fopen("settings.js","w");
	if(fp){
		fwrite(buf,1,strlen(buf),fp);
	}
	fclose(fp);
	
}

/*********************************************/
int fdserve(int fd)
{
	int n;
	char buffer[4096];
	

	n=read(fd,buffer,sizeof(buffer));
	if(n<1) {
		if(debug) {
			if(n<0) perror("read");
			printf("Nothing read.\n");
		}
		return 0;
	}

	runLua(fd,buffer);
	shutdown(fd, SHUT_RDWR);
	close(fd);
	return 1;
}

int main(int argc, char *argv[])
{
	int children= 0;
	int uw;	
	struct timeval tv;
	extern int optind;
	extern char *optarg;
	int c,port;
	int last;

	port=httpport;
	while((c=getopt(argc,argv,"dp:l:"))!=-1) 
		switch(c) {
			case 'd': debug++; break;
			case 'p': port=atoi(optarg); break;
			default: printf("Usage: %s [-d] [-p port]\n",argv[0]);
				exit(1);
		}


	signal(SIGHUP,SIG_IGN); 
	generateSettings(TARGET_ARCH);

	if(debug) printf("Using port %d\n",port);
    
	openLuaService();

	/* open port to listen to */
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		if(debug) perror("socket");
		return 0;
	}
	sc_in.sin_family = AF_INET;
	sc_in.sin_addr.s_addr = INADDR_ANY;
	sc_in.sin_port = htons((u_short) port);
	if (bind(s, (struct sockaddr *) &sc_in, sizeof(sc_in)) < 0) {
		perror("bind: ");
		return 0;
	}
	if (listen(s, 25) < 0) {
		perror("listen");
		return 0;
	}

	/* now try and set the socket option to allow reuse of address */
	{
		int b;
		b=-1;
		if(setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&b,4)) {
			perror("setsockopt");
			exit(1);
		}
	}

	/* set up fd mask */
	FD_ZERO(&active);
	FD_SET(s,&active);

	if(!debug)  {
		if( fork() ) exit(0);
		setsid(); 
	}

	children=0;
	last = s;
	for( ;; ) {
		while(wait3(&uw,WNOHANG,NULL)>0) {
			children--;
		}
		tv.tv_sec = 60;
		tv.tv_usec = 0;
		FD_ZERO(&active);
		FD_SET(s,&active);
		if (select(FD_SETSIZE, &active, NONE, NONE, &tv) < 0) {
			if(debug)
			perror("select");
			break;
		}
		if( FD_ISSET(s,&active) ) {
			if ((j = accept(s, IGNORE, (socklen_t *) 0)) < 0) {
				if(debug) perror("accept");
			} else {
				last=j;
				children++;
				if(!fork()) {
					close(s);
					fdserve(j);
					exit(0);
				}
				close(j);
			}
		}
	}
	return 0;
}
