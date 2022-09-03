#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

void *thread(void *vargp);
void doit(int fd);
int parse_url(char *url,char *host,
        char *port,char *uri);
void clienterror(int fd,char *cause,char *errnum,
        char *shortmsg,char *longmsg);
void request_server(int fd,rio_t *cli_req,
        char *host,char *uri,char *method);
void send_client(int fd,int srv_fd);
void use_serverhdrs(int fd,rio_t *srv_req,
        size_t *content_length);

int main(int argc, char **argv)
{
    int *connfdp,listenfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;
    if(argc!=2){
        fprintf(stderr,"usage: %s <port>\n",argv[0]);
        exit(1);
    }
    listenfd=Open_listenfd(argv[1]);
    while(1){
        clientlen=sizeof(clientaddr);
        connfdp=Malloc(sizeof(int));
        *connfdp=Accept(listenfd,(SA *)&clientaddr,&clientlen);
        Pthread_create(&tid,NULL,thread,connfdp);
    }
}

void *thread(void *vargp){
    int connfd=*((int *)vargp);
    Pthread_detach(pthread_self());
    Free(vargp);
    doit(connfd);
    Close(connfd);
    return NULL;
}

void doit(int fd){
    char buf[MAXLINE];
    char method[MAXLINE],url[MAXLINE],version[MAXLINE];
    int srv_fd;
    char host[MAXLINE],port[6],uri[MAXLINE];
    rio_t cli_req;
    Rio_readinitb(&cli_req,fd);
    if(!Rio_readlineb(&cli_req,buf,MAXLINE))
        return;
    sscanf(buf,"%s %s %s",method,url,version);
    if (strcasecmp(method,"GET")){
        clienterror(fd,method,"501","Not Implemented",
                "Proxy does not implement this method");
        return;
    }
    if(!parse_url(url,host,port,uri)){
        clienterror(fd,method,"400","Bad Request",
                "Check your request and try again");
        return;
    }
    srv_fd=open_clientfd(host,port);
    request_server(srv_fd,&cli_req,host,uri,method);
    send_client(fd,srv_fd);
}

void send_client(int fd,int srv_fd){
    size_t content_length;
    rio_t srv_req;
    char buf[MAXLINE];
    void *srcp;
    Rio_readinitb(&srv_req,srv_fd);
    use_serverhdrs(fd,&srv_req,&content_length);
    sprintf(buf,"Proxy-Connection: close\r\n\r\n");
    Rio_writen(fd,buf,strlen(buf));
    ssize_t read_size;
    while((read_size=Rio_readnb(&srv_req,buf,MAXLINE)))
        Rio_writen(fd,buf,read_size);
}

/*
 * use_serverhdrs - read headers from the server,
 *             and get ready to send it to client.
 */
void use_serverhdrs(int fd,rio_t *srv_req,
        size_t *content_length){
    char buf[MAXLINE];
    Rio_readlineb(srv_req,buf,MAXLINE);
    while(strcmp(buf,"\r\n")){
        if(strstr(buf,"Content-length"))
            sscanf(buf,"Content-length: %ld",content_length);
        Rio_writen(fd,buf,strlen(buf));
        Rio_readlineb(srv_req,buf,MAXLINE);
    }
}

void request_server(int fd,rio_t *cli_req,
        char *host,char *uri,char *method){
    char buf[MAXLINE];
    int host_hdr=1;
    sprintf(buf,"%s %s HTTP/1.0\r\n",method,uri);
    Rio_writen(fd,buf,strlen(buf));

    Rio_readlineb(cli_req,buf,MAXLINE);
    while(strcmp(buf,"\r\n")){
        if(!(strstr(buf,"Connection")||
                strstr(buf,"Proxy-Connection")||
                strstr(buf,"User-Agent")))
            Rio_writen(fd,buf,strlen(buf));
        if(host_hdr&&strstr(buf,"Host"))
            host_hdr=0;
        Rio_readlineb(cli_req,buf,MAXLINE);
    }
    if(host_hdr){
        sprintf(buf,"Host: %s\r\n",host);
        Rio_writen(fd,buf,strlen(buf));
    }
    Rio_writen(fd,user_agent_hdr,strlen(user_agent_hdr));
    sprintf(buf,"Proxy-Connection: close\r\n");
    Rio_writen(fd,buf,strlen(buf));
    sprintf(buf,"Connection: close\r\n\r\n");
    Rio_writen(fd,buf,strlen(buf));
}

/*
 * parse_url - parse URL into host and uri args
 *             return 0 if any error occurred , 1 by default
 */
int parse_url(char *url,char *host,
        char *port,char *uri){
    char *ptr;
    if(!(ptr=strchr(&url[7],'/')))
        return 0;
    size_t host_len;
    host_len=ptr-&url[7];
    strncpy(host,&url[7],host_len);
    host[host_len]='\0';
    strcpy(uri,ptr);
    if((ptr=strchr(host,':'))){
        strcpy(port,ptr+1);
        *ptr='\0';
    }
    else{
        strcpy(port,"80");
    }
    return 1;
}

/*
 * clienterror - returns an error message to the client
 */
void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg) 
{
    char buf[MAXLINE];

    /* Print the HTTP response headers */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n\r\n");
    Rio_writen(fd, buf, strlen(buf));

    /* Print the HTTP response body */
    sprintf(buf, "<html><title>Tiny Error</title>");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<body bgcolor=""ffffff"">\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "%s: %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<p>%s: %s\r\n", longmsg, cause);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<hr><em>The Tiny Web server</em>\r\n");
    Rio_writen(fd, buf, strlen(buf));
}
