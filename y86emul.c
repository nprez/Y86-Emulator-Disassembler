#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "y86emul.h"

struct flags{
	unsigned int OF:1;
	unsigned int ZF:1;
	unsigned int SF:1;
};

char fine = 1;

int hexToInt(char* s){
	int total = 0;
	int i;
	for(i=strlen(s)-1; i>=0; i--){
		if(isdigit(s[i])){
			total+=(s[i]-48)*((int)pow(16,strlen(s)-(i+1)));
		}
		else{
			switch(s[i]){
				case 'a':
				case 'A':{
					total+=10*((int)pow(16,strlen(s)-(i+1)));
					break;
				}
				case 'b':
				case 'B':{
					total+=11*((int)pow(16,strlen(s)-(i+1)));
					break;
				}
				case 'c':
				case 'C':{
					total+=12*((int)pow(16,strlen(s)-(i+1)));
					break;
				}
				case 'd':
				case 'D':{
					total+=13*((int)pow(16,strlen(s)-(i+1)));
					break;
				}
				case 'e':
				case 'E':{
					total+=14*((int)pow(16,strlen(s)-(i+1)));
					break;
				}
				case 'f':
				case 'F':{
					total+=15*((int)pow(16,strlen(s)-(i+1)));
					break;
				}
				default:{
					fine = 0;
					return 0;
				}
			}
		}
	}
	return total;
}

int main(int argc, char** argv){
	if(argc!=2){
		fprintf(stderr, "ERROR: Invalid parameters\n");
		return 1;
	}
	if(strlen(argv[1])==2 && argv[1][0] == '-' &&
	(argv[1][1] == 'h' || argv[1][1] == 'H')){
		printf("Useage: y86emul <y86 input file>\n");
		return 1;
	}
	FILE* input = fopen(argv[1], "r+");
	if(input==NULL){
		fprintf(stderr, "ERROR: Cannot open listed file\n");
		return 1;
	}
	
	unsigned int pc;
	unsigned int tStart;
	
	char* memory = (char*)malloc(sizeof(char));
	
	
	unsigned int size;
	
	struct flags f;
	f.OF=0;
	f.ZF=0;
	f.SF=0;
	
	int registers[8];
	//0 = eax
	//1 = ecx
	//2 = edx
	//3 = ebx
	//4 = esp
	//5 = ebp
	//6 = esi
	//7 = edi
	
	int i;
	for(i=0; i<8; i++)
		registers[i]=0;
	
	char foundSize = 0;
	char foundText = 0;
	
	//perform directives
	char line[1000];
	while(fgets(line, 1000, input)){
		char* arg1 = strtok(line, "\t");
		char* arg2 = strtok(NULL, "\t");
		char* arg3 = strtok(NULL, "\t");
		if(strtok(NULL, "\t")!=NULL){
			fprintf(stderr, "ERROR: Too many arguments for %s directive\n", arg1);
			free(memory);
			fclose(input);
			return 1;
		}
		while(arg1!=NULL && (arg1[strlen(arg1)-1]=='\n' || arg1[strlen(arg1)-1]=='\r'))
			arg1[strlen(arg1)-1]='\0';
		while(arg2!=NULL && (arg2[strlen(arg2)-1]=='\n' || arg2[strlen(arg2)-1]=='\r'))
			arg2[strlen(arg2)-1]='\0';
		while(arg3!=NULL && (arg3[strlen(arg3)-1]=='\n' || arg3[strlen(arg3)-1]=='\r'))
			arg3[strlen(arg3)-1]='\0';
		if(strcmp(arg1,".size")==0){
			foundSize++;
			if(arg2==NULL || arg3!=NULL){
				fprintf(stderr, "ERROR: Invalid arguments for .size directive\n");
				free(memory);
				fclose(input);
				return 1;
			}
			size = hexToInt(arg2);
			if(fine == 0){
				fprintf(stderr, "ERROR: Invalid hexadecimal number for .size directive\n");
				return 1;
			}
			memory=(char*)malloc(size*sizeof(char));
			registers[4]=size-1;
			registers[5]=size-1;
		}
		else if(strcmp(arg1,".string")==0){
			if(arg2==NULL || arg3==NULL || strlen(arg3)<2
			|| arg3[0]!='"' || arg3[strlen(arg3)-1]!='"'){
				fprintf(stderr, "ERROR: Invalid arguments for .string directive\n");
				free(memory);
				fclose(input);
				return 1;
			}
			int adr = hexToInt(arg2);
			if(fine == 0){
				fprintf(stderr, "ERROR: Invalid hexadecimal address for .string directive\n");
				return 1;
			}
			for(i = 1; i<strlen(arg3)-1; i++){
				memory[adr+(i-1)]=arg3[i];
			}
		}
		else if(strcmp(arg1,".long")==0){
			if(arg2==NULL || arg3==NULL){
				fprintf(stderr, "ERROR: Invalid arguments for .long directive\n");
				free(memory);
				fclose(input);
				return 1;
			}
			int adr = hexToInt(arg2);
			if(fine == 0){
				fprintf(stderr, "ERROR: Invalid hexadecimal address for .long directive\n");
				return 1;
			}
			int temp = atoi(arg3);
			memory[adr]=((unsigned int)temp)>>24;
			memory[adr+1]=(((unsigned int)temp)>>16)&(((unsigned int)(~0))>>8);
			memory[adr+2]=(((unsigned int)temp)>>8)&(((unsigned int)(~0))>>16);
			memory[adr+3]=temp&(((unsigned int)(~0))>>24);
		}
		else if(strcmp(arg1,".bss")==0){
			if(arg2==NULL || arg3==NULL){
				fprintf(stderr, "ERROR: Invalid arguments for .bss directive\n");
				free(memory);
				fclose(input);
				return 1;
			}
			int adr = hexToInt(arg2);
			if(fine == 0){
				fprintf(stderr, "ERROR: Invalid hexadecimal address for .bss directive\n");
				return 1;
			}
			int temp = atoi(arg3);
			for(i = 0; i<temp; i++){
				memory[adr+i]=0;
			}
		}
		else if(strcmp(arg1,".byte")==0){
			if(arg2==NULL || arg3==NULL){
				fprintf(stderr, "ERROR: Invalid arguments for .byte directive\n");
				free(memory);
				fclose(input);
				return 1;
			}
			int adr = hexToInt(arg2);
			if(fine == 0){
				fprintf(stderr, "ERROR: Invalid hexadecimal address for .byte directive\n");
				printf("Status Code: ADR\n");
				return 1;
			}
			int val = hexToInt(arg3);
			if(fine == 0){
				fprintf(stderr, "ERROR: Invalid hexadecimal number for .byte directive\n");
				return 1;
			}
			memory[adr]=val;
		}
		else if(strcmp(arg1,".text")==0){
			foundText++;
			if(arg2==NULL || arg3==NULL){
				fprintf(stderr, "ERROR: Invalid arguments for .text directive\n");
				free(memory);
				fclose(input);
				return 1;
			}
			int temp = hexToInt(arg2);
			if(fine == 0){
				fprintf(stderr, "ERROR: Invalid hexadecimal address for .text directive\n");
				return 1;
			}
			pc = temp;
			tStart = pc;
			for(i = 0; i<strlen(arg3); i++){
				memory[pc+i]=arg3[i];
			}
		}
		else{
			fprintf(stderr, "ERROR: Invalid directive given\n");
			free(memory);
			fclose(input);
			return 1;
		}
	}
	if(foundSize!=1 || foundText!=1){
		fprintf(stderr, "ERROR: Incorrect number of .size and/or .text directives\n");
		free(memory);
		fclose(input);
		return 1;
	}
	
	//execute instructions
	char keepGoing=1;
	while(keepGoing==1){
		switch(memory[pc]){
			//nop
			case '0':{
				pc++;
				if(memory[pc]!='0'){
					fprintf(stderr, "ERROR: Invalid second opcode for 0\n");
					printf("Status Code: INS\n");
					free(memory);
					fclose(input);
					return 1;
				}
				pc++;
				break;
			}
			//halt
			case '1':{
				pc++;
				if(memory[pc]!='0'){
					fprintf(stderr, "ERROR: Invalid second opcode for 1\n");
					printf("Status Code: INS\n");
					free(memory);
					fclose(input);
					return 1;
				}
				keepGoing=0;
				printf("Status Code: HLT\n");
				pc++;
				break;
			}
			//rrmovl
			case '2':{
				pc++;
				if(memory[pc]!='0'){
					fprintf(stderr, "ERROR: Invalid second opcode for 2\n");
					printf("Status Code: INS\n");
					free(memory);
					fclose(input);
					return 1;
				}
				pc++;
				int ra=memory[pc]-48;
				if(memory[pc]<'0' || memory[pc]>'7'){
					fprintf(stderr, "ERROR: Invalid first register for 2\n");
					printf("Status Code: INS\n");
					free(memory);
					fclose(input);
					return 1;
				}
				pc++;
				int rb=memory[pc]-48;
				if(memory[pc]<'0' || memory[pc]>'7'){
					fprintf(stderr, "ERROR: Invalid second register for 2\n");
					printf("Status Code: INS\n");
					free(memory);
					fclose(input);
					return 1;
				}
				registers[rb]=registers[ra];
				pc++;
				break;
			}
			//irmovl
			case '3':{
				pc++;
				if(memory[pc]!='0'){
					fprintf(stderr, "ERROR: Invalid second opcode for 3\n");
					printf("Status Code: INS\n");
					free(memory);
					fclose(input);
					return 1;
				}
				pc++;
				if(memory[pc]!='f' &&  memory[pc]!='F'){
					fprintf(stderr, "ERROR: Invalid register for 3 (no f listed)\n");
					printf("Status Code: INS\n");
					free(memory);
					fclose(input);
					return 1;
				}
				pc++;
				int r = memory[pc]-48;
				if(memory[pc]<'0' || memory[pc]>'7'){
					fprintf(stderr, "ERROR: Invalid register for 3\n");
					printf("Status Code: INS\n");
					free(memory);
					fclose(input);
					return 1;
				}
				char* correct = (char*)malloc(sizeof(char)*9);
				correct[8]='\0';
				pc++;
				for(i=0; i<4; i++){
					correct[(8-((2*i)+1))-1]=memory[pc];
					pc++;
					correct[(8-(2*i))-1]=memory[pc];
					pc++;
				}
				int temp = hexToInt(correct);
				free(correct);
				if(fine == 0){
					fprintf(stderr, "ERROR: Invalid hexadecimal number for 3\n");
					printf("Status Code: INS\n");
					return 1;
				}
				registers[r]=temp;
				break;
			}
			//rmmovl
			case '4':{
				pc++;
				if(memory[pc]!='0'){
					fprintf(stderr, "ERROR: Invalid second opcode for 4\n");
					printf("Status Code: INS\n");
					free(memory);
					fclose(input);
					return 1;
				}
				pc++;
				int ra = memory[pc]-48;
				if(memory[pc]<'0' || memory[pc]>'7'){
					fprintf(stderr, "ERROR: Invalid first register for 4\n");
					printf("Status Code: INS\n");
					free(memory);
					fclose(input);
					return 1;
				}
				pc++;
				int rb = memory[pc]-48;
				if(memory[pc]<'0' || memory[pc]>'7'){
					fprintf(stderr, "ERROR: Invalid second register for 4\n");
					printf("Status Code: INS\n");
					free(memory);
					fclose(input);
					return 1;
				}
				pc++;
				char* displacement = (char*)malloc(sizeof(char)*9);
				displacement[8]='\0';
				for(i=0; i<4; i++){
					displacement[(8-((2*i)+1))-1]=memory[pc];
					pc++;
					displacement[(8-(2*i))-1]=memory[pc];
					pc++;
				}
				int disp = hexToInt(displacement);
				free(displacement);
				if(fine == 0){
					fprintf(stderr, "ERROR: Invalid hexadecimal displacement for 4\n");
					printf("Status Code: ADR\n");
					return 1;
				}
				memory[registers[rb]+disp+3]=(unsigned char)(((unsigned int)registers[ra])>>24);				//first byte
				memory[registers[rb]+disp+2]=(unsigned char)((((unsigned int)registers[ra])>>16)&(((unsigned int)(~0))>>24));//second byte
				memory[registers[rb]+disp+1]=(unsigned char)((((unsigned int)registers[ra])>>8)&(((unsigned int)(~0))>>24));	//third byte
				memory[registers[rb]+disp]=(unsigned char)(((unsigned int)registers[ra])&(((unsigned int)(~0))>>24));		//last byte
				break;
			}
			//mrmovl
			case '5':{
				pc++;
				if(memory[pc]!='0'){
					fprintf(stderr, "ERROR: Invalid second opcode for 5\n");
					printf("Status Code: INS\n");
					free(memory);
					fclose(input);
					return 1;
				}
				pc++;
				int ra = memory[pc]-48;
				if(memory[pc]<'0' || memory[pc]>'7'){
					fprintf(stderr, "ERROR: Invalid first register for 5\n");
					printf("Status Code: INS\n");
					free(memory);
					fclose(input);
					return 1;
				}
				pc++;
				int rb = memory[pc]-48;
				if(memory[pc]<'0' || memory[pc]>'7'){
					fprintf(stderr, "ERROR: Invalid second register for 5\n");
					printf("Status Code: INS\n");
					free(memory);
					fclose(input);
					return 1;
				}
				pc++;
				char* displacement = (char*)malloc(sizeof(char)*9);
				displacement[8]='\0';
				for(i=0; i<4; i++){
					displacement[(8-((2*i)+1))-1]=memory[pc];
					pc++;
					displacement[(8-(2*i))-1]=memory[pc];
					pc++;
				}
				int disp = hexToInt(displacement);
				free(displacement);
				if(fine == 0){
					fprintf(stderr, "ERROR: Invalid hexadecimal displacement for 5\n");
					printf("Status Code: ADR\n");
					return 1;
				}
				registers[ra]=(unsigned char)memory[registers[rb]+disp];
				break;
			}
			//opX
			case '6':{
				pc++;
				switch(memory[pc]){
					//addl
					case '0':{
						pc++;
						int ra = memory[pc]-48;
						if(memory[pc]<'0' || memory[pc]>'7'){
							fprintf(stderr, "ERROR: Invalid first register for 60\n");
							printf("Status Code: INS\n");
							free(memory);
							fclose(input);
							return 1;
						}
						pc++;
						int rb = memory[pc]-48;
						if(memory[pc]<'0' || memory[pc]>'7'){
							fprintf(stderr, "ERROR: Invalid second register for 60\n");
							printf("Status Code: INS\n");
							free(memory);
							fclose(input);
							return 1;
						}
						if(registers[ra]>0 && registers[rb]>0){
							registers[rb]+=registers[ra];
							f.OF = (registers[rb]<0)?1:0;
						}
						else if(registers[ra]<0 && registers[rb]<0){
							registers[rb]+=registers[ra];
							f.OF = (registers[rb]>0)?1:0;
						}
						else{
							registers[rb]+=registers[ra];
							f.OF=0;
						}
						f.ZF = (registers[rb]==0)?1:0;
						f.SF = (registers[rb]<0)?1:0;
						pc++;
						break;
					}
					//subl
					case '1':{
						pc++;
						int ra = memory[pc]-48;
						if(memory[pc]<'0' || memory[pc]>'7'){
							fprintf(stderr, "ERROR: Invalid first register for 61\n");
							printf("Status Code: INS\n");
							free(memory);
							fclose(input);
							return 1;
						}
						pc++;
						int rb = memory[pc]-48;
						if(memory[pc]<'0' || memory[pc]>'7'){
							fprintf(stderr, "ERROR: Invalid second register for 61\n");
							printf("Status Code: INS\n");
							free(memory);
							fclose(input);
							return 1;
						}
						if(registers[ra]<0 && registers[rb]>0){
							registers[rb]-=registers[ra];
							f.OF = (registers[rb]<0)?1:0;
						}
						else if(registers[ra]>0 && registers[rb]<0){
							registers[rb]-=registers[ra];
							f.OF = (registers[rb]>0)?1:0;
						}
						else{
							registers[rb]-=registers[ra];
							f.OF=0;
						}
						f.ZF = (registers[rb]==0)?1:0;
						f.SF = (registers[rb]<0)?1:0;
						pc++;
						break;
					}
					//andl
					case '2':{
						pc++;
						int ra = memory[pc]-48;
						if(memory[pc]<'0' || memory[pc]>'7'){
							fprintf(stderr, "ERROR: Invalid first register for 62\n");
							printf("Status Code: INS\n");
							free(memory);
							fclose(input);
							return 1;
						}
						pc++;
						int rb = memory[pc]-48;
						if(memory[pc]<'0' || memory[pc]>'7'){
							fprintf(stderr, "ERROR: Invalid second register for 62\n");
							printf("Status Code: INS\n");
							free(memory);
							fclose(input);
							return 1;
						}
						registers[rb]&=registers[ra];
						f.OF = 0;
						f.ZF = (registers[rb]==0)?1:0;
						f.SF = (registers[rb]<0)?1:0;
						pc++;
						break;
					}
					//xorl
					case '3':{
						pc++;
						int ra = memory[pc]-48;
						if(memory[pc]<'0' || memory[pc]>'7'){
							fprintf(stderr, "ERROR: Invalid first register 63\n");
							printf("Status Code: INS\n");
							free(memory);
							fclose(input);
							return 1;
						}
						pc++;
						int rb = memory[pc]-48;
						if(memory[pc]<'0' || memory[pc]>'7'){
							fprintf(stderr, "ERROR: Invalid second register 63\n");
							printf("Status Code: INS\n");
							free(memory);
							fclose(input);
							return 1;
						}
						registers[rb]^=registers[ra];
						f.OF = 0;
						f.ZF = (registers[rb]==0)?1:0;
						f.SF = (registers[rb]<0)?1:0;
						pc++;
						break;
					}
					//mull
					case '4':{
						pc++;
						int ra = memory[pc]-48;
						if(memory[pc]<'0' || memory[pc]>'7'){
							fprintf(stderr, "ERROR: Invalid first register for 64\n");
							printf("Status Code: INS\n");
							free(memory);
							fclose(input);
							return 1;
						}
						pc++;
						int rb = memory[pc]-48;
						if(memory[pc]<'0' || memory[pc]>'7'){
							fprintf(stderr, "ERROR: Invalid second register for 64\n");
							printf("Status Code: INS\n");
							free(memory);
							fclose(input);
							return 1;
						}
						if((registers[ra]>0 && registers[rb]>0) ||
						(registers[ra]<0 && registers[rb]<0)){
							registers[rb]*=registers[ra];
							f.OF = (registers[rb]<0)?1:0;
						}
						else{
							registers[rb]*=registers[ra];
							f.OF = (registers[rb]>0)?1:0;
						}
						f.ZF = (registers[rb]==0)?1:0;
						f.SF = (registers[rb]<0)?1:0;
						pc++;
						break;
					}
					//cmpl
					case '5':{
						pc++;
						int ra = memory[pc]-48;
						if(memory[pc]<'0' || memory[pc]>'7'){
							fprintf(stderr, "ERROR: Invalid first register for 65\n");
							printf("Status Code: INS\n");
							free(memory);
							fclose(input);
							return 1;
						}
						pc++;
						int rb = memory[pc]-48;
						if(memory[pc]<'0' || memory[pc]>'7'){
							fprintf(stderr, "ERROR: Invalid second register for 65\n");
							printf("Status Code: INS\n");
							free(memory);
							fclose(input);
							return 1;
						}
						if(registers[ra]>0 && registers[rb]<0){
							f.OF = ((registers[rb]-registers[ra])>0)?1:0;
						}
						else if(registers[ra]<0 && registers[rb]>0){
							f.OF = ((registers[rb]-registers[ra])<0)?1:0;
						}
						else{
							f.OF=0;
						}
						f.ZF = ((registers[rb]-registers[ra])==0)?1:0;
						f.SF = ((registers[rb]-registers[ra])<0)?1:0;
						pc++;
						break;
					}
					default:{
						fprintf(stderr, "ERROR: Invalid second opcode for 6\n");
						printf("Status Code: INS\n");
						free(memory);
						fclose(input);
						return 1;
					}
				}
				break;
			}
			//jXX
			case '7':{
				pc++;
				switch(memory[pc]){
					//jmp
					case '0':{
						pc++;
						char* label = (char*)malloc(sizeof(char)*9);
						label[8]='\0';
						for(i=0; i<4; i++){
							label[(8-((2*i)+1))-1]=memory[pc];
							pc++;
							label[(8-(2*i))-1]=memory[pc];
							pc++;
						}
						int l = hexToInt(label);
						free(label);
						if(fine == 0){
							fprintf(stderr, "ERROR: Invalid hexadecimal address for 70\n");
							printf("Status Code: ADR\n");
							return 1;
						}
						pc=((l-tStart)*2)+tStart;
						break;
					}
					//jle
					case '1':{
						pc++;
						char* label = (char*)malloc(sizeof(char)*9);
						label[8]='\0';
						for(i=0; i<4; i++){
							label[(8-((2*i)+1))-1]=memory[pc];
							pc++;
							label[(8-(2*i))-1]=memory[pc];
							pc++;
						}
						if(((f.SF^f.OF)==1)||(f.ZF==1)){
							int l = hexToInt(label);
							free(label);
							if(fine == 0){
								fprintf(stderr, "ERROR: Invalid hexadecimal address for 71\n");
								printf("Status Code: ADR\n");
								return 1;
							}
							pc=((l-tStart)*2)+tStart;
						}
						else
							free(label);
						break;
					}
					//jl
					case '2':{
						char* label = (char*)malloc(sizeof(char)*9);
						label[8]='\0';
						pc++;
						for(i=0; i<4; i++){
							label[(8-((2*i)+1))-1]=memory[pc];
							pc++;
							label[(8-(2*i))-1]=memory[pc];
							pc++;
						}
						if((f.SF^f.OF)==1){
							int l = hexToInt(label);
							free(label);
							if(fine == 0){
								fprintf(stderr, "ERROR: Invalid hexadecimal address for 72\n");
								printf("Status Code: ADR\n");
								return 1;
							}
							pc=((l-tStart)*2)+tStart;
						}
						else
							free(label);
						break;
					}
					//je
					case '3':{
						pc++;
						char* label = (char*)malloc(sizeof(char)*9);
						label[8]='\0';
						for(i=0; i<4; i++){
							label[(8-((2*i)+1))-1]=memory[pc];
							pc++;
							label[(8-(2*i))-1]=memory[pc];
							pc++;
						}
						if(f.ZF==1){
							int l = hexToInt(label);
							free(label);
							if(fine == 0){
								fprintf(stderr, "ERROR: Invalid hexadecimal address for 73\n");
								printf("Status Code: ADR\n");
								return 1;
							}
							pc=((l-tStart)*2)+tStart;
						}
						else
							free(label);
						break;
					}
					//jne
					case '4':{
						pc++;
						char* label = (char*)malloc(sizeof(char)*9);
						label[8]='\0';
						for(i=0; i<4; i++){
							label[(8-((2*i)+1))-1]=memory[pc];
							pc++;
							label[(8-(2*i))-1]=memory[pc];
							pc++;
						}
						if(f.ZF==0){
							int l = hexToInt(label);
							free(label);
							if(fine == 0){
								fprintf(stderr, "ERROR: Invalid hexadecimal address for 74\n");
								printf("Status Code: ADR\n");
								return 1;
							}
							pc=((l-tStart)*2)+tStart;
						}
						else
							free(label);
						break;
					}
					//jge
					case '5':{
						pc++;
						char* label = (char*)malloc(sizeof(char)*9);
						label[8]='\0';
						for(i=0; i<4; i++){
							label[(8-((2*i)+1))-1]=memory[pc];
							pc++;
							label[(8-(2*i))-1]=memory[pc];
							pc++;
						}
						if((f.SF^f.OF)==0){
							int l = hexToInt(label);
							free(label);
							if(fine == 0){
								fprintf(stderr, "ERROR: Invalid hexadecimal address for 75\n");
								printf("Status Code: ADR\n");
								return 1;
							}
							pc=((l-tStart)*2)+tStart;
						}
						else
							free(label);
						break;
					}
					//jg
					case '6':{
						pc++;
						char* label = (char*)malloc(sizeof(char)*9);
						label[8]='\0';
						for(i=0; i<4; i++){
							label[(8-((2*i)+1))-1]=memory[pc];
							pc++;
							label[(8-(2*i))-1]=memory[pc];
							pc++;
						}
						if((f.SF^f.OF)==0 && f.ZF==0){
							int l = hexToInt(label);
							free(label);
							if(fine == 0){
								fprintf(stderr, "ERROR: Invalid hexadecimal address for 76\n");
								printf("Status Code: ADR\n");
								return 1;
							}
							pc=((l-tStart)*2)+tStart;
						}
						free(label);
						break;
					}
					default:{
						fprintf(stderr, "ERROR: Invalid second opcode for 7\n");
						printf("Status Code: INS\n");
						free(memory);
						fclose(input);
						return 1;
					}
				}
				break;
			}
			//call
			case '8':{
				pc++;
				if(memory[pc]!='0'){
					fprintf(stderr, "ERROR: Invalid second opcode for 8\n");
					printf("Status Code: INS\n");
					free(memory);
					fclose(input);
					return 1;
				}
				pc++;
				char* dest = (char*)malloc(sizeof(char)*9);
				dest[8]='\0';
				for(i=0; i<4; i++){
					dest[(8-((2*i)+1))-1]=memory[pc];
					pc++;
					dest[(8-(2*i))-1]=memory[pc];
					pc++;
				}
				//push ret address onto stack
				memory[registers[4]-1]=((unsigned int)pc)>>24;				//first byte
				memory[registers[4]-2]=(((unsigned int)pc)>>16)&(((unsigned int)(~0))>>24);	//second byte
				memory[registers[4]-3]=(((unsigned int)pc)>>8)&(((unsigned int)(~0))>>24);	//third byte
				memory[registers[4]-4]=pc&(((unsigned int)(~0))>>24);		//last byte
				registers[4]-=4;
				//jump to dest
				int d = hexToInt(dest);
				free(dest);
				if(fine == 0){
					fprintf(stderr, "ERROR: Invalid hexadecimal address for 80\n");
					printf("Status Code: ADR\n");
					return 1;
				}
				pc=((d-tStart)*2)+tStart;
				break;
			}
			//ret
			case '9':{
				pc++;
				if(memory[pc]!='0'){
					fprintf(stderr, "ERROR: Invalid second opcode for 9\n");
					printf("Status Code: INS\n");
					free(memory);
					fclose(input);
					return 1;
				}
				pc++;
				pc=(~0);
				//pop ret address into pc
				pc&=(((unsigned int)(~0)>>8)|((int)(unsigned char)memory[registers[4]+3])<<24); 					//first byte
				pc&=((((unsigned int)(~0)>>16)|((~0)<<24))|(((int)(unsigned char)memory[registers[4]+2])<<16));	//second byte
				pc&=((((unsigned int)(~0)>>24)|((~0)<<16))|(((int)(unsigned char)memory[registers[4]+1])<<8));	//third byte
				pc&=(((unsigned int)(~0)<<8)|((int)(unsigned char)memory[registers[4]]));							//fourth byte
				registers[4]+=4;
				break;
			}
			//pushl
			case 'a':
			case 'A':{
				pc++;
				if(memory[pc]!='0'){
					fprintf(stderr, "ERROR: Invalid second opcode for a\n");
					printf("Status Code: INS\n");
					free(memory);
					fclose(input);
					return 1;
				}
				pc++;
				int ra = memory[pc]-48;
				if(memory[pc]<'0' || memory[pc]>'7'){
					fprintf(stderr, "ERROR: Invalid register for a\n");
					printf("Status Code: INS\n");
					free(memory);
					fclose(input);
					return 1;
				}
				pc++;
				if(memory[pc]!='f' &&  memory[pc]!='F'){
					fprintf(stderr, "ERROR: Invalid register for a (no f listed)\n");
					printf("Status Code: INS\n");
					free(memory);
					fclose(input);
					return 1;
				}
				memory[registers[4]-1]=((unsigned int)registers[ra])>>24;				//first byte
				memory[registers[4]-2]=(((unsigned int)registers[ra])>>16)&(((unsigned int)(~0))>>24);	//second byte
				memory[registers[4]-3]=(((unsigned int)registers[ra])>>8)&(((unsigned int)(~0))>>24);	//third byte
				memory[registers[4]-4]=registers[ra]&(((unsigned int)(~0))>>24);		//last byte
				registers[4]-=4;
				pc++;
				break;
			}
			//popl
			case 'b':
			case 'B':{
				pc++;
				if(memory[pc]!='0'){
					fprintf(stderr, "ERROR: Invalid second opcode for b\n");
					printf("Status Code: INS\n");
					free(memory);
					fclose(input);
					return 1;
				}
				pc++;
				int ra = memory[pc]-48;
				if(memory[pc]<'0' || memory[pc]>'7'){
					fprintf(stderr, "ERROR: Invalid register for b\n");
					printf("Status Code: INS\n");
					free(memory);
					fclose(input);
					return 1;
				}
				pc++;
				if(memory[pc]!='f' &&  memory[pc]!='F'){
					fprintf(stderr, "ERROR: Invalid register for b (no f listed)\n");
					printf("Status Code: INS\n");
					free(memory);
					fclose(input);
					return 1;
				}
				registers[ra]=(~0);
				registers[ra]&=(((unsigned int)(~0)>>8)|((int)(unsigned char)memory[registers[4]+3])<<24);
				registers[ra]&=((((unsigned int)(~0)>>16)|((~0)<<24))|(((int)(unsigned char)memory[registers[4]+2])<<16));
				registers[ra]&=((((unsigned int)(~0)>>24)|((~0)<<16))|(((int)(unsigned char)memory[registers[4]+1])<<8));
				registers[ra]&=(((~0)<<8)|((int)(unsigned char)memory[registers[4]]));							//fourth byte
				registers[4]+=4;
				pc++;
				break;
			}
			//readX
			case 'c':
			case 'C':{
				pc++;
				switch(memory[pc]){
					//readb
					case '0':{
						pc++;
						int r = memory[pc]-48;
						if(memory[pc]<'0' || memory[pc]>'7'){
							fprintf(stderr, "ERROR: Invalid register for c0\n");
							printf("Status Code: INS\n");
							free(memory);
							fclose(input);
							return 1;
						}
						pc++;
						if(memory[pc]!='f' &&  memory[pc]!='F'){
							fprintf(stderr, "ERROR: Invalid register for c0 (no f listed)\n");
							printf("Status Code: INS\n");
							free(memory);
							fclose(input);
							return 1;
						}
						pc++;
						char* displacement = (char*)malloc(sizeof(char)*9);
						displacement[8]='\0';
						for(i=0; i<4; i++){
							displacement[(8-((2*i)+1))-1]=memory[pc];
							pc++;
							displacement[(8-(2*i))-1]=memory[pc];
							pc++;
						}
						int disp = hexToInt(displacement);
						free(displacement);
						if(fine == 0){
							fprintf(stderr, "ERROR: Invalid hexadecimal displacement for c0\n");
							printf("Status Code: ADR\n");
							return 1;
						}
						//read a single character, set ZF if unsuccessful
						f.ZF=(scanf("%c", &memory[registers[r]+disp])<0)?1:0;
						memory[registers[r]+disp]-=48;
						break;
					}
					//readl
					case '1':{
						pc++;
						int r = memory[pc]-48;
						if(memory[pc]<'0' || memory[pc]>'7'){
							fprintf(stderr, "ERROR: Invalid register for c1\n");
							printf("Status Code: INS\n");
							free(memory);
							fclose(input);
							return 1;
						}
						pc++;
						if(memory[pc]!='f' &&  memory[pc]!='F'){
							fprintf(stderr, "ERROR: Invalid register for c1 (no f listed)\n");
							printf("Status Code: INS\n");
							free(memory);
							fclose(input);
							return 1;
						}
						pc++;
						char* displacement = (char*)malloc(sizeof(char)*9);
						displacement[8]='\0';
						for(i=0; i<4; i++){
							displacement[(8-((2*i)+1))-1]=memory[pc];
							pc++;
							displacement[(8-(2*i))-1]=memory[pc];
							pc++;
						}
						int disp = hexToInt(displacement);
						free(displacement);
						if(fine == 0){
							fprintf(stderr, "ERROR: Invalid hexadecimal displacement for c1\n");
							printf("Status Code: ADR\n");
							return 1;
						}
						//read 4 characters, set ZF if unsuccessful
						f.ZF=(scanf("%c%c%c%c",
						&memory[registers[r]+disp],
						&memory[registers[r]+disp+1],
						&memory[registers[r]+disp+2],
						&memory[registers[r]+disp+3])<0)?1:0;
						memory[registers[r]+disp]-=48;
						memory[registers[r]+disp+1]-=48;
						memory[registers[r]+disp+2]-=48;
						memory[registers[r]+disp+3]-=48;
						if(memory[registers[r]+disp+1]>=0){
							memory[registers[r]+disp]*=10;
							memory[registers[r]+disp]+=memory[registers[r]+disp+1];
						}
						break;
					}
					default:{
						fprintf(stderr, "ERROR: Invalid second opcode for c\n");
						printf("Status Code: INS\n");
						free(memory);
						fclose(input);
						return 1;
					}
				}
				break;
			}
			//writeX
			case 'd':
			case 'D':{
				pc++;
				switch(memory[pc]){
					//writeb
					case '0':{
						pc++;
						int r = memory[pc]-48;
						if(memory[pc]<'0' || memory[pc]>'7'){
							fprintf(stderr, "ERROR: Invalid register for d0\n");
							printf("Status Code: INS\n");
							free(memory);
							fclose(input);
							return 1;
						}
						pc++;
						if(memory[pc]!='f' &&  memory[pc]!='F'){
							fprintf(stderr, "ERROR: Invalid register for d0 (no f listed)\n");
							printf("Status Code: INS\n");
							free(memory);
							fclose(input);
							return 1;
						}
						pc++;
						char* displacement = (char*)malloc(sizeof(char)*9);
						displacement[8]='\0';
						for(i=0; i<4; i++){
							displacement[(8-((2*i)+1))-1]=memory[pc];
							pc++;
							displacement[(8-(2*i))-1]=memory[pc];
							pc++;
						}
						int disp = hexToInt(displacement);
						free(displacement);
						if(fine == 0){
							fprintf(stderr, "ERROR: Invalid hexadecimal displacement for d0\n");
							printf("Status Code: ADR\n");
							return 1;
						}
						//write a single character
						printf("%c", memory[registers[r]+disp]);
						break;
					}
					//writel
					case '1':{
						pc++;
						int r = memory[pc]-48;
						if(memory[pc]<'0' || memory[pc]>'7'){
							fprintf(stderr, "ERROR: Invalid register for d1\n");
							printf("Status Code: INS\n");
							free(memory);
							fclose(input);
							return 1;
						}
						pc++;
						if(memory[pc]!='f' &&  memory[pc]!='F'){
							fprintf(stderr, "ERROR: Invalid register for d1 (no f listed)\n");
							printf("Status Code: INS\n");
							free(memory);
							fclose(input);
							return 1;
						}
						pc++;
						char* displacement = (char*)malloc(sizeof(char)*9);
						displacement[8]='\0';
						for(i=0; i<4; i++){
							displacement[(8-((2*i)+1))-1]=memory[pc];
							pc++;
							displacement[(8-(2*i))-1]=memory[pc];
							pc++;
						}
						int disp = hexToInt(displacement);
						free(displacement);
						if(fine == 0){
							fprintf(stderr, "ERROR: Invalid hexadecimal displacement for d1\n");
							printf("Status Code: ADR\n");
							return 1;
						}
						printf("%d",(((unsigned int)(unsigned char)(memory[registers[r]+disp]))+
						(((unsigned int)(unsigned char)(memory[registers[r]+disp+1])))*16*16+
						(((unsigned int)(unsigned char)(memory[registers[r]+disp+2])))*16*16*16*16+
						(((unsigned int)(unsigned char)(memory[registers[r]+disp+3])))*16*16*16*16*16*16));
						break;
					}
					default:{
						fprintf(stderr, "ERROR: Invalid second opcode for d\n");
						printf("Status Code: INS\n");
						free(memory);
						fclose(input);
						return 1;
					}
				}
				break;
			}
			//movsbl
			case 'e':
			case 'E':{
				pc++;
				if(memory[pc]!='0'){
					fprintf(stderr, "ERROR: Invalid second opcode for e\n");
					printf("Status Code: INS\n");
					free(memory);
					fclose(input);
					return 1;
				}
				pc++;
				int ra = memory[pc]-48;
				if(memory[pc]<'0' || memory[pc]>'7'){
					fprintf(stderr, "ERROR: Invalid first register for e0\n");
					printf("Status Code: INS\n");
					free(memory);
					fclose(input);
					return 1;
				}
				pc++;
				int rb = memory[pc]-48;
				if(memory[pc]<'0' || memory[pc]>'7'){
					fprintf(stderr, "ERROR: Invalid second register for e0\n");
					printf("Status Code: INS\n");
					free(memory);
					fclose(input);
					return 1;
				}
				pc++;
				char* displacement = (char*)malloc(sizeof(char)*9);
				displacement[8]='\0';
				for(i=0; i<4; i++){
					displacement[(8-((2*i)+1))-1]=memory[pc];
					pc++;
					displacement[(8-(2*i))-1]=memory[pc];
					pc++;
				}
				int disp = hexToInt(displacement);
				free(displacement);
				if(fine == 0){
					fprintf(stderr, "ERROR: Invalid hexadecimal displacement for e0\n");
					printf("Status Code: ADR\n");
					return 1;
				}
				registers[ra]=memory[registers[rb]+disp];
				break;
			}
			default:{
				fprintf(stderr, "ERROR: Invalid first opcode\n");
				printf("Status Code: INS\n");
				free(memory);
				fclose(input);
				return 1;
			}
		}
	}
	
	free(memory);
	fclose(input);
	return 0;
}