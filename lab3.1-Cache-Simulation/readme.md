# Lab 3.1 Cache-Simulation

### 实验目标
本lab能够实现：

1. elf格式可执行文件的读取与自动分析，并将所需代码和数据自动加载到内存
2. 完整模拟sim函数的运行
3. 单步调试，支持寄存器与存储器的查看
4. 支持单层Cache的模拟，根据输入的参数（cache size, associativity, line_size等）统计miss rate
5. 在lab2.2流水线基础上嵌入支持三级Cache，其中L1将数据指令分开

*****************
### 文件内容
解压压缩包后有：

1. 1500012946\_王彦斌\_lab3.1实验报告.pdf
2. lab3-parta文件夹
3. lab3-partb文件夹
4. readme.md

****************
### 执行脚本
## parta
无执行脚本，运行后根据输出提示依次输入**cache size**, **line size**,**associativity**,**Write missing policy**，**Write hit policy**，**Cache hit latency**。

输出结果包括**Hit times**，**Miss times**， **Hit rate**，**Miss rate**， 
**Request access time**。
## partb
提供执行脚本**run.sh**能够完成编译并将测试样例中的可执行文件进行elf解析与指令模拟，结果分别存放在后缀*_elf.txt*和*_sim.txt*的文件中。*_sim.txt*的文件中包含pipelined的debug输出，结尾有cache的miss rate，hit time等模拟cache数据。
**************
### 调试/运行流程
## parta
*MakeFile* 编译,在linux终端键入

```
$ make
```

若存在*.o键入

```
$ make clean;make
```

运行：

```
$ ./sim 1.trace
$ ./sim 2.trace
```

## partb
*Makefile*编译,在linux终端键入

```
$ make
```
若存在*.o键入

```
$ make clean;make
```

编译结束后生成可执行文件main，正常/调试模式下运行

```
$ ./sim test1 test1_elf.txt <option>
``` 
其中test1为测试elf程序，test1_elf.txt存放add的elf解析结果。option包括：

```
-h		输出help信息
-a  	一次全部执行完
-af  	执行到最后一条指令停止，进入debug模式
-p   	进入debug模式，依次执行一条指令
```
debug模式下支持命令：

```
c		单步调试
i		显示寄存器内容
m		显示指定地址指定长度的内存内容
r		转换到正常模式全部执行完
q		退出

```
由于模拟器debug输出通常较多，可以键入以下命令来存放输出结果，执行脚本**run.sh**也是如此处理的。之后可在*test1_sim.txt*查看**test1**可执行文件的指令模拟信息。

```
$ ./sim test1 test1_elf.txt -a > test1_sim.txt
```
