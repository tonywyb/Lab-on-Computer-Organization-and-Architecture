#include<iostream>
#include<stdio.h>
#include<math.h>
//#include <io.h>
//#include <process.h>
#include<time.h>
#include<stdlib.h>
#include"Reg_def.h"

#define OP_ADDW 0x3b

#define F3_ADDW 0
#define F3_MULW 0
#define F7_ADDW 0
#define F7_MULW 1

#define OP_R 0x33

#define F3_ADD 0
#define F3_MUL 0
#define F3_SUB 0
#define F3_SLL 1
#define F3_MULH 1
#define F3_SLT 2
#define F3_XOR 4
#define F3_DIV 4
#define F3_SRL 5
#define F3_SRA 5
#define F3_OR 6
#define F3_REM 6
#define F3_AND 7

#define F7_ADD 0
#define F7_MUL 1
#define F7_MULH 1
#define F7_SUB 0x20
#define F7_SLL 0
#define F7_SLT 0
#define F7_XOR 0
#define F7_DIV 1
#define F7_SRL 0
#define F7_SRA 0x20
#define F7_OR 0
#define F7_REM 1
#define F7_AND 0


#define OP_LW 3
#define F3_LB 0
#define F3_LH 1
#define F3_LW 2
#define F3_LD 3

#define OP_I 0x13
#define F3_ADDI 0
#define F3_SLLI 1
#define F3_SLTI 2
#define F3_XORI 4
#define F3_SRLI 5
#define F3_SRAI 5
#define F3_ORI 6
#define F3_ANDI 7
#define IMM_SLLI 0
#define IMM_SRLI 0
#define IMM_SRAI 0x10

#define OP_IW 0x1B
#define F3_ADDIW 0
#define F3_SLLIW 1
#define F3_SRLIW 5

#define OP_JALR 0x67
#define F3_IJ 0

#define OP_SCALL 0x73
#define F3_SCALL 0
#define IMM_SCALL 0

#define OP_SW 0x23
#define F3_SB 0
#define F3_SH 1
#define F3_SW 2
#define F3_SD 3

#define OP_BEQ 0x63
#define F3_BEQ 0
#define F3_BNE 1
#define F3_BLT 4
#define F3_BGE 5

#define OP_UA 0x17
#define OP_UL 0x37
#define OP_JAL 0x6f

#define ALU_ADD 0
#define ALU_MUL 1
#define ALU_SUB 2
#define ALU_SLL 3
#define ALU_XOR 4 
#define ALU_DIV 5
#define ALU_SRL 6
#define ALU_OR 7
#define ALU_REM 8
#define ALU_AND 9
#define ALU_SLT 10
#define ALU_SRA 11
#define ALU_MULH 12
#define ALU_ADDIW 13
#define ALU_JALR 14
#define ALU_ADDW 15
#define ALU_MULW 16
#define ALU_SLLIW 17
#define ALU_SRLIW 18

#define NOT_BRANCH 0
#define BEQ 1
#define BNE 2
#define BLT 3
#define BGE 4
#define B_JALR 5
#define B_JAL 6
#define SYS_BRANCH 7
//ALUop:	0,1,2,3, 4,5,6, 7,8,9,10, 11, 12
//		    +,*,-,<<,^,/,>>,|,%,&,slt,sra,mulh


#define MAX 100000000

//主存
unsigned int memory[MAX]={0};
//寄存器堆
REG reg[32]={0};
//PC
int PC=0;


//各个指令解析段
unsigned int OP=0;
unsigned int fuc3=0,fuc7=0;
int shamt=0;
int rs=0,rt=0,rd=0;
unsigned int imm12=0;
unsigned int imm20=0;
unsigned int imm7=0;
unsigned int imm5=0;



//加载内存
void load_memory();

void simulate();

void IF();

void ID();

void EX();

void MEM();

void WB();


//符号扩展
int ext_signed(int src,int bit);

//获取指定位
unsigned int getbit(int s,int e);
unsigned int setbyte(unsigned int mem, int s, unsigned int  rs);
unsigned int getbit(unsigned inst,int s,int e)
{
	unsigned int mask = 0;
	for(int i = s;i <= e;++i)
	{
		mask = mask|(1<<i);
	}
	return (inst & mask) >> s;
}
int ext_signed(int src,int bit)
{
	int res = (src<< (32-bit)) >> (32-bit);
    return res;
}


