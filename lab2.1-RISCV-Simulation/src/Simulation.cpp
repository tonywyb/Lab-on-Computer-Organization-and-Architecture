#include "Simulation.h"
#include<cstring>
using namespace std;

extern void read_elf(char* path, char* res_path);
extern unsigned int cadr;
extern unsigned int csize;
extern unsigned int vadr;
extern unsigned long long gp;
extern unsigned int madr;
extern unsigned int endPC;
extern unsigned int entry;
extern unsigned int res_adr;
extern FILE *file;
bool if_debug = 0;
bool if_help = 0;
bool is_first = 0;
bool if_final_debug = 0;

//指令运行数
long long inst_num=0;

//系统调用退出指示
int exit_flag=0;

//加载代码段
//初始化PC
void load_memory()
{
	fseek(file,cadr,SEEK_SET);
	fread(&memory[vadr>>2],1,csize,file);

	vadr=vadr>>2;
	csize=csize>>2;
	fclose(file);
}

int main(int argc, char **argv)
{
	//解析elf文件
	read_elf(argv[1], argv[2]);
	if (strcmp(argv[3], "-p") == 0)
	{
		if_debug = 1;
		if_help = 0;
		if_final_debug = 0;
	}
	else if(strcmp(argv[3], "-a") == 0)
	{
		if_debug = 0;
		if_help = 0;
		if_final_debug = 0;
	}
	else if(strcmp(argv[3], "-h") == 0)
	{
		if_help = 1;
		if_final_debug = 0;
	}
	else if(strcmp(argv[3], "-af") == 0)
	{
		if_final_debug = 1;
		if_debug = 0;
		if_help = 0;
	}
	else
	{
		if_help = 0;
		printf("invalid argv!\n");
		printf("use -h to show the info!\n");
		return 1;
	}
	if (if_help == 1)
	{
		printf("Usage: ./main (<argv1>) (<argv2>) <option>\n");
		printf("	<option>:\n");
		printf("		-h		See help\n");
		printf("		-a		run program in normal mode\n");
		printf("		-p		run program in debug mode\n");
		printf("		-af		run program in normal mode but stop at the final instruction\n");
		printf("	debug mode:\n");
		printf("		 q		quit debug mode\n");
		printf("		 c		continue to next instruction\n");
		printf("		 r		change debug mode to normal mode\n");
		printf("		 i		show the register info\n");
		printf("		 m		show the memory info\n");
		return 0;
	}
	//设置全局数据段地址寄存器
	reg[3]=gp;

	reg[2]=MAX/2;//栈基址 （sp寄存器）

	printf("start running!\n");
	printf("read_elf complete!\n");
	printf("gp is: %llx\n",gp);
	printf("endPC is: %x\n",endPC);
	printf("entry is: %x\n",entry);
	printf("madr is: %x\n",madr);
	printf("vadr is: %x\n",vadr);
	printf("cadr is: %x\n",cadr);
	printf("csize is: %u\n",csize);
	printf("====================================\n");
	//加载内存
	load_memory();
	printf("load memory complete!\n");
	printf("====================================\n");
	//设置入口地址
	PC=madr>>2;
	printf("the first inst is:%x\n", memory[PC]);

	simulate();

	cout <<"simulate over!"<<endl;

	return 0;
}

void simulate()
{
	//结束PC的设置
	int end=(int)endPC/4-1;
	while(PC!=end)
	{
		//运行
		IF();
		printf("IF done!\n");
		ID();
		printf("ID done!\n");
		EX();
		printf("EX done!\n");
		MEM();
		printf("MEM done!\n");
		WB();
		printf("WB done!\n");
		printf("*******************************\n");
		if(if_debug == 1)
		{
			printf("(debug mode)");
   			char c[10];
   			while(cin >> c)
			{
				printf("(debug mode)");
    			if (strcmp("c", c) == 0)		//continue
     				break;
    			else if (strcmp("q", c) == 0)	//quit
				{
    				exit_flag = 1;
     				break;
    			}
				else if (strcmp("r", c) == 0)	//run to end
				{
					if_debug = 0;
					break;
				}
    			else if (strcmp("i", c) == 0)	//register info
				{
     				for (int i = 0;i < 32;i++)
					{
     					printf("reg[%d] = 0x%x\n", i, reg[i] );
     				}
					printf("(debug mode)");
    			}
				else if (strcmp("m", c) == 0)	//memory info
				{
					 printf("Please enter memory address: 0x");
					 long long st;
					 cin >> st;
					 int num = 4;
					 printf("Please enter the number of words: ");
					 cin >> num;
					 while(num--)
					 {
						 printf("%02x ", getbit(memory[st>>2],0,7));
						 printf("%02x ", getbit(memory[st>>2],8,15));
						 printf("%02x ", getbit(memory[st>>2],16,23));
						 printf("%02x\n", getbit(memory[st>>2],24,31));
						 st+=4;
					 }
					 printf("(debug mode)");
				}
			   	else
				{
					 printf("cannot recognize the parameter!\n");
					 printf("(debug mode)");
			   	}

   			}
  		}
		//更新中间寄存器
		/*
		IF_ID=IF_ID_old;
		ID_EX=ID_EX_old;
		EX_MEM=EX_MEM_old;
		MEM_WB=MEM_WB_old;
		*/
		//exit_flag = 1;
		int res[42] = {0};
		//unsigned char* st = (unsigned char*)memory + res_adr;
		printf("===============================\n");
		for(int i=0; i<42; i++)
		{
			res[i] = *((int*)((unsigned char*)memory + res_adr + 4 * i));
			printf("result[%d] is %d\n", i, res[i]);
		}
		printf("===============================\n");
		for(int i=0;i<32;i++)
		{
			printf("reg[%d] is %llu\n", i, reg[i]);
		}
		printf("===============================\n");
        if(exit_flag==1)
            break;

        reg[0]=0;//一直为零

	}
	if (if_final_debug == 1)
	{
		cout << "res_adr is: 0x" << hex << res_adr << endl;
		printf("(debug mode)");
		char c[10];
		while(cin>>c)
		{
			if (strcmp("q", c) == 0)
				break;
			else if (strcmp("i", c) == 0)	//register info
			{
		 		for (int i = 0;i < 32;i++)
				{
		 			printf("reg[%d] = 0x%x\n", i, reg[i] );
		 		}
				printf("(debug mode)");
			}
			else if (strcmp("m", c) == 0)
			{
				printf("Please enter memory address: 0x");
				long long st;
				cin >> hex >> st;
				int num = 1;
				printf("Please enter the number of words: 0x");
				cin >> num;
				while(num--)
				{
					printf("%02x ", getbit(memory[st>>2],0,7));
					printf("%02x ", getbit(memory[st>>2],8,15));
					printf("%02x ", getbit(memory[st>>2],16,23));
					printf("%02x\n", getbit(memory[st>>2],24,31));
					st+=4;
				}
				printf("(debug mode)");
			}
			else
			{
				printf("cannot recognize the parameter!\n");
				printf("(debug mode)");
			}
		}
	}

}


//取指令
void IF()
{
	printf("PC=%x\n", PC);
	printf("real PC=%x\n", PC*4);
	//write IF_ID_old
	IF_ID.inst=memory[PC];
	printf("instruction is %x\n",IF_ID.inst);
	PC=PC+1;
	IF_ID.PC=PC;
	return;
}

//译码
//EXTOP=0:SIGNEXT;=1:ZEROEXT
//REGDST NONE
//ALUop:	0,1,2,3, 4,5,6, 7,8,9,10
//		+,*,-,<<,^,/,>>,|,%,&,slt
//ALUSrc=0:rs2;=1:imm
//Branch=0,1,2,3,4
//MemRead=0:no;
//=1:byte
//=2:half word
//=3:word
//=4:double word
//MmeWrite=0:no;
//=1:byte
//=2:half word
//=3:word
//=4:double word
//MemtoReg=0:not write;
//1:ALUout;
//2:Memread;
//3:PC+4;
//RegWrite=0:not;=1:write
void ID()
{
	char *inst_name = new char[20];
	//Read IF_I
	unsigned int inst=IF_ID.inst;
	int temp_PC=IF_ID.PC;
	int EXTop=0;
	unsigned int EXTsrc=0;
	int rs1, rs2;
	long long Rs1, Rs2;
	char RegDst = 0,ALUop = 0,ALUSrc = 0;
	char Branch = NOT_BRANCH ,MemRead = 0,MemWrite = 0;
	char RegWrite = 0,MemtoReg = 0;
	unsigned int OP = getbit(inst,0, 6);
	int imm,imm1,imm2,imm3,imm4;
	printf("opcode is:%x\n", OP);
	//....

	if(OP==OP_ADDW)
	{
		rs1=getbit(inst,15,19);
		rs2=getbit(inst,20,24);
		rd=getbit(inst,7,11);
		fuc3=getbit(inst,12,14);
		fuc7=getbit(inst,25,31);
		EXTop=0;
		EXTsrc=0;
		RegDst=rd;
		ALUSrc=0;
		RegWrite = 1;
		Branch=NOT_BRANCH;
		MemtoReg=1;
		MemRead=0;
		MemWrite=0;
		Rs1=reg[rs1];
		Rs2=reg[rs2];
		if(fuc7 == F7_ADDW)
		{
			ALUop=ALU_ADDW;
			inst_name="ADDW";
		}
		else if(fuc7 == F7_MULW)
		{
			ALUop=ALU_MULW;
			inst_name="MULW";
		}
	}
	else if(OP==OP_R)
	{
		rs1=getbit(inst,15,19);
		rs2=getbit(inst,20,24);
		rd=getbit(inst,7,11);
		fuc3=getbit(inst,12,14);
		fuc7=getbit(inst,25,31);
		EXTop=0;
		EXTsrc=0;
		RegDst=rd;
		ALUSrc=0;
		RegWrite = 1;
		Branch=NOT_BRANCH;
		MemtoReg=1;
		MemRead=0;
		MemWrite=0;
		Rs1=reg[rs1];
		Rs2=reg[rs2];
		if(fuc3==F3_ADD&&fuc7==F7_ADD)
		{
			ALUop=ALU_ADD;
			inst_name="ADD";
		}
		else if(fuc3==F3_MUL&&fuc7==F7_MUL)
		{
		   	ALUop=ALU_MUL;
			inst_name="MUL";
		}
		else if(fuc3==F3_MULH&&fuc7==F7_MULH)
		{
			ALUop=ALU_MULH;
			inst_name="MULH";
		}
		else if(fuc3==F3_SUB&&fuc7==F7_SUB)
		{
		   	ALUop=ALU_SUB;
			inst_name="SUB";
		}
		else if(fuc3==F3_SLL&&fuc7==F7_SLL)
		{
		   	ALUop=ALU_SLL;
			inst_name="SLL";
		}
		else if(fuc3==F3_XOR&&fuc7==F7_XOR)
		{
		   	ALUop=ALU_XOR;
		}
		else if(fuc3==F3_DIV&&fuc7==F7_DIV)
		{
		   	ALUop=ALU_DIV;
			inst_name="DIV";
		}
		else if(fuc3==F3_SRL&&fuc7==F7_SRL)
		{
		   	ALUop=ALU_SRL;
			inst_name="SRL";
		}
		else if(fuc3==F3_SRA&&fuc7==F7_SRA)
		{
			ALUop=ALU_SRA;
			inst_name="SRA";
		}
		else if(fuc3==F3_OR&&fuc7==F7_OR)
		{
		   	ALUop=ALU_OR;
			inst_name="OR";
		}
		else if(fuc3==F3_REM&&fuc7==F7_REM)
		{
		   	ALUop=ALU_REM;
			inst_name="REM";
		}
		else if(fuc3==F3_AND&&fuc7==F7_AND)
		{
		   	ALUop=ALU_AND;
			inst_name="AND";
		}
		else if(fuc3==F3_SLT&&fuc7==F7_SLT)
		{
		   	ALUop=ALU_SLT;
			inst_name="SLT";
		}
	}
	else if(OP==OP_LW)
	{
		rs1=getbit(inst,15,19);
		imm=getbit(inst,20,31);
		rd=getbit(inst,7,11);
		fuc3=getbit(inst,12,14);
		EXTop=12;
		EXTsrc=imm;
		imm=ext_signed(imm,12);
		RegDst=rd;
		ALUop=ALU_ADD;
		ALUSrc=1;//from imm
		RegWrite = 1;
		Branch=NOT_BRANCH;
		MemtoReg=2;//Memread to register
		MemWrite=0;
		Rs1=reg[rs1];
		switch(fuc3)
		{
			case F3_LB:
				MemRead=1;
				inst_name="LB";
				break;
			case F3_LH:
				MemRead=2;
				inst_name="LH";
				break;
			case F3_LW:
				MemRead=3;
				inst_name="LW";
				break;
			case F3_LD:
				MemRead=4;
				inst_name="LD";
				break;
		}
	}
	else if(OP==OP_I)
	{
		rs1=getbit(inst,15,19);
		imm=getbit(inst,20,31);
		rd=getbit(inst,7,11);
		fuc3=getbit(inst,12,14);
		EXTop=12;
		EXTsrc=imm;
		imm=ext_signed(imm,12);
		RegDst=rd;
		ALUSrc=1;//from imm
		RegWrite = 1;
		Branch=NOT_BRANCH;
		MemtoReg=1;//ALU_out to Register
		MemWrite=0;
		Rs1=reg[rs1];
		switch(fuc3)
		{
			case F3_ADDI:
				ALUop=ALU_ADD;
				inst_name="ADDI";
				break;
			case F3_SLLI:
				imm=imm&0x3f;
				ALUop=ALU_SLL;
				inst_name="SLLI";
				break;
			case F3_SLTI:
				ALUop=ALU_SLT;
				inst_name="SLTI";
				break;
			case F3_XORI:
				ALUop=ALU_XOR;
				inst_name="XORI";
				break;
			case F3_SRLI:
				if (imm == IMM_SRLI)
				{
					imm=imm&0x3f;
					ALUop=ALU_SRL;
					inst_name="SRLI";
				}
				else if (imm == IMM_SRAI)
				{
					imm=imm&0x3f;
					ALUop=ALU_SRA;
					inst_name="SRAI";
				}
				break;
			case F3_ORI:
				ALUop=ALU_OR;
				inst_name="ORI";
				break;
			case F3_ANDI:
				ALUop=ALU_AND;
				inst_name="ANDI";
				break;
		}
	}
	else if(OP==OP_IW)
	{
		rs1=getbit(inst,15,19);
		imm=getbit(inst,20,31);
		rd=getbit(inst,7,11);
		fuc3=getbit(inst,12,14);
		EXTop=12;// EXTEND IMM
		EXTsrc=imm;
		imm=ext_signed(imm,12);
		RegDst=rd;
		ALUSrc=1;//from imm
		RegWrite=1;
		Branch=NOT_BRANCH;
		MemtoReg=1;//ALU_out to Register
		MemRead=0;
		MemWrite=0;
		Rs1=reg[rs1];
		if (fuc3 == F3_ADDIW)
		{
			ALUop=ALU_ADDIW;
			inst_name="ADDIW";
		}
		else if (fuc3 == F3_SLLIW )
		{
			ALUop=ALU_SLLIW;
			inst_name="SLLIW";
		}
		else if (fuc3 == F3_SRLIW)
		{
			ALUop=ALU_SRLIW;
			inst_name="SRLIW";
		}
	}
	else if(OP==OP_JALR)		//  how!!!
	{
		rs1=getbit(inst,15,19);
		imm=getbit(inst,20,31);
		//EXTop=12;
		//EXTsrc=imm;
		imm=ext_signed(imm,12);
		imm=imm>>2;
		rd=getbit(inst,7,11);
		fuc3=getbit(inst,12,14);
		RegDst=rd;
		ALUSrc=1;//from imm
		RegWrite = 1;
		Branch=B_JALR;
		MemtoReg=3;// PC+4 to Register
		MemRead=0;
		MemWrite=0;
		RegWrite=1;
		ALUop=ALU_JALR;
		Rs1=reg[rs1];
		Rs2=imm;
		inst_name="JALR";

	}
	/*
	else if(OP==OP_SCALL)	//
	{
		EXTop=0;
		RegDst=0;
		ALUSrc=1;//from imm
		RegWrite = 1;
		Branch=SYS_BRANCH;
		MemtoReg=0;
		MemRead=0;
		MemWrite=0;
		RegWrite=1;
		//ALUop=?
		rs1=getbit(inst,15,19);
		imm=getbit(inst,20,31);
		rd=getbit(inst,7,11);
		fuc3=getbit(inst,12,14);
	}
	*/
    else if(OP==OP_SW)
    {
    	rs1=getbit(inst,15,19);
		rs2=getbit(inst,20,24);
		imm1=getbit(inst,7,11);
		fuc3=getbit(inst,12,14);
		imm2=getbit(inst,25,31);
		imm = (imm2 << 5) + imm1;
		EXTsrc = imm;
    	EXTop=12;
		imm=ext_signed(imm,12);
		RegDst=0;
		ALUSrc=1;// ALU input2 from imm
		RegWrite=0;
		Branch=NOT_BRANCH;
		MemtoReg=0;
		MemRead=0;

		Rs1=reg[rs1];
		Rs2=reg[rs2];
    	switch(fuc3)
    	{
    		case F3_SB:
    			MemWrite=1;
    			Rs2=Rs2&0xff;
				inst_name="SB";
    			break;
    		case F3_SH:
    			MemWrite=2;
    			Rs2=Rs2&0xffff;
				inst_name="SH";
    			break;
    		case F3_SW:
    			MemWrite=3;
    		    Rs2=Rs2&0xffffffff;
				inst_name="SW";
    			break;
    		case F3_SD:
    			MemWrite=4;
    			Rs2=Rs2&0xffffffffffffffff;
				inst_name="SD";
    			break;
    	}
    }
    else if(OP==OP_BEQ)
    {
    	rs1=getbit(inst,15,19);
		rs2=getbit(inst,20,24);
		fuc3=getbit(inst,12,14);
		imm1=getbit(inst,8,11);
		imm2=getbit(inst,25,30);
		imm3=getbit(inst,7,7);
		imm4=getbit(inst,31,31);
		RegDst=0;
		ALUSrc=0; 	//ALU input2 from register
		ALUop=ALU_SUB;
		RegWrite = 0;
		MemtoReg=0;
		MemRead=0;
		MemWrite=0;
		imm=imm1+(imm2<<4)+(imm3<<10)+(imm4<<11);
		imm=(imm<<1)&(~0x1);
		imm=ext_signed(imm,12);
		imm=imm>>2;
		//EXTop=11;
		//EXTsrc=imm;
		Rs1=reg[rs1];
		Rs2=reg[rs2];
    	switch(fuc3)
    	{
    		case F3_BEQ:
    			Branch=BEQ;
				inst_name="BEQ";
    			break;
    		case F3_BNE:
    			Branch=BNE;
				inst_name="BNE";
    			break;
    		case F3_BLT:
    			Branch=BLT;
				inst_name="BLT";
    			break;
    		case F3_BGE:
    			Branch=BGE;
				inst_name="BGE";
    			break;
			default:
				break;
    	}
    }
    else if(OP==OP_UA)
    {
    	rd=getbit(inst,7,11);
    	imm=getbit(inst,12,31);
    	imm=(imm<<12)&(~0xfff);
		imm=imm>>2;
    	EXTop=0;
		EXTsrc=imm;
		RegDst=rd;
		ALUSrc=1; 	//ALU input2 from imm
		ALUop=ALU_ADD;
		Branch=NOT_BRANCH;
		MemtoReg=1;
		MemRead=0;
		MemWrite=0;
		RegWrite=1;
    	Rs1=temp_PC-1;
    	Rs2=imm;
		inst_name="AUIPC";
    }
    else if(OP==OP_UL)
    {
    	rd=getbit(inst,7,11);
    	imm=getbit(inst,12,31);
    	imm=(imm<<12)&(~0xfff);
    	EXTop=0;
		EXTsrc=imm;
		RegDst=rd;
		ALUSrc=1;//ALU input2 from imm
		ALUop=ALU_ADD;
		Branch=NOT_BRANCH;
		MemtoReg=1;
		MemRead=0;
		MemWrite=0;
		RegWrite=1;

    	Rs1=0;
    	Rs2=imm;
		inst_name="LUI";
    }
    else if(OP==OP_JAL)
    {
    	rd=getbit(inst,7,11);
    	imm1=getbit(inst,21,30);
    	imm2=getbit(inst,20,20);
    	imm3=getbit(inst,12,19);
       	imm4=getbit(inst,31,31);
		imm=imm1+(imm2<<10)+(imm3<<11)+(imm4<<19);
       	imm=imm<<1;
		EXTop=21;
		EXTsrc=imm;
		imm=ext_signed(imm,21);
		imm=imm>>2;
		RegDst=rd;
		ALUSrc=1;	//ALU input2 from imm
		ALUop=ALU_ADD;
		Branch=B_JAL;
		MemtoReg=3;
		MemRead=0;
		MemWrite=0;
		RegWrite=1;
       	Rs1=temp_PC-1;
       	Rs2=imm;
		inst_name="JAL";
    }
	else
	{
		inst_name="UNK";
		printf("inst_name is %s\n",inst_name);
		printf("its op is:0x%x\n", OP);
		if(is_first == 1)
		{
			is_first = 0;
		}
		else
		{
			exit_flag = 1;
		}

	}
	//write ID_EX_old
	ID_EX.PC = temp_PC;
	ID_EX.Rd=rd;
	ID_EX.Rs1=rs1;
	ID_EX.Rs2=rs2;
	ID_EX.Reg_Rs1=Rs1;
	ID_EX.Reg_Rs2=Rs2;
	ID_EX.Imm=imm;
	printf("imm is 0x%x\n", ID_EX.Imm);
	printf("imm is %d\n", ID_EX.Imm);
	//...
	ID_EX.Ctrl_EX_ALUSrc=ALUSrc;
	ID_EX.Ctrl_EX_ALUOp=ALUop;
	ID_EX.Ctrl_EX_RegDst=RegDst;

	ID_EX.Ctrl_M_Branch=Branch;
	ID_EX.Ctrl_M_MemWrite=MemWrite;
	ID_EX.Ctrl_M_MemRead=MemRead;

	ID_EX.Ctrl_WB_RegWrite=RegWrite;
	ID_EX.Ctrl_WB_MemtoReg=MemtoReg;
	if(inst_name != "UNK")
	{
		printf("inst_name is %s\n",inst_name);
	}
	//....
	printf("Rs1 = %llu\n", Rs1);
	printf("Rs2 = %llu\n", Rs2);
	printf("rs1 = %llu\n", rs1);
	printf("rs2 = %llu\n", rs2);
	printf("PC = 0x%x\n", temp_PC);
	printf("real PC = 0x%x\n", temp_PC*4);
	return;
}

//执行
void EX()
{
	//read ID_EX
	int temp_PC = ID_EX.PC;
	int rs1 = ID_EX.Rs1;
	int rs2 = ID_EX.Rs2;
	int rd = ID_EX.Rd;
	int imm = ID_EX.Imm;
	long long Rs1 = ID_EX.Reg_Rs1;
	long long Rs2 = ID_EX.Reg_Rs2;

	char ALUSrc = ID_EX.Ctrl_EX_ALUSrc;
	char ALUOp = ID_EX.Ctrl_EX_ALUOp;
	char RegDst = ID_EX.Ctrl_EX_RegDst;

	char Branch = ID_EX.Ctrl_M_Branch;
	char MemWrite = ID_EX.Ctrl_M_MemWrite;
	char MemRead = ID_EX.Ctrl_M_MemRead;

	char RegWrite = ID_EX.Ctrl_WB_RegWrite;
	char MemtoReg = ID_EX.Ctrl_WB_MemtoReg;

	int Reg_dst=0;
	if(RegDst != 0)
	{
		Reg_dst = rd;
	}
	else
	{
		Reg_dst = rs2;
	}
	long long input1, input2;
	input1 = Rs1;
	if(ALUSrc == 0)
	{
		input2 = (long long)reg[rs2];
	}
	else if (ALUSrc == 1)
	{
		input2 = (long long)imm;
	}
	printf("EX input1 = 0x%llx\n", input1);
	printf("EX input1 = %lld\n", input1);
	printf("EX input2 = 0x%llx\n", input2);
	printf("EX input2 = %lld\n", input2);
	//Branch PC calulate
	//...

	//choose ALU input number
	//...


	//alu calculate
	int Zero;
	long long ALUout;
	long long A_Low, A_High, B_High, B_Low, AHBH, AHBL, ALBH;
	int addw1, addw2;
	int mulw1, mulw2;
	int slliw1, slliw2;
	int srliw1, srliw2;
	long long addw_res;
	long long mulw_res;
	long long slliw_res;
	long long srliw_res;
	switch(ALUOp)
	{
		case ALU_MULW:
			mulw1 = (int)input1;
			mulw2 = (int)input2;
			mulw_res = (long long)(mulw1 * mulw2);
			ALUout = mulw_res;
			break;
		case ALU_ADDW:
			addw1 = (int)input1;
			addw2 = (int)input2;
			addw_res = (long long)(addw1 + addw2);
			ALUout = addw_res;
			break;
		case ALU_ADD:
			ALUout = input1 + input2;
			if (Branch == B_JAL)
			{
				temp_PC = ALUout;
			}
			break;
		case ALU_MUL:
			ALUout = input1 * input2;break;
		case ALU_SUB:
			ALUout = input1 - input2;
			if(Branch == BEQ)
			{
				if(ALUout == 0)
				{
					temp_PC = temp_PC - 1 + imm;
				}
			}
			else if(Branch == BNE)
			{
				if(ALUout != 0)
				{
					temp_PC = temp_PC - 1 + imm;
				}

			}
			else if(Branch == BLT)
			{
				if(input1 < input2)
				{
					temp_PC = temp_PC - 1 + imm;
				}
			}
			else if(Branch == BGE)
			{
				if(input1 >= input2)
				{
					temp_PC = temp_PC - 1 + imm;
				}
			}
			break;
		case ALU_SLL:
			ALUout = input1 << input2;break;
		case ALU_MULH:
			A_High = ((input1 & 0xFFFFFFFF00000000) >> 32) & 0xFFFFFFFF;
			A_Low = input1 & 0x00000000FFFFFFFF;
			B_High = ((input2 & 0xFFFFFFFF00000000) >> 32) & 0xFFFFFFFF;
			B_Low = input2 & 0x00000000FFFFFFFF;
			AHBH = A_High*B_High;
			AHBL = ((A_High*B_Low) >> 32) & 0xFFFFFFFF;
			ALBH = ((A_Low*B_High) >> 32) & 0xFFFFFFFF;
			ALUout = AHBH + AHBL + ALBH;
			break;

		case ALU_SLT:
			if (input1 < input2)
			{
				ALUout = 1;
			}
			else
			{
				ALUout = 0;
			}
			break;
		case ALU_XOR:
			ALUout = input1 ^ input2;break;
		case ALU_DIV:
			ALUout = input1 / input2;break;
		case ALU_SRL:
			ALUout = (unsigned long long)input1 >> input2;break;
		case ALU_SRA:
			input1 = (signed long long)input1;
			ALUout = input1 >> input2;break;
		case ALU_OR:
			ALUout = input1 | input2;break;
		case ALU_REM:
			ALUout = input1 % input2;break;
		case ALU_AND:
			ALUout = input1 & input2;break;
		case ALU_ADDIW:
			ALUout = ext_signed(getbit(input1 + input2,0,31),32);break;
		case ALU_SLLIW:
			slliw1 = (int)input1;
			slliw2 = (int)input2;
			slliw_res = (long long)(slliw1 << slliw2);
			ALUout = slliw_res;
			break;
		case ALU_SRLIW:
			srliw1 = (int)input1;
			srliw2 = (int)input2;
			srliw_res = (long long)(srliw1 >> srliw2);
			ALUout = srliw_res;
			break;
		case ALU_JALR:
			ALUout = input1 + input2;
			temp_PC = ALUout;
			break;
		default:break;

	}
	printf("ALUout = %llx\n", ALUout);
	printf("ALUout = %lld\n", ALUout);
	//choose reg dst address
	/*
	int Reg_Dst;
	if(RegDst)
	{

	}
	else
	{

	}
	*/
	//write EX_MEM_old
	EX_MEM.PC = temp_PC;
	EX_MEM.Reg_dst = Reg_dst;
	EX_MEM.ALU_out = ALUout;
	EX_MEM.Reg_Rs2 = Rs2;

	EX_MEM.Ctrl_M_Branch = Branch;
	EX_MEM.Ctrl_M_MemWrite = MemWrite;
	EX_MEM.Ctrl_M_MemRead = MemRead;

	EX_MEM.Ctrl_WB_RegWrite = RegWrite;
	EX_MEM.Ctrl_WB_MemtoReg = MemtoReg;
    //.....
	printf("PC=0x%x\n", temp_PC);
	printf("real PC=0x%x\n", temp_PC*4);
}

//访问存储器
void MEM()
{
	//read EX_MEM
	int temp_PC = EX_MEM.PC;
	int Reg_dst = EX_MEM.Reg_dst;
	long long ALUout = EX_MEM.ALU_out;
	long long Rs2 = EX_MEM.Reg_Rs2;

	char Branch = EX_MEM.Ctrl_M_Branch;
	char MemWrite = EX_MEM.Ctrl_M_MemWrite;
	char MemRead = EX_MEM.Ctrl_M_MemRead;

	char RegWrite = EX_MEM.Ctrl_WB_RegWrite;
	char MemtoReg = EX_MEM.Ctrl_WB_MemtoReg;
	//complete Branch instruction PC change

	//read / write memory
	long long Mem_read = 0;
	char temp;
	short temp1;
	int temp2;
	long long temp3;
	switch(MemRead)
	{
		case 0:
			break;
		case 1://byte
			temp = *((char*)((unsigned char*)memory + ALUout));
			Mem_read = (long long)temp;
			break;
		case 2://half
			temp1 = *((short*)((unsigned char*)memory + ALUout));
			Mem_read = (long long)temp1;
			break;
		case 3://word
			temp2 = *((int*)((unsigned char*)memory + ALUout));
			Mem_read = (long long)temp2;
			break;
		case 4://double word
			temp3 = *((long long*)((unsigned char*)memory + ALUout));
			Mem_read = (long long)temp3;
			break;
	}

	if(MemRead != 0)
	{
		printf("read %llx from memory\n", Mem_read);
		printf("read %lld from memory\n", Mem_read);
	}

	switch(MemWrite)
	{
		case 0:
			break;
		case 1://byte
			Rs2 = (char)Rs2;
			*((char*)((unsigned char*)memory + ALUout)) = Rs2;
			break;
		case 2://half
			Rs2 = (short)Rs2;
			*((short*)((unsigned char*)memory + ALUout)) = Rs2;
			break;
		case 3://word
			Rs2 = (int)Rs2;
			*((int*)((unsigned char*)memory + ALUout)) = Rs2;
			break;
		case 4://double word
			Rs2 = (long long)Rs2;
			*((long long*)((unsigned char*)memory + ALUout)) = Rs2;
			break;
	}

	if(MemWrite != 0)
	{
		printf("write %llx to memory\n", Rs2);
		printf("write %lld to memory\n", Rs2);
	}

	//write MEM_WB_old
	MEM_WB.PC = temp_PC;
	MEM_WB.Mem_read = Mem_read;
	MEM_WB.ALU_out = ALUout;
	MEM_WB.Reg_dst = Reg_dst;

	MEM_WB.Ctrl_WB_RegWrite = RegWrite;
	MEM_WB.Ctrl_WB_MemtoReg = MemtoReg;
	printf("PC=%x\n", temp_PC);
	printf("real PC=%x\n", temp_PC*4);
	return;
}


//写回
void WB()
{
	int temp_PC = MEM_WB.PC;
	long long ALU_out = MEM_WB.ALU_out;
	long long Mem_read = MEM_WB.Mem_read;
	int Reg_dst = MEM_WB.Reg_dst;
	char RegWrite = MEM_WB.Ctrl_WB_RegWrite;
	char MemtoReg = MEM_WB.Ctrl_WB_MemtoReg;
	//read MEM_WB
	long long WB_res = 0;
	if(RegWrite == 0)
	{
	}
	else if (MemtoReg == 1)
	{
		WB_res = ALU_out;
	}
	else if(MemtoReg == 2)
	{
		WB_res = Mem_read;
	}
	else if(MemtoReg == 3)
	{
		WB_res = PC;
	}
	PC = temp_PC;
	//write reg
	if(RegWrite != 0)
	{
		reg[Reg_dst] = WB_res;
		printf("write %llx to Reg[%d]\n", WB_res, Reg_dst);
		printf("write %lld to Reg[%d]\n", WB_res, Reg_dst);
	}
	/*
	printf("reg[14] = %lld\n", reg[14]);
	printf("reg[15] = %lld\n", reg[15]);
	printf("reg[10] = %lld\n", reg[10]);
	printf("reg[11] = %lld\n", reg[11]);
	*/
	printf("PC=0x%x\n", PC);
	printf("real_PC=0x%x\n", PC*4);
	return;
}
