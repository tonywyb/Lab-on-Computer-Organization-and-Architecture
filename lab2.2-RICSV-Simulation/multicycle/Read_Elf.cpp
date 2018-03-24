#include"Read_Elf.h"

FILE *elf=NULL;
Elf64_Ehdr elf64_hdr;

//Program headers
unsigned int padr=0;
unsigned int psize=0;
unsigned int pnum=0;

//Section Headers
unsigned int sadr=0;
unsigned int ssize=0;
unsigned int snum=0;

//Symbol table
unsigned int symnum=0;
unsigned int symadr=0;
unsigned int symsize=0;

//ÓÃÓÚÖ¸Ê¾ °üº¬½ÚÃû³ÆµÄ×Ö·û´®ÊÇµÚ¼¸¸ö½Ú£¨´ÓÁã¿ªÊ¼¼ÆÊý£©
unsigned int Index=0;

//×Ö·û´®±íÔÚÎÄ¼þÖÐµØÖ·£¬ÆäÄÚÈÝ°üÀ¨.symtabºÍ.debug½ÚÖÐµÄ·ûºÅ±í
unsigned int stradr=0;


bool open_file(char* path)
{
	file = fopen(path, "rb");
	if (file == NULL)
		return false;
	return true;
}

void read_elf(char* path, char* res_path)
{
	if(!open_file(path))
	{
		printf("cannot open elf-file!\n");
		return;
	}	
	elf = fopen(res_path, "w");
	fprintf(elf,"ELF Header:\n");
	read_Elf_header();
	printf("read elf header done!\n");
	fflush(stdout);
	fprintf(elf,"\n\nSection Headers:\n");
	read_elf_sections();
	printf("read sections done!\n");
	fflush(stdout);
	fprintf(elf,"\n\nProgram Headers:\n");
	read_Phdr();
	printf("read program done!\n");
	fflush(stdout);
	fprintf(elf,"\n\nSymbol table:\n");
	read_symtable();
	printf("read symtable done!\n");
	fflush(stdout);
	fclose(elf);
}

void read_Elf_header()
{
	//file should be relocated
	fread(&elf64_hdr,1,sizeof(elf64_hdr),file);
	
	fprintf(elf," magic number:  ");
	for (int i = 0; i < 16; ++i) 
		fprintf(elf, " %02x",elf64_hdr.e_ident[i]);
	fprintf(elf, "\n");
	
	if (elf64_hdr.e_ident[4] == 1) 
		fprintf(elf," Class:  ELFCLASS32\n");		
	else if (elf64_hdr.e_ident[4] == 2) 
		fprintf(elf," Class:  ELFCLASS64\n");
	
	if (elf64_hdr.e_ident[5] == 1) 
		fprintf(elf," Data:  little-endian\n");
	else if (elf64_hdr.e_ident[5] == 2) 
		fprintf(elf," Data:  big-endian\n");
	
		
	fprintf(elf," Version:   %u\n", elf64_hdr.e_version);

	fprintf(elf," OS/ABI:	 System V ABI\n");
	
	fprintf(elf," ABI Version:   0\n");
	
	unsigned short type = *(unsigned short*)(&elf64_hdr.e_type);
	switch(type)
	{
		case 1:
			fprintf(elf, " Type:  REL\n");break;
		case 2: 
			fprintf(elf, " Type:  EXEC\n");break;
		case 3:
			fprintf(elf, " Type:  DYN\n");break;
		default:
			fprintf(elf, " Type:  UNKNOWN\n");break;
	}

	unsigned short machine = *(unsigned short*)(&elf64_hdr.e_machine);
	switch(machine)
	{
		case 0x3:
			fprintf(elf, " Machine:    INTEL x86\n");break;
		case 0xf3:
			fprintf(elf, " Machine:    RISC-V\n");break;
		default:
			fprintf(elf, " Machine:    UNKNOWN\n");break;
	}

    
	fprintf(elf," Version:  0x%x\n", elf64_hdr.e_version);

	entry = (unsigned int)(*((unsigned long*)(&elf64_hdr.e_entry)));

	fprintf(elf," Entry point address:  0x%x\n", elf64_hdr.e_entry);

	padr = (unsigned int)(*(unsigned long*)(&elf64_hdr.e_phoff));

	fprintf(elf," Start of program headers:   %ld bytes into  file\n",elf64_hdr.e_phoff);

	sadr = (unsigned int)(*(unsigned long*)(&elf64_hdr.e_shoff));

	fprintf(elf," Start of section headers:   %ld bytes into  file\n",elf64_hdr.e_shoff);

	fprintf(elf," Flags:  0x%x\n",elf64_hdr.e_flags);

	fprintf(elf," Size of this header:  %u Bytes\n",elf64_hdr.e_ehsize);
	psize = (unsigned int)(*((unsigned short*)(&elf64_hdr.e_phentsize)));
	fprintf(elf," Size of program headers:  %u Bytes\n",elf64_hdr.e_phentsize);
	pnum = (unsigned int)(*((unsigned short*)(&elf64_hdr.e_phnum)));
	fprintf(elf," Number of program headers:    %u\n",elf64_hdr.e_phnum);
	ssize = (unsigned int)(*((unsigned short*)(&elf64_hdr.e_shentsize)));
	fprintf(elf," Size of section headers:   %u Bytes\n",elf64_hdr.e_shentsize);
	snum = (unsigned int)(*((unsigned short*)(&elf64_hdr.e_shnum)));
	fprintf(elf," Number of section headers:   %u\n",elf64_hdr.e_shnum);
	Index = (unsigned int)(*((unsigned short*)(&elf64_hdr.e_shstrndx)));
	fprintf(elf," Section header string table index:   %u\n",elf64_hdr.e_shstrndx);
	
}

void read_elf_sections()
{
	int ncnt = 0;
	Elf64_Shdr elf64_shdr;
	char name[30] = {0};
	unsigned long int shstradr;
	if (fseek(file,sadr,SEEK_SET)==-1)
	{
		printf("read sections error\n");
		return;
	}
	for(int i=0;i<snum;i++)
	{
		fread(&elf64_shdr,1,sizeof(elf64_shdr),file);
		if(i == Index)
			shstradr=*((unsigned long*)(&elf64_shdr.sh_addr)) + *((unsigned long*)(&elf64_shdr.sh_offset));
	}

	for(int c=0;c<snum;c++)
	{
		fprintf(elf," [%3d]\n",c);
		
		//file should be relocated
		fseek(file, sadr + c * sizeof(elf64_shdr), SEEK_SET);
		fread(&elf64_shdr,1,sizeof(elf64_shdr),file);

		unsigned long str_adr = shstradr + *(unsigned int*)&elf64_shdr.sh_name;
		fseek(file, str_adr, SEEK_SET);
		fread(&name[ncnt], 1, 1, file);
		while(name[ncnt] != '\0')
		{
			ncnt++;
			fread(&name[ncnt],1,1,file);
		}
		ncnt=0;
		fprintf(elf," Name: %s",name);

		if(!strcmp(name, ".strtab"))
		{
			stradr=*((unsigned long int*)(&elf64_shdr.sh_addr)) + *((unsigned long int*)(&elf64_shdr.sh_offset));
		}
		if(!strcmp(name, ".symtab"))
		{
			symsize=*((unsigned long int*)(&elf64_shdr.sh_entsize));
		}
		int tmp = *((int*)(&elf64_shdr.sh_type));
		/*
		if (!strcmp(name, ".data")) {
			gp = *((unsigned long *)(&elf64_phdr.sh_addr));
		}

		if (!strcmp(name, ".text")) {
			endPC = *((unsigned long *)(&elf64_shdr.sh_addr)) + *((unsigned long *)(&elf64_shdr.sh_size));
		}
		*/

		switch(tmp)
		{
			case 0:
				fprintf(elf, " Type: NULL");break;
			case 1:
				fprintf(elf, " Type: PROGBITS");break;
			case 2:
				fprintf(elf, " Type: SYMTAB");
				symnum = *((unsigned long int*)(&elf64_shdr.sh_size))/symsize;
				symadr = *((unsigned long int*)(&elf64_shdr.sh_addr)) + *((unsigned long int*)(&elf64_shdr.sh_offset));
				break;
			case 3:
				fprintf(elf, " Type: STRTAB");break;
			case 5:
				fprintf(elf, " Type: HASH");break;
			case 6:
				fprintf(elf, " Type: DYNAMIC");break;
			case 8:
				fprintf(elf, " Type: NOBITS");break;
			case 14:
				fprintf(elf, " Type: INIT_ARRAY");break;
			case 15:
				fprintf(elf," Type: FINI_ARRAY");break;
		}
		fprintf(elf," Address:  %016x",elf64_shdr.sh_addr);

		fprintf(elf," Offset:  %08x\n",elf64_shdr.sh_offset);

		fprintf(elf," Size:  %016x",elf64_shdr.sh_size);

		fprintf(elf," Entsize:  %016x\n",elf64_shdr.sh_entsize);

		fprintf(elf," Flags:  %x",elf64_shdr.sh_flags);
		
		fprintf(elf," Link:  %d",elf64_shdr.sh_link);

		fprintf(elf," Info:  %d",elf64_shdr.sh_info);

		fprintf(elf," Align:  %d\n",elf64_shdr.sh_addralign);

 	}
}

void read_Phdr()
{
	Elf64_Phdr elf64_phdr;
	if (fseek(file, padr, SEEK_SET)==-1)
	{
		printf("read program error!\n");
		return;
	}
	int load_cnt = 0;
	for(int c=0; c<pnum; c++)
	{
		fprintf(elf," [%3d]\n",c);
			
		//file should be relocated
		fread(&elf64_phdr,1,sizeof(elf64_phdr),file);
		int tmp = *(int*)(&elf64_phdr.p_type);
		switch (tmp)
		{
			case 0: 
				fprintf(elf, " Type:   NULL");break;
			case 1:
				fprintf(elf, " Type:   LOAD");
				if (load_cnt == 0)
				{
					cadr = (unsigned int)*(unsigned long*)&elf64_phdr.p_offset;
					vadr = (unsigned int)*(unsigned long*)&elf64_phdr.p_vaddr;
					load_cnt ++;
				}
				csize = *((unsigned long*)(&elf64_phdr.p_offset)) - cadr + *((unsigned long*)(&elf64_phdr.p_filesz));
				break;
			case 2:
				fprintf(elf, " Type:   DYNAMIC");break;
			case 3:
				fprintf(elf, " Type:   INTERP");break;
			case 4:
				fprintf(elf, " Type:   NOTE");break;
			case 5:
				fprintf(elf, " Type:   SHLIB");break;
			case 6:
				fprintf(elf, "  Type:   PHDR");break;
			case 7:
				fprintf(elf, " Type:   TLS");break;
			case 8:
				fprintf(elf, " Type:   NUM");break;
			default:
				fprintf(elf, " Type:   UNKONWN");break;
		}
		fprintf(elf," Flags:   0x%x", elf64_phdr.p_flags);
		
		fprintf(elf," Offset:   %ld", elf64_phdr.p_offset);
		
		fprintf(elf," VirtAddr:  %ld", elf64_phdr.p_vaddr);
		
		fprintf(elf," PhysAddr:   %ld", elf64_phdr.p_paddr);

		fprintf(elf," FileSiz:   %ld", elf64_phdr.p_filesz);

		fprintf(elf," MemSiz:   %ld", elf64_phdr.p_memsz);
		
		fprintf(elf," Align:   %ld\n", elf64_phdr.p_align);
	}
}


void read_symtable()
{
	Elf64_Sym elf64_sym;

	char name[40] = {0};

	int ncnt = 0;
	for(int c=0;c<symnum;c++)
	{
		fprintf(elf," [%3d]   ",c);

		fseek(file, symadr + c * sizeof(elf64_sym), SEEK_SET);
		
		//file should be relocated
		fread(&elf64_sym,1,sizeof(elf64_sym),file);

		unsigned long adr = stradr + *((unsigned int*)(&elf64_sym.st_name));

		fseek(file, adr, SEEK_SET);

		fread(&name[ncnt], 1, 1, file);
		while(name[ncnt] != '\0') {
			ncnt++;
			fread(&name[ncnt], 1, 1, file);
		}
		ncnt = 0;


		fprintf(elf," Name:  %40s   \n", name);
		if(strcmp(name, "_gp") == 0 )
		{
			gp = *(unsigned long*)&elf64_sym.st_value;
		}
		if(!strcmp(name, "atexit"))
		{
			endPC = *((unsigned int*)(&elf64_sym.st_value));
		}
		if(!strcmp(name, "main"))
		{
			madr = *((unsigned int*)(&elf64_sym.st_value));
		}
		if(!strcmp(name, "result"))
		{
			res_adr = *((unsigned int*)(&elf64_sym.st_value));
		}
		unsigned int type = (unsigned int)(elf64_sym.st_info&0xF);
		unsigned int bind = (unsigned int)elf64_sym.st_info>>4;

		if(bind == 0){
			fprintf(elf," Bind:   LOCAL");
		}
		else if(bind == 1) {
			fprintf(elf," Bind:   GLOBAL");
		}
		else if(bind == 2){
			fprintf(elf," Bind:   WEAK");
		}
		else if(bind == 3){
			fprintf(elf," Bind:   NUM");
		}else {
			fprintf(elf," Bind:   UNKONWN");
		}
		switch(type)
		{
			case 0:
				fprintf(elf, " Type:   NOTYPE");break;
			case 1:
				fprintf(elf, " Type:   OBJECT");break;
			case 2:
				fprintf(elf, " Type:   FUNC");break;
			case 3:
				fprintf(elf, " Type:   SECTION");break;
			case 4:
				fprintf(elf, " Type:   FILE");break;
			case 5:
				fprintf(elf, " Type:   COMMOM");break;
			case 6:
				fprintf(elf, " Type:   TLS");break;
			case 7:
				fprintf(elf, " Type:   NUM");break;
			default:
				fprintf(elf, " Type:   UNKONWN");break;
		}
		fprintf(elf," NDX:   %hd", elf64_sym.st_shndx);
		
		fprintf(elf," Size:   %ld", elf64_sym.st_size);

		fprintf(elf," Value:   %ld\n", elf64_sym.st_value);

	}
}
/*
int main(int argc, char *argv[])
{
	printf("hello\n");
	read_elf(argv[1], argv[2]);
}
*/

