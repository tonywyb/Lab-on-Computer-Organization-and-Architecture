# Lab 3.2 Cache管理策略优化

### 实验目标
本lab能够实现的Cache性能优化如下：

1. cache替换策略：用ARC替代LRU
2. 旁路技术（详见报告）
3. 预取策略
4. 各级cache的miss rate以及总的AMAT

****************
### 运行流程
进入lab3.2-prefetch+bypass+arc文件夹
*MakeFile* 编译,在linux终端键入

```
$ make
```

若存在*.o键入

```
$ make clean;make
```

以命令行参数形式处理2个trace文件

```
$ ./sim 01-mcf-gem5-xcg.trace
$ ./sim 02-stream-gem5-xaa.trace
```
键入是否bypass，可以得到相应的l1,l2的miss rate和总体的AMAT
