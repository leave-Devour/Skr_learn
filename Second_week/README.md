# 第二周总结

- 复现 CVE-2021-3156第三种的收尾，第三种的最后的竞争想了好久，网上那个竞争其实得要修改过passwd文件，因为在sudo里面会判断打开文件的时间戳，如果大于系统启动的时间就会删除掉，但是这样就做不到exp通用了，然后发现有一种竞争就能绕过这种写法，具体的分析我在完成的时候，放到了第一周的目录里面
- 完成 DataLab（各种位运算脑壳疼
- 完成boom_lab

# 第三周总结

- sudo 第三种的exp，本地测过可行（但那个time要自己调整
- malloc_lab

分数如下:

![img](https://github.com/leave-Devour/Skr_learn/blob/main/Second_week/Snipaste_2021-02-20_23-48-10.png)