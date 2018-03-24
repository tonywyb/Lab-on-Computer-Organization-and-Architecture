typedef unsigned long long REG;

struct IFID{
	unsigned int inst;
	int PC;
}IF_ID,IF_ID_old;

struct IDEX{
	int Rd,Rs1,Rs2;
	int PC;
	int Imm;
	long long Reg_Rs1,Reg_Rs2;

	char Ctrl_EX_ALUSrc;
	char Ctrl_EX_ALUOp;
	char Ctrl_EX_RegDst;

	char Ctrl_M_Branch;
	char Ctrl_M_MemWrite;
	char Ctrl_M_MemRead;

	char Ctrl_WB_RegWrite;
	char Ctrl_WB_MemtoReg;

}ID_EX,ID_EX_old;

struct EXMEM{
	int PC;
	int Reg_dst;
	long long ALU_out;
	int Zero;
	long long Reg_Rs2;

	//char Ctrl_M_MemLen;
	char Ctrl_M_Branch;
	char Ctrl_M_MemWrite;
	char Ctrl_M_MemRead;

	char Ctrl_WB_RegWrite;
	char Ctrl_WB_MemtoReg;

}EX_MEM,EX_MEM_old;

struct MEMWB{
	int PC;
	long long Mem_read;
	long long ALU_out;
	int Reg_dst;

	char Ctrl_WB_RegWrite;
	char Ctrl_WB_MemtoReg;

}MEM_WB,MEM_WB_old;
