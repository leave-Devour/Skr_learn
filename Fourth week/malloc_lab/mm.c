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
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define WSIZE 4
#define DSIZE 8

#define CHUNKSIZE (1<<6)

#define MAX(x,y) ((x) > (y) ? (x) : (y))

#define PACK(size, alloc) ((size) | (alloc))

#define GET(p) (*(unsigned int *)(p))
#define PUT(p,val) (*(unsigned int *)(p)=(val))

#define GET_SIZE(p) (GET(p) & ~0x7)

#define GET_ALLOC(p) (GET(p) & 0x1)

#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) +GET_SIZE(HDRP(bp)) - DSIZE)
#define FD(bp) ((char *)(bp))
#define BK(bp) ((char *)(bp) + WSIZE)
#define NEXT_BLKP(bp) ((char *) (bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *) (bp) - GET_SIZE(((char *)(bp) - DSIZE)))


static char *heap_listp;
static char *heap_bin;
static void *find_fit(size_t asize);
static void place(void * bp, size_t asize,unsigned flag);
static void * coalesce(void *bp);
static void *extend_heap(size_t words);
void hexdump(const char *desc, void *addr, int len);
void inset_Node(void *a,void *b);
void insert(void *elem,void *queue);
void * pop_HeapBin(size_t size,void *queue);
void *select_heap(int size);
void push_HeapBin(void *ptr);

void del_ele(void *bp,size_t size);
void eeee(size_t size);
void test(size_t size,size_t size1){
    if(size==size1)getchar();
}
static char *pre_listp;
/*这个是合并函数*/
static void * coalesce(void *bp){

    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp))); 
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
	//printf("flag1 %d flag2 %d\n",prev_alloc,next_alloc);
    if(prev_alloc && next_alloc) //
        {

        return bp;
        }  
    else if (!prev_alloc && next_alloc){
        del_ele(PREV_BLKP(bp), GET_SIZE(HDRP(PREV_BLKP(bp))));
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp),PACK(size,0));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        bp = PREV_BLKP(bp);

    }
    else if(prev_alloc && !next_alloc){
        del_ele(NEXT_BLKP(bp), GET_SIZE(HDRP(NEXT_BLKP(bp))));
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp),PACK(size,0));
        PUT(FTRP(bp),PACK(size,0));
    }
    else{
        del_ele(NEXT_BLKP(bp), GET_SIZE(HDRP(NEXT_BLKP(bp))));
        del_ele(PREV_BLKP(bp), GET_SIZE(HDRP(PREV_BLKP(bp))));
        size += GET_SIZE(HDRP(PREV_BLKP(bp)))+GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        PUT(FTRP(NEXT_BLKP(bp)),PACK(size,0));
        bp = PREV_BLKP(bp);
    }
    //printf("size %d\n",GET_SIZE(HDRP((bp))));
    return bp;

}


static void *extend_heap(size_t words){
    char *bp;
    size_t size;
    void * tmp;
    size = (words %2 ) ? (words + 1)*WSIZE : words * WSIZE; /* 对齐 */
    if((long )( bp = mem_sbrk(size)) == -1)
        return NULL;
    PUT(HDRP(bp), PACK(size,0));
    PUT(FTRP(bp), PACK(size,0));
    PUT(HDRP(NEXT_BLKP(bp)),PACK(0,1));
    tmp = coalesce(bp);
    return tmp;
}



/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)

{
    if((heap_listp = mem_sbrk(0x18*WSIZE)) == (void *)-1)
    {
	    return -1;
    }
     for(int i=0;i<0x15;i++){
            PUT(heap_listp+(i*WSIZE),0);
        }
     	PUT(heap_listp+(0x15*WSIZE),PACK(DSIZE,1));
        PUT(heap_listp+(0x16*WSIZE),PACK(DSIZE,1));
        PUT(heap_listp+(0x17*WSIZE),PACK(0,1));
        heap_bin = heap_listp;
        heap_listp += (0x16*WSIZE);
	//printf("addr %p\n",heap_listp);
        if(extend_heap(CHUNKSIZE/WSIZE)==NULL)
            return -1;
    return 0;
}


/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */

void *mm_malloc(size_t size)
{
    size_t asize;
    size_t extendsize;
    char * bp;
    
    if(size == 0) return NULL;

     asize = DSIZE  + ALIGN(size);
    //hexdump("yezi",heap_listp-(2*WSIZE),0x1000);
    //printf("\naaaa\n");
      if((bp = find_fit(asize)) != NULL){
        place(bp, asize,1);
        return bp;
    }

    //printf("bp %p\n",bp);
    extendsize = MAX(asize, CHUNKSIZE);
    if((bp = extend_heap(extendsize/WSIZE)) == NULL) return NULL;
    place(bp, asize,1);
    return bp;
}

void eeee(size_t size){

}

static void *find_fit(size_t asize)
{
    void *tmp = select_heap(asize);
    //hexdump("yezi",tmp,0x10);
    return pop_HeapBin(asize,tmp);
}
void del_ele(void *bp,size_t size){
    char *tmp = select_heap(size);
    if(GET(tmp)!=0 ){ 
        if(GET(BK(bp))==0){
            if(GET(FD(bp))==tmp){
                PUT(tmp,0);
            }
            else{
                PUT(BK(GET(FD(bp))),0);
            }
        }
        else{
            if(GET(FD(bp))==tmp){
                PUT(FD(GET(BK(bp))),tmp);
                PUT(tmp,GET(BK(bp)));
            }
            else{
                PUT(BK(GET(FD(bp))),GET(BK(bp)));
                PUT(FD(GET(BK(bp))),GET(FD(bp)));
            }
        }
    }

}
static void place(void * bp, size_t asize,unsigned flag){
    size_t size = GET_SIZE((char*)(bp) - WSIZE);
    size_t sub = size - asize;
    char *tmp;
    tmp = select_heap(size);
    if(GET(tmp)!=0 && flag ==0){ 
        if(GET(BK(bp))==0){
            if(GET(FD(bp))==tmp){
                PUT(tmp,0);
            }
            else{
                PUT(BK(GET(FD(bp))),0);
            }
        }
        else{
            if(GET(FD(bp))==tmp){
                PUT(FD(GET(BK(bp))),tmp);
                PUT(tmp,GET(BK(bp)));
            }
            else{
                PUT(BK(GET(FD(bp))),GET(BK(bp)));
                PUT(FD(GET(BK(bp))),GET(FD(bp)));
            }
        }
    }
    if(sub >= 4*WSIZE){

        PUT(HDRP(bp), PACK(asize,1));
        PUT(FTRP(bp), PACK(asize,1));
        PUT(HDRP(NEXT_BLKP(bp)),PACK(sub,0));
        PUT(FTRP(NEXT_BLKP(bp)),PACK(sub,0));
        mm_free(NEXT_BLKP(bp));
    }
    else{
        PUT(HDRP(bp), PACK(GET_SIZE(((char *)(bp) - WSIZE)),1));
        PUT(FTRP(bp), PACK(GET_SIZE(((char *)(bp) - WSIZE)),1));
    }
}

void inset_Node(void *a,void *b){ // insert a in b 
    PUT(BK(b),a);
    PUT(FD(a),b);
    PUT(BK(a),0);
}
void insert(void *elem,void *queue){
    void *e;
    if(GET(queue)==0){
        PUT(queue,elem);
        PUT(FD(elem),queue);
        PUT(BK(elem),0);
    }
    /*
    else{
	e = GET(queue);
        for(;GET(BK(e))!=0;e=GET(BK(e)));
        inset_Node(elem,e);
    }
    */
    else{
        e = GET(queue);
        PUT(queue,elem);
        PUT(FD(elem),queue);
        PUT(BK(elem),e);
        PUT(FD(e),elem);
    }
}   


void * pop_HeapBin(size_t size,void *queue){
    void *e;
    //printf("pop\n");
	if(GET(queue)==0)return NULL;
    else{
        for(e=GET(queue);GET(BK(e))!=0;e=GET(BK(e))){
            if(GET(BK(e))==0){
            if(GET_SIZE(HDRP(e)) < size)
            {
                return NULL;
            }
            else{

                PUT(BK(GET(FD(e))),0);
                return (e);
            }
            }
            if(GET_SIZE(HDRP(e)) >= size){
                if(queue==GET(FD(e))){
                    PUT(queue,GET(BK(e)));
                    PUT(FD(GET(BK(e))),GET(FD(e)));
                    return e;
                }
                    PUT(BK(GET(FD(e))),GET(BK(e)));
                    PUT(FD(GET(BK(e))),GET(FD(e)));
                    return e;
            }
        }
        e = GET(queue);
        if(GET_SIZE(HDRP(e)) >= size){
            PUT(queue,0);
            return e;
        }


    }
    return NULL;
}


void *select_heap(int size){
	//printf("asize %d\n",size);
    if(0 <  size && size < 0x140){
        int size1 = size >> 4;
        return heap_bin+(size1*WSIZE);
    }else{
        return heap_bin+(0x14*WSIZE);
    }
    
}

void push_HeapBin(void *ptr){
    size_t size = GET_SIZE(HDRP(ptr));
    void * tmp = select_heap(size);
    insert(ptr,select_heap(size));
    //hexdump("yezi",select_heap(size),0x10);

}
/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{   //printf("\n free \n");
    //
//test(0xf69c7078,ptr);
    size_t size = GET_SIZE(HDRP(ptr));
    PUT(HDRP(ptr),PACK(size,0));
    PUT(FTRP(ptr),PACK(size,0));
    ptr=coalesce(ptr);

    //printf("aaa\n");
    push_HeapBin(ptr);


}




/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    char *oldptr = ptr;
    size_t old_size=0;
    size_t p = GET(HDRP(oldptr));
    old_size = GET_SIZE(HDRP(oldptr));
        if (size ==0) mm_free(ptr);
    else 
        size  = DSIZE  + ALIGN(size);
    char *nextblk=NEXT_BLKP(ptr);
    char *prevblk=PREV_BLKP(ptr);

    if (old_size < size){
        
        if(!GET_ALLOC(HDRP(nextblk)) && ((GET_SIZE(HDRP(nextblk))+old_size )>=size)){
            del_ele(nextblk,GET_SIZE(HDRP(nextblk)));
            PUT(HDRP(ptr),PACK(old_size+GET_SIZE(HDRP(nextblk)),1));
            PUT(FTRP(ptr),PACK(old_size+GET_SIZE(HDRP(nextblk)),1));
            place(ptr,size,1);
            return ptr;
        }
        
        else if(!GET_ALLOC(HDRP(prevblk)) && ((GET_SIZE(HDRP(prevblk))+old_size)>=size)){
            del_ele(prevblk,GET_SIZE(HDRP(prevblk)));
            PUT(HDRP(prevblk),PACK(old_size+GET_SIZE(HDRP(prevblk)),1));
            PUT(FTRP(prevblk),PACK(old_size+GET_SIZE(HDRP(prevblk)),1));        
            memmove(prevblk, oldptr, old_size-DSIZE);
            place(prevblk,size,1);
            return prevblk;
            }
            
        else{

        char *newptr=mm_malloc(size);
        memcpy(newptr, oldptr, old_size-DSIZE);
        mm_free(oldptr);
            return newptr;
        }

    }

   if(old_size >= size)
    place(ptr,size,1);
    return ptr;
}




void   hexdump(const char *desc, void *addr, int len) {
    int i;
    char code;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char *)addr;
    if (desc != NULL)
        printf("%s:\n", desc);
    if (len == 0) {
        printf("  ZERO LENGTH\n");
        return;
    }
    if (len < 0) {
        printf("  NEGATIVE LENGTH: %i\n", len);
        return;
    }

    for (i = 0; i < len; i++) {
        if ((i % 16) == 0) {
            if (i != 0)
                printf("  %s\n", buff);

        }

        if((i)%0x100000==0&&i!=0){
        printf("[*] do you want to continue\n");
        code = getchar();
        if(code=='q'||code=='Q'){
            getchar();
            printf("[*] exit\n");
            return;
        }
        }
        if(i%16==0){
            printf("0x%04x 0x", i);
        }
        if(i%8==0&&i%16!=0)printf("  0x");
        printf("%02x", pc[i]);
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }
    while ((i % 16) != 0) {
        printf("   ");
        i++;
    }
    printf("  %s\n", buff);
}