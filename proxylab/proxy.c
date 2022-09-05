#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

#define WORKERS 8

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

typedef struct CACHE_T{
    char data[MAX_OBJECT_SIZE];
    char url[MAXLINE];
    size_t header_length;
    size_t content_length;
    struct CACHE_T *next;
    size_t visit;
} cache_t;

int queue[WORKERS];
int front=0;
int gear=-1;
sem_t items;
sem_t slots;

cache_t *cache;
size_t cache_size;
sem_t cache_sem;
sem_t mutex;
int reader_cnt=0;

void *thread(void *vargp);
cache_t *doit(int fd);
int parse_url(char *url,char *host,
        char *port,char *uri);
void clienterror(int fd,char *cause,char *errnum,
        char *shortmsg,char *longmsg);
void request_server(int fd,rio_t *cli_req,
        char *host,char *uri,char *method);
void forward_client(int fd,int srv_fd,cache_t *obj);
void *forward_header(int fd,rio_t *srv_req,
        cache_t *obj);

cache_t *find_cache(char *url);
void update_cache(cache_t *obj);
void evicit_cacheobj(cache_t *obj);
void insert_cacheobj(cache_t *previous_obj,cache_t *obj);

void enqueue(int fd);
int dequeue();

int main(int argc, char **argv)
{
    int connfd;
    int listenfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;

    if(argc!=2){
        fprintf(stderr,"usage: %s <port>\n",argv[0]);
        exit(1);
    }

    cache=Malloc(sizeof(cache_t));
    memset(cache,0,sizeof(cache_t));
    Sem_init(&cache_sem,0,1);
    Sem_init(&mutex,0,1);

    Sem_init(&slots,0,WORKERS);
    Sem_init(&items,0,0);

    for(int i=0;i<WORKERS;i++)
        Pthread_create(&tid,NULL,thread,NULL);
    listenfd=Open_listenfd(argv[1]);
    while(1){
        clientlen=sizeof(clientaddr);
        connfd=Accept(listenfd,(SA *)&clientaddr,&clientlen);
        enqueue(connfd);
    }
}

void *thread(void *vargp){
    cache_t *obj;
    int connfd;

    Pthread_detach(pthread_self());
    Free(vargp);

    while(connfd=dequeue()){
        obj=doit(connfd);
        Close(connfd);

        if(obj){
            if((obj->header_length+obj->content_length)
                    <=MAX_OBJECT_SIZE)
                update_cache(obj);
            else
                Free(obj);
        }
    }
    return NULL;
}

cache_t *doit(int fd){
    char buf[MAXLINE];
    char method[MAXLINE],url[MAXLINE],version[MAXLINE];
    int srv_fd;
    char host[MAXLINE],port[6],uri[MAXLINE];
    cache_t *obj;
    rio_t cli_req;

    Rio_readinitb(&cli_req,fd);
    if(!Rio_readlineb(&cli_req,buf,MAXLINE))
        return NULL;

    sscanf(buf,"%s %s %s",method,url,version);
    if (strcasecmp(method,"GET")){
        clienterror(fd,method,"501","Not Implemented",
                "Proxy does not implement this method");
        return NULL;
    }
    if(!parse_url(url,host,port,uri)){
        clienterror(fd,method,"400","Bad Request",
                "Check your request and try again");
        return NULL;
    }

    P(&mutex);
    reader_cnt++;
    if(reader_cnt==1)
        P(&cache_sem);
    V(&mutex);

    obj=find_cache(url);
    if(obj)
        Rio_writen(fd,obj->data,
                obj->header_length+obj->content_length);

    P(&mutex);
    reader_cnt--;
    if(obj)
        obj->visit++;
    if(reader_cnt==0)
        V(&cache_sem);
    V(&mutex);

    if(obj)
        return NULL;

    obj=Malloc(sizeof(cache_t));
    strcpy(obj->url,url);
    obj->visit=0;

    if((srv_fd=Open_clientfd(host,port))>0){
        request_server(srv_fd,&cli_req,host,uri,method);
        forward_client(fd,srv_fd,obj);
    }
    else{
        clienterror(fd,method,"502","Bad Gateway",
                strerror(errno));
        return NULL;
    }
    return obj;
}

void enqueue(int fd){
    P(&slots);
    gear=(gear+1)%WORKERS;
    queue[gear]=fd;
    V(&items);
}

int dequeue(){
    int item;
    P(&items);
    item=queue[front];
    front=(front+1)%WORKERS;
    V(&slots);
    return item;
}
    
cache_t *find_cache(char *url){
    cache_t *obj=cache;
    while(obj){
        if(obj->content_length&&
                !strcmp(obj->url,url))
            return obj;
        obj=obj->next;
    }
    return NULL;
}

void update_cache(cache_t *obj){
    size_t new_size=cache_size+obj->content_length;
    if(new_size>MAX_CACHE_SIZE){
        evicit_cacheobj(obj);
        return;
    }
    P(&cache_sem);
    insert_cacheobj(cache,obj);
    cache_size=new_size;
    V(&cache_sem);
}

void insert_cacheobj(cache_t *previous_obj,cache_t *obj){
    cache_t *next_obj=previous_obj->next;
    if(next_obj)
        obj->next=next_obj;
    previous_obj->next=obj;
}

void evicit_cacheobj(cache_t *obj){
    cache_t *previous_obj=cache;
    cache_t *cur_obj=cache;
    cache_t *evicit_obj;
    P(&cache_sem);
    while(cur_obj->next){
        if(cur_obj->next->visit
                <=previous_obj->next->visit)
            previous_obj=cur_obj;
        cur_obj=cur_obj->next;
    }
    evicit_obj=previous_obj->next;
    if(cache_size-evicit_obj->content_length
            +obj->content_length<=MAX_CACHE_SIZE){
        insert_cacheobj(previous_obj,obj);
        Free(evicit_obj);
    }
    V(&cache_sem);
}

void forward_client(int fd,int srv_fd,cache_t *obj){
    rio_t srv_req;
    char *buf;
    ssize_t read_size;
    size_t obj_size;
    Rio_readinitb(&srv_req,srv_fd);
    buf=forward_header(fd,&srv_req,obj);
    obj->header_length=buf-obj->data;
    obj_size=obj->header_length+obj->content_length;
    while((read_size=
                Rio_readnb(&srv_req,buf,MAXLINE))){
        Rio_writen(fd,buf,read_size);
        if(obj_size<=MAX_OBJECT_SIZE) buf+=read_size;
    }
}

/*
 * forward_header - read headers from the server,
 *             and get ready to send it to client.
 */
void *forward_header(int fd,rio_t *srv_req,
        cache_t *obj){
    size_t size;
    char *buf=obj->data;
    size=Rio_readlineb(srv_req,buf,MAXLINE);
    while(strcmp(buf,"\r\n")){
        if(strstr(buf,"Content-length"))
            sscanf(buf,"Content-length: %ld",&obj->content_length);
        Rio_writen(fd,buf,strlen(buf));
        buf+=size;
        size=Rio_readlineb(srv_req,buf,MAXLINE);
    }
    sprintf(buf,"Proxy-Connection: close\r\n\r\n");
    Rio_writen(fd,buf,strlen(buf));
    buf+=strlen(buf);
    return buf;
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
    sprintf(buf, "<html><title>Proxy Error</title>");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<body bgcolor=""ffffff"">\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "%s: %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<p>%s: %s\r\n", longmsg, cause);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<hr><em>The Proxy server</em>\r\n");
    Rio_writen(fd, buf, strlen(buf));
}
