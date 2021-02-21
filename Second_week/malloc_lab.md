

## 前言

首先这个代码是按书上的改写的 +_+

因为不能用结构体，和数组这些，所以在操作header footer 设置size这些操作的时候会显得很麻烦，所以可以使用 宏来解决这个问题

这个堆大致的结构是

```
heap_list + chunk
```

chunk的大致结构

|       header (size| flag)      |

|              data                       |

|            footer                      |



flag为1 的时候是alloc的状态，为0是free状态

## 设计思路

### mm_init

```c
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
```

这里初始化了一个堆的结构，建立了一个heap_list，可以通过heap_list遍历整个heap空间的chunk

然后就是

### mm_malloc

```c
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
```

这里就跟glibc的设计差不多了，这里有两种情况，一种是去free的找，另外一种直接从后面分配

`find_fix` 就是一个 遍历函数，用来找是否有满足的chunk，这个函数的实现就关乎free_chunk的重用效率了

find_fix 刚开始的是用first_fix写的，后来为了跑好点分数，就改成了 next_fix

但是next_fix的话要注意几种情况，就是在合并堆块的时候

先说下 next_fix 

```
它其实就差不多是一个循环查询，下次查询开始的位置是上一次的查询结束的位置
```

而 在合并chunk的时候，会出现 next_fix指向堆快被修改了，这样就会出现 next_fix查询的时候异常

然后就算 free

### mm_free

```c
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));
    PUT(HDRP(ptr),PACK(size,0));
    PUT(FTRP(ptr),PACK(size,0));
    coalesce(ptr);

}
```

这个就是直接设置 flag位，能合并就合并，没有任何检测机制

### mm_realloc

关于realloc我遇到了几点问题，其中一个比较关键的就是size没对齐，这就会导致在realloc的时候 size出现问题，至于什么问题。我没太深入去看

然后realloc也差不多和glibc的实现差不多

```c
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
```

当前的chunk的size 叫做 old_size 

上一个chunk的size 叫做 prev_size

下一个chunk的size 叫做 next_size

请求的size叫做 request_size

old_size > request_size ，就直接分割就OK

old_size = request_size 直接返回

old_size > request_size，这个就要分几种情况了

我先说下我现在知道的，也不知道有没漏

```
当前的chunk前面和后面有 free的chunk，如果它们的size加上当前的chunk的size，如果满足传入的size，合并

old_size + next_size > request_size
old_size + prev_size > request_size

然后要保证 chunk的内容不变

如果不满足就直接 malloc一个新的
```





