/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))
#define BLOCK_SIZE(block) ((*(size_t *)(block)) & (-1<<2))
#define FREE_BLOCK(block) !((*(size_t *)(block)) & 1)
#define EMPTY_BLOCK(block) !(BLOCK_SIZE(block))
#define PREVIOUS_USED(current) (((*(size_t *)(current)) & 0x2) >> 1)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
#define NEXT_BLOCK(current) (void *)((((char *)current)+BLOCK_SIZE(current)))
#define PREVIOUS_END(current) (((char *)(current)-SIZE_T_SIZE))

void *mm_list;
int has_free=0;

// HACK: TARGET AT THE TRACE FILE PROVIDED
static size_t align(size_t size){
	if(size>64){
		if(size<256){
			if(size>128) return((size)+255)&~255;
			return((size)+127)&~127;
		}
		else if(size<512) return((size)+511)&~511;
	}
	else{
		if(size>16){
			if(size>32) return ((size)+63)&~63;
			return ((size)+31)&~31;
		}
		else if(size>8) return ((size)+15)&~15;
	}
	return ALIGN(size);
}
static void write_header(void *head,size_t size,int allocate){
	int previous=PREVIOUS_USED(head)<<1;
	size_t tag=size|previous|allocate;
	*((size_t *)head)=tag;
}
static void write_footer(void *head,size_t size,int allocate){
	if(!allocate&&size!=SIZE_T_SIZE)
		*((size_t *)((char *)head+size-SIZE_T_SIZE))=size;
	size_t *footer=(size_t *)((char *)head+size);
	if(!allocate) *footer=(*footer)&((-1<<2)+1);
	else *footer=(*footer)|(allocate<<1);
}
static void *new_block(size_t size){
	void *p=mem_sbrk(size);
	if(p==(void *)-1)
		return NULL;
	else{
		return p;
	}
}
static void* split_free(void *original_block,size_t size){
	size_t new_size=BLOCK_SIZE(original_block)-size;
	if(new_size==0) return NULL;
	void *new_block=(char *)original_block+size;
	write_header(new_block,new_size,0);
	write_footer(new_block,new_size,0);

	return original_block;
}
static void* extend_block(void *block,size_t size){
	size_t old_size=BLOCK_SIZE(block);
	if(EMPTY_BLOCK(NEXT_BLOCK(block))){
		if(size>old_size){
			new_block(size-old_size);
			write_header(block,size,1);
			*(size_t *)((char *)block+size)=0;
			write_footer(block,size,1);
			return block;
		}
	}
	return NULL;
}
static void* find_free(size_t size){
	if(!has_free) return NULL;
	void *block=mm_list;
	void *ptr=NULL;
	size_t min_size=0;

	while(!EMPTY_BLOCK(block)){
		if(FREE_BLOCK(block)){
			if(BLOCK_SIZE(block)==size)
				return block;
			// best fit
			else if(BLOCK_SIZE(block)>=size+SIZE_T_SIZE){
				if(min_size>BLOCK_SIZE(block)||!min_size){
					min_size=BLOCK_SIZE(block);
					ptr=block;
				}
			}
		}
		block=NEXT_BLOCK(block);
	}
	if(ptr) return split_free(ptr,size);
	if(EMPTY_BLOCK(block)){
		has_free=0;
		if(!PREVIOUS_USED(block)){
			void *new_block=(char *)block-*((size_t *)PREVIOUS_END(block));
			new_block=extend_block(new_block,size);
			if(new_block) return new_block;
		}
	}
	return NULL;
}
static void* coalescing_previous(void *current,size_t *size){
	void *previous=PREVIOUS_END(current);
	while(!PREVIOUS_USED(current)&&!EMPTY_BLOCK(previous)){
		current=(char *)current-BLOCK_SIZE(previous);
		*size+=BLOCK_SIZE(previous);
		previous=PREVIOUS_END(current);
	}
	return current;
}
static void* coalescing_next(void *current,size_t *size){
	void *next=(char *)current+*size;
	while(!EMPTY_BLOCK(next)&&FREE_BLOCK(next)){
		*size+=BLOCK_SIZE(next);
		next=(char *)current+*size;
	}
	return current;
}
static void* coalescing(void *current,size_t* size){
	coalescing_next(current,size);
	current=coalescing_previous(current,size);
	return current;
}
static void* split_ptr(void *ptr,size_t size){
	void *block=(char *)ptr-SIZE_T_SIZE;
	size=align(size)+SIZE_T_SIZE;
	if(size>BLOCK_SIZE(block)) return NULL;
	void *free_block=(char *)block+size;
	size_t free_size=BLOCK_SIZE(block)-size;
	write_header(block,size,1);
	if(free_size!=0){
		*(size_t *)free_block=0;
		write_footer(block,size,1);
		write_header(free_block,free_size,0);
		write_footer(free_block,free_size,0);
	}
	else write_footer(block,size,1);
	return ptr;
}
static void* extend_ptr(void *ptr,size_t size){
	void *block=(char *)ptr-SIZE_T_SIZE;
	size_t old_size=BLOCK_SIZE(block);
	size=align(size)+SIZE_T_SIZE;
	if(size==old_size) return ptr;
	/*else if(size>old_size){
		size_t new_size=old_size;
		void *new_block=coalescing_previous(block,&new_size);
		size_t free_size=new_size-size;
		if(new_size>size||free_size==0){
			void *newptr=(char *)new_block+SIZE_T_SIZE;
			memmove(newptr,ptr,old_size-SIZE_T_SIZE);
			void *free_block=(char *)new_block+size;
			write_header(new_block,size,1);
			if(free_size==0)
				write_footer(new_block,size,1);
			else{
				*(size_t *)free_block=0;
				write_footer(new_block,size,1);
				write_header(free_block,free_size,0);
				write_footer(free_block,free_size,0);
			}
			return newptr;
		}
	}*/
	void *new_block=extend_block(block,size);
	if(new_block) return ((char *)new_block+SIZE_T_SIZE);
	return NULL;
}
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
	mm_list=new_block(SIZE_T_SIZE*2);
	if(!mm_list){
		return -1;
	}
	else{
		*(size_t *)mm_list=0;
		mm_list=(char *)mm_list+SIZE_T_SIZE;
		*(size_t *)mm_list=0;
		return 0;
	}
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
	size = align(size)+SIZE_T_SIZE;
	void *p=find_free(size);
	if(!p){
		p=new_block(size);
		if(!p) return NULL;
		p=(char *)p-SIZE_T_SIZE;
		//initialize the last block
		*(size_t *)((char *)p+size)=0;
	}
	write_header(p,size,1);
	write_footer(p,size,1);
	return (void *)((char *)p+SIZE_T_SIZE);
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
	has_free=1;
	void *current=((char *)ptr-SIZE_T_SIZE);
	size_t size=BLOCK_SIZE(current);
	current=coalescing(current,&size);
	write_header(current,size,0);
	write_footer(current,size,0);
	//mm_check();
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
	void *oldptr = ptr;
	void *newptr;
	size_t copySize = BLOCK_SIZE((char *)oldptr-SIZE_T_SIZE)-SIZE_T_SIZE;
	newptr = split_ptr(oldptr,size);
	if(newptr)
		return newptr;
	newptr = extend_ptr(oldptr,size);
	if(newptr)
		return newptr;
	newptr = mm_malloc(size);
	if (newptr == NULL)
		return NULL;
	memcpy(newptr, oldptr, copySize);
	mm_free(oldptr);
	return newptr;
}

/*
 * mm_check - Used to check if heap is consistent
 *
 */
int mm_check(void){
	void *p=mm_list;
	void *last=(char *)(mem_heap_hi()+1-SIZE_T_SIZE);
	void *previous=NULL;
	while(p!=last){
		if(previous&&PREVIOUS_USED(p)!=((*(size_t *)previous)&0x1)){
			printf("block before %x not in freelist: current %d,previous %d\n",p,PREVIOUS_USED(p),(*(size_t *)previous)&0x1);
		}
		if(EMPTY_BLOCK(p)){
			printf("freelist ends unexpectedly\n");
			return -1;
		}
		previous=p;
		p=NEXT_BLOCK(p);
	}
	return 0;
}
