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

#define CHUNKSIZE (1<<12)

#define MAX(x,y) ((x) > (y) ? (x) : (y))

#define PACK(size, alloc) ((size) | (alloc))

#define GET(p) (*(unsigned int *)(p))
#define PUT(p,val) (*(unsigned int *)(p)=(val))

#define GET_SIZE(p) (GET(p) & ~0x7)

#define GET_ALLOC(p) (GET(p) & 0x1)

#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) +GET_SIZE(HDRP(bp)) - DSIZE)

#define NEXT_BLKP(bp) ((char *) (bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *) (bp) - GET_SIZE(((char *)(bp) - DSIZE)))

static char *heap_listp;
static void *find_fit(size_t asize);
static void place(void * bp, size_t asize);
static void * coalesce(void *bp);
static void *extend_heap(size_t words);
void hexdump(const char *desc, void *addr, int len);

static char *pre_listp;
/*这个是合并函数*/
static void * coalesce(void *bp){

    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp))); // 获取上一个 chunk的 alloc标志位
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp))); // 获取下一个 chunk的 alloc标志位
    size_t size = GET_SIZE(HDRP(bp));//获取当前的size

    if(prev_alloc && next_alloc) // 前后都是 alloc的不合并，直接返回
        {
        pre_listp = bp;
        return bp;
        }
        
    else if (prev_alloc && !next_alloc){ // 向后合并
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp),PACK(size,0));
        PUT(FTRP(bp),PACK(size,0));
    }
    else if(!prev_alloc && next_alloc){// 向前合并
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp),PACK(size,0));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        bp = PREV_BLKP(bp);
    }
    else{ // 前后都有

        size += GET_SIZE(HDRP(PREV_BLKP(bp)))+GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        PUT(FTRP(NEXT_BLKP(bp)),PACK(size,0));
        bp = PREV_BLKP(bp);
    }
    pre_listp = bp;
    return bp;

}


static void *extend_heap(size_t words){ // 这个就是扩展heap，不够就去拿
    char *bp;
    size_t size;
    size = (words %2 ) ? (words + 1)*WSIZE : words * WSIZE; /* 对齐 */
    if((long )( bp = mem_sbrk(size)) == -1)
        return NULL;
    PUT(HDRP(bp), PACK(size,0));
    PUT(FTRP(bp), PACK(size,0));
    PUT(HDRP(NEXT_BLKP(bp)),PACK(0,1));

    return coalesce(bp);
}



/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)

{
    if((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
        return -1;
        PUT(heap_listp,0);
        PUT(heap_listp+(1*WSIZE),PACK(DSIZE,1));
        PUT(heap_listp+(2*WSIZE),PACK(DSIZE,1));
        PUT(heap_listp+(3*WSIZE),PACK(0,1));
        heap_listp += (2*WSIZE);
	pre_listp = heap_listp;
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

    if (size <= DSIZE) asize = 2 *DSIZE;
    else 
        asize = DSIZE *((size + (DSIZE) + (DSIZE -1)) / DSIZE);
    //hexdump("yezi",heap_listp-(2*WSIZE),0x1000);
      if((bp = find_fit(asize)) != NULL){
        place(bp, asize);
        return bp;
    } 
    
    extendsize = MAX(asize, CHUNKSIZE);
    if((bp = extend_heap(extendsize/WSIZE)) == NULL) return NULL;
    place(bp, asize);
    return bp;
}

static void *find_fit(size_t asize)
{
    char *bp = pre_listp;
    size_t alloc;
    size_t size;
    while (GET_SIZE(HDRP(NEXT_BLKP(bp))) > 0) {
        bp = NEXT_BLKP(bp);
        alloc = GET_ALLOC(HDRP(bp));
        if (alloc) continue;
        size = GET_SIZE(HDRP(bp));
        if (size < asize) continue;
        return bp;
    } 
    bp = heap_listp;
    while (bp != pre_listp) {
        bp = NEXT_BLKP(bp);
        alloc = GET_ALLOC(HDRP(bp));
        if (alloc) continue;
        size = GET_SIZE(HDRP(bp));
        if (size < asize) continue;
        return bp;
    } 
    return NULL;
}

static void place(void * bp, size_t asize){
    size_t size = GET_SIZE((char*)(bp) - WSIZE);
    size_t sub = size - asize;    
    if(sub >= 2*WSIZE){
        PUT(HDRP(bp), PACK(asize,1));
        PUT(FTRP(bp), PACK(asize,1));
        PUT(HDRP(NEXT_BLKP(bp)),PACK(sub,0));
        PUT(FTRP(NEXT_BLKP(bp)),PACK(sub,0));
    }
    else{
        PUT(HDRP(bp), PACK(GET_SIZE(((char *)(bp) - WSIZE)),1));
        PUT(FTRP(bp), PACK(GET_SIZE(((char *)(bp) - WSIZE)),1));
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));
    PUT(HDRP(ptr),PACK(size,0));
    PUT(FTRP(ptr),PACK(size,0));
    coalesce(ptr);

}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    char *oldptr = ptr;
    size_t old_size;
    old_size = GET_SIZE(HDRP(oldptr));
        if (size <= DSIZE) size = 2 *DSIZE;
    else 
        size = DSIZE *((size + (DSIZE) + (DSIZE -1)) / DSIZE);
    char *nextblk=NEXT_BLKP(ptr);
    char *prevblk=PREV_BLKP(ptr);

    if (old_size <= size){
        if(!GET_ALLOC(HDRP(nextblk)) && ((GET_SIZE(HDRP(nextblk))+old_size )>=size)){
            PUT(HDRP(ptr),PACK(old_size+GET_SIZE(HDRP(nextblk)),1));
            PUT(FTRP(ptr),PACK(old_size+GET_SIZE(HDRP(nextblk)),1));
            place(ptr,size);
            pre_listp = ptr;
            return ptr;
        }
        else{      
        void *newptr=mm_malloc(size);
        memcpy(newptr, oldptr, old_size-DSIZE);
        mm_free(oldptr);
            return newptr;
        }

    }
    place(ptr,size);
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

		if((i)%0x100==0&&i!=0){
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








