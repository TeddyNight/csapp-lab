#include "cachelab.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>

bool verbose=false;

/*
 *  at most 32bits set index & 32bits mark 
 *
 */
struct cache_block{
	unsigned long long tag;
       	unsigned long long index;
	struct cache_block *prev;
	unsigned visit;
	bool vaild;
};

int logger(char op,unsigned long long add,int size,char *msg){
	if(verbose) printf("%c %llx,%d %s\n",op,add,size,msg);
	return 0;
}

struct cache_block* find_one(struct cache_block* s,unsigned long long index,unsigned long long tag){
	while(s){
		if(s->index==index) break;
		s=s->prev;
	}
	struct cache_block *t=s;
	bool find=false;
	// find one matched
	while(!find){
		// find vaild one
		if(t->tag==tag&&t->vaild) find=true;
		if(!(t->prev)||t->prev->index!=index) break;
		else if(!find) t=t->prev;
	}
	if(!find) t=s;
	// find available space
	while(!find){
		if(!(t->vaild)) find=true;
		if(!(t->prev)||t->prev->index!=index) break;
		else if(!find) t=t->prev;
	}
	if(!find) t=s;
	unsigned visit=s->visit;
	// find the least frequently used one
	while(!find&&s){
		//equal?
		if(s->visit>=visit){
			// forget to update the max visit cause errors, inspired by the answers on the Internet
			visit=s->visit;
			t=s;
		}
		if(!(s->prev)||s->prev->index!=index) break;
		s=s->prev;
	}
	// reset visit counter
	t->visit=0;
	return t;
}

int record_time(struct cache_block *t){
	while(t!=NULL){
		t->visit++;
		t=t->prev;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	int hit=0,miss=0,evi=0;
	int ch,num_e,num_b,num_s;
	FILE *trace_file;
	if(argc==1){
		fprintf(stderr,"Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n",
				argv[0]);
		exit(EXIT_FAILURE);
	}
	//getopt() itself can deal with some problems with arguments
	while((ch=getopt(argc,argv,"hvs:E:b:t:"))!=-1){
		switch(ch){
			case 'v':
				verbose=true;
				break;
			case 's':
				num_s=atoi(optarg);
				break;
			case 'E':
				num_e=atoi(optarg);
				break;
			case 'b':
				num_b=atoi(optarg);
				break;
			case 't':
				if((trace_file=fopen(optarg,"r"))==NULL){
					fprintf(stderr,"Error occured when opening the file\n");
					exit(EXIT_FAILURE);
				}
				break;
			default: /* '?' */
				fprintf(stderr,"Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n",
						argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	struct cache_block *cache=NULL;
	// 2^x is 2 xor x !!
	for(int i=(1<<num_s)-1;i>=0;i--){
		for(int j=1;j<=num_e;j++){
			struct cache_block *t;
			t=malloc(sizeof(struct cache_block));
			t->prev=cache;
			t->vaild=false;
			t->tag=0;
			t->index=i;
			t->visit=0;
			cache=t;
		}
	}

	char op;
	unsigned long long d_add;
	int d_size,r_ret;
	int offset_t=num_s+num_b;
	unsigned a_index=(1<<num_s)-1;

	while((r_ret=fscanf(trace_file," %c %llx,%d",&op,&d_add,&d_size))!=EOF){
		if(r_ret!=3){
			fprintf(stderr,"Error occured when reading the file\n");
			exit(EXIT_FAILURE);
		}
		if(op=='I') continue;
		unsigned long long d_tag=d_add>>offset_t;
		// 2^4 is 2 xor 4 !!
		unsigned long long d_index=(d_add>>num_b)&a_index;
		struct cache_block *t=find_one(cache,d_index,d_tag);
		//record time
		record_time(cache);
		// When the checker was put in the while loop above, it would not check the first one in the group
		if(t->tag==d_tag&&t->vaild){
			hit++;
			//M: data store followed by a data load
			if(op=='M'){
				hit++;
				logger(op,d_add,d_size,"hit hit");
			}
			else logger(op,d_add,d_size,"hit");
		}
		else if(t->tag!=d_tag&&t->vaild){
			miss++;
			evi++;
			t->tag=d_tag;
			//M: a data store followed by a data load
			if(op=='M'){
				hit++;
				logger(op,d_add,d_size,"miss eviction hit");
			}
			else logger(op,d_add,d_size,"miss eviction");
		}	
		else if(!t->vaild){
			t->tag=d_tag;
			t->vaild=true;
			miss++;
			//M: a data store followed by a data load
			if(op=='M'){
				hit++;
				logger(op,d_add,d_size,"miss hit");
			}
			else logger(op,d_add,d_size,"miss");
		}
	}

	while(cache!=NULL){
		struct cache_block *t;
		t=cache->prev;
		free(cache);
		cache=t;
	}
	fclose(trace_file);
	printSummary(hit, miss, evi);
	return 0;
}
