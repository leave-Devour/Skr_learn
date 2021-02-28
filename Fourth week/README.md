# 第三周总结

使用分离适配原则，重新实现malloc lab



设计思路:

chunk的结构

```
|   size      |     fd         |
|    bk       |      data      |
|  pre_size   |
```

对于size<0x140的 每0x10为一个bin组，类似于tacahe，但是这个我使用的是往后插入，没有在链表前面插入，这样的效率会比前面的差

然后size大于 0x140的都装到一个bin组，其实这里用 2的n次方来分组可能更好点。

在合并的时候，我采取的是直接合并，不管在不在同一个组，在的合并的完，再加入到相应的组

realloc不边，依旧是size满足就直接分配，不满足上下找合并，然后分配，如何还是没有就直接malloc新的

分数：

![image-20210228173442032](https://github.com/leave-Devour/Skr_learn/blob/main/Fourth%20week/images/image-20210228173442032.png)

