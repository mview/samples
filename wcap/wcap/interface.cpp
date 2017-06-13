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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef __cplusplus
extern "C" {
#endif

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#ifdef __cplusplus
}
#endif

static lua_State *L;
void sendrmsg(int fd,int n)
{
    const char* rmsg="HTTP/1.1 200 OK\r\n"
        "Server: httpd/0.1.0\r\n"
        "Content-Length: ";
    char head[512];
    sprintf(head,"%s%d\r\n\r\n",rmsg,n);
    int len=strlen(head);
    write(fd,head,len);
}
static void sendframe2socket(int fd,const char *filename)
{
	FILE *fp;
    int n;
	fp=fopen(filename,"rb");
    fseek(fp,0L,SEEK_END);
    n=ftell(fp); 
    fseek(fp,0L,SEEK_SET); 
    sendrmsg(fd,n);
	while(!feof(fp)){
		char buf[1024];
		int len;
		len=fread(buf,1,1024,fp);
		write(fd,buf,len);
	}
	fclose(fp);
}
static void fdsend(int fd,const char *title,const  char *head,const  char *body)
{
    if(title==NULL && head==NULL){
        sendrmsg(fd,strlen(body));
		write(fd,body,strlen(body));
		return;
	}
	char buffer[4096];
    int n;
	n=sprintf(buffer,"<HEAD><TITLE>HTTP Server Message</TITLE></HEAD>\n<BODY>\n");
	if(title) {
		n+=sprintf(&buffer[n],"<H1>%s</H1>\n",title);
	}
	if(head) {
		n+=sprintf(&buffer[n],"<H2>%s</H2>\n",head);
	}
	if(body) {
		n+=sprintf(&buffer[n],"%s<P>\n",body);
	}
	n+=sprintf(&buffer[n],"</BODY>\n");
    sendrmsg(fd,n);
	write(fd,buffer,n);
}

void sendfile2socket(int fd,const char *dir,const char *file)
{
	char fullname[512];
	char *buffer;
	FILE *fp;
	int n;
	sprintf(fullname,"./%s/%s",dir,file);
	fp=fopen(fullname,"rb");
	if(fp==NULL) {
		fdsend(fd,"Error 405","no such file",fullname);
		return;
	}
    fseek(fp,0L,SEEK_END);
	n=ftell(fp); 
	buffer=(char *)malloc(n+1);
	fseek(fp,0L,SEEK_SET); 
	fread(buffer,n,1,fp); 
    sendrmsg(fd,n);
	write(fd,buffer,n);
	fclose(fp);
    
}
static int wcap_sendfile(lua_State *L)
{
	sendfile2socket(lua_tonumber(L, 1),lua_tostring(L, 2),lua_tostring(L, 3));
	return 1;
}
static int wcap_sendframe(lua_State *L)
{
	sendframe2socket(lua_tonumber(L, 1),lua_tostring(L, 2));
	return 1;
}
static int wcap_sendmsg(lua_State *L)
{
	fdsend(lua_tonumber(L, 1),lua_tostring(L, 2),lua_tostring(L, 3),lua_tostring(L, 4));
	return 1;
}


void openLuaService()
{
	L = lua_open();
	luaL_openlibs(L);
	lua_register(L,"wcap_sendmsg", wcap_sendmsg);
	lua_register(L,"wcap_sendfile", wcap_sendfile);
	lua_register(L,"wcap_sendframe", wcap_sendframe);
}
void runLua(int socket,char *buffer)
{
    const char *filename="scripts/analyze.lua";
	FILE* fp=fopen(filename,"r");
	if(fp==NULL){
		fdsend(socket,"Error 404","enviroment not setup","homepage is NULL");
		return;
	} 
	if(luaL_loadfile(L,filename) || lua_pcall(L,0,0,0))
		printf("Error failed to load: %s",lua_tostring(L,-1));
	
	lua_getglobal(L, "analyze");
    lua_pushnumber(L,socket);
	lua_pushstring(L,buffer);
	if(lua_pcall(L,2,1,0))
		printf("Error failed to load: %s",lua_tostring(L,-1));
	int ret=lua_tonumber(L, -1);
    lua_pop(L, 1);
}
