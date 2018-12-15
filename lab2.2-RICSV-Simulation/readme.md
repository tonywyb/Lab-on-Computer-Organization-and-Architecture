# Lab 2.2 RISC-V Simulation

### 实验目标
本lab能够实现：

1. elf格式可执行文件的读取与自动分析，并将所需代码和数据自动加载到内存
2. 完整模拟main函数的运行
3. 单步调试，支持寄存器与存储器的查看

*****************
### 文件内容
解压压缩包后有：

1. 实验报告lab2.2.pdf
2. 多周期文件夹
3. 流水线文件夹
4. test_sim文件夹
5. readme.md

其中模拟程序代码文件夹中包括：

```
run.sh
Simulation.cpp
Simulation.h
Read_Elf.h
Read_Elf.cpp
Reg_def.h
10个测试程序
```
****************
### 执行脚本
本RISC-V模拟器在lab2.1模板基础上编写而成。执行脚本**run.sh**能够完成编译并将测试样例中的可执行文件进行elf解析与指令模拟，结果分别存放在后缀*_elf.txt*和*_sim.txt*的文件中。
**************
### 调试/运行流程
1. *Makefile*编译,在linux终端键入

```
$ make
```
若存在Simulation.o或Read_Elf.o，键入

```
$ make clean;make
```
2. 编译结束后生成可执行文件main，正常/调试模式下运行

```
$ ./main test1 test1_elf.txt <option>
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
$ ./main test1 test1_elf.txt -a > test1_sim.txt
```
