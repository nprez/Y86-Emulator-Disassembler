#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "y86dis.h"

char fine;

char* getReg(int x){
	switch(x){
		case 0:{
			return "%EAX";
			break;
		}
		case 1:{
			return "%ECX";
			break;
		}
		case 2:{
			return "%EDX";
			break;
		}
		case 3:{
			return "%EBX";
			break;
		}
		case 4:{
			return "%ESP";
			break;
		}
		case 5:{
			return "%EBP";
			break;
		}
		case 6:{
			return "%ESI";
			break;
		}
		case 7:{
			return "%EDI";
			break;
		}
		default:{
			return "GOOFED";
		}
	}
}

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

char* intToHex(int num){
	char* output = (char*)malloc(sizeof(char));
	int i = 0;
	while(1){
		if(strlen(output)<(i+1))
			output = (char*)realloc(output, sizeof(output)+sizeof(char));
		if((num%16)<10)
			output[i]=(char)((num%16)+48);
		else{
			switch(num%16){
				case 10:{
					output[i]='A';
					break;
				}
				case 11:{
					output[i]='B';
					break;
				}
				case 12:{
					output[i]='C';
					break;
				}
				case 13:{
					output[i]='D';
					break;
				}
				case 14:{
					output[i]='E';
					break;
				}
				case 15:{
					output[i]='F';
					break;
				}
			}
		}
		num/=16;
		if(num==0)
			break;
		i++;
	}
	output = (char*)realloc(output, sizeof(output)+sizeof(char));
	output[i+1] = '\0';
	//reverse it
	int j = strlen(output)-1;
	for(i = 0; i<j; i++, j--){
		char temp = output[i];
		output[i]=output[j];
		output[j]=temp;
	}
	return output;
}

int main(int argc, char** argv){
	fine = 1;
	if(argc!=2){
		fprintf(stderr, "ERROR: Invalid parameters\n");
		return 1;
	}
	if(strlen(argv[1])==2 && argv[1][0] == '-' &&
	(argv[1][1] == 'h' || argv[1][1] == 'H')){
		printf("Useage: y86dis <y86 input file>\n");
		return 1;
	}
	FILE* input = fopen(argv[1], "r+");
	if(input==NULL){
		fprintf(stderr, "ERROR: Cannot open listed file\n");
		return 1;
	}
	
	char* text=(char*)malloc(sizeof(char));
	char found = 0;
	
	int i;
	unsigned int pc=0;
	unsigned int start;
	
	char line[9999];
	while(fgets(line, 9999, input)){
		char* arg1 = strtok(line, "\t");
		char* arg2 = strtok(NULL, "\t");
		char* arg3 = strtok(NULL, "\t");
		if(strtok(NULL, "\t")!=NULL){
			fprintf(stderr, "ERROR: Too many arguments for %s directive\n", arg1);
			free(text);
			fclose(input);
			return 1;
		}
		while(arg1!=NULL && (arg1[strlen(arg1)-1]=='\n' || arg1[strlen(arg1)-1]=='\r'))
			arg1[strlen(arg1)-1]='\0';
		while(arg2!=NULL && (arg2[strlen(arg2)-1]=='\n' || arg2[strlen(arg2)-1]=='\r'))
			arg2[strlen(arg2)-1]='\0';
		while(arg3!=NULL && (arg3[strlen(arg3)-1]=='\n' || arg3[strlen(arg3)-1]=='\r'))
			arg3[strlen(arg3)-1]='\0';
		if(strcmp(arg1, ".text")==0){
			text=(char*)malloc(sizeof(char)*(strlen(arg3)+1));
			for(i=0; i<strlen(arg3); i++){
				text[i]=arg3[i];
			}
			start=hexToInt(arg2);
			text[strlen(arg3)]='\0';
			found++;
		}
	}
	
	if(found!=1){
		fprintf(stderr, "ERROR: Incorrect number of .text directives\n");
		free(text);
		fclose(input);
		return 1;
	}
	
	while(pc<strlen(text)){
		switch(text[pc]){
			//nop
			case '0':{
				char* insadr = intToHex(start+pc);
				char op[3];
				for(i=0; i<2; i++)
					op[i]=text[pc+i];
				op[2]='\0';
				pc++;
				if(text[pc]!='0'){
					fprintf(stderr, "ERROR: Invalid second opcode for 0\n");
					free(text);
					fclose(input);
					return 1;
				}
				pc++;
				printf("0x%s\tNOP\t\t\t%s\n",insadr,op);
				break;
			}
			//halt
			case '1':{
				char* insadr = intToHex(start+pc);
				char op[3];
				for(i=0; i<2; i++)
					op[i]=text[pc+i];
				op[2]='\0';
				pc++;
				if(text[pc]!='0'){
					fprintf(stderr, "ERROR: Invalid second opcode for 1\n");
					free(text);
					fclose(input);
					return 1;
				}
				pc++;
				printf("0x%s\tHLT\t\t\t%s\n",insadr,op);
				break;
			}
			//rrmovl
			case '2':{
				char* insadr = intToHex(start+pc);
				char op[5];
				for(i=0; i<4; i++)
					op[i]=text[pc+i];
				op[4]='\0';
				pc++;
				if(text[pc]!='0'){
					fprintf(stderr, "ERROR: Invalid second opcode for 2\n");
					free(text);
					fclose(input);
					return 1;
				}
				pc++;
				int ra=text[pc]-48;
				if(text[pc]<'0' || text[pc]>'7'){
					fprintf(stderr, "ERROR: Invalid first register for 2\n");
					free(text);
					fclose(input);
					return 1;
				}
				pc++;
				int rb=text[pc]-48;
				if(text[pc]<'0' || text[pc]>'7'){
					fprintf(stderr, "ERROR: Invalid second register for 2\n");
					free(text);
					fclose(input);
					return 1;
				}
				printf("0x%s\tMOVL\t%s,%s\t%s\n",insadr,getReg(ra), getReg(rb),op);
				pc++;
				break;
			}
			//irmovl
			case '3':{
				char* insadr = intToHex(start+pc);
				char op[13];
				for(i=0; i<12; i++)
					op[i]=text[pc+i];
				op[12]='\0';
				pc++;
				if(text[pc]!='0'){
					fprintf(stderr, "ERROR: Invalid second opcode for 3\n");
					free(text);
					fclose(input);
					return 1;
				}
				pc++;
				if(text[pc]!='f' &&  text[pc]!='F'){
					fprintf(stderr, "ERROR: Invalid register for 3 (no f listed)\n");
					free(text);
					fclose(input);
					return 1;
				}
				pc++;
				int r = text[pc]-48;
				if(text[pc]<'0' || text[pc]>'7'){
					fprintf(stderr, "ERROR: Invalid register for 3\n");
					free(text);
					fclose(input);
					return 1;
				}
				char* correct = (char*)malloc(sizeof(char)*9);
				correct[8]='\0';
				pc++;
				for(i=0; i<4; i++){
					correct[(8-((2*i)+1))-1]=text[pc];
					pc++;
					correct[(8-(2*i))-1]=text[pc];
					pc++;
				}
				int temp = hexToInt(correct);
				free(correct);
				if(fine == 0){
					fprintf(stderr, "ERROR: Invalid hexadecimal number for 3\n");
					free(text);
					fclose(input);
					return 1;
				}
				if((temp/10)>0)
					printf("0x%s\tMOVL\t$%d,%s\t%s\n",insadr,temp,getReg(r),op);
				else
					printf("0x%s\tMOVL\t$%d,%s\t\t%s\n",insadr,temp,getReg(r),op);
				break;
			}
			//rmmovl
			case '4':{
				char* insadr = intToHex(start+pc);
				char op[13];
				for(i=0; i<12; i++)
					op[i]=text[pc+i];
				op[12]='\0';
				pc++;
				if(text[pc]!='0'){
					fprintf(stderr, "ERROR: Invalid second opcode for 4\n");
					free(text);
					fclose(input);
					return 1;
				}
				pc++;
				int ra = text[pc]-48;
				if(text[pc]<'0' || text[pc]>'7'){
					fprintf(stderr, "ERROR: Invalid first register for 4\n");
					free(text);
					fclose(input);
					return 1;
				}
				pc++;
				int rb = text[pc]-48;
				if(text[pc]<'0' || text[pc]>'7'){
					fprintf(stderr, "ERROR: Invalid second register for 4\n");
					free(text);
					fclose(input);
					return 1;
				}
				pc++;
				char* displacement = (char*)malloc(sizeof(char)*9);
				displacement[8]='\0';
				for(i=0; i<4; i++){
					displacement[(8-((2*i)+1))-1]=text[pc];
					pc++;
					displacement[(8-(2*i))-1]=text[pc];
					pc++;
				}
				int disp = hexToInt(displacement);
				free(displacement);
				if(fine == 0){
					fprintf(stderr, "ERROR: Invalid hexadecimal displacement for 4\n");
					free(text);
					fclose(input);
					return 1;
				}
				printf("0x%s\tMOVL\t%s,%d(%s)\t%s\n",insadr,getReg(ra),disp,getReg(rb),op);
				break;
			}
			//mrmovl
			case '5':{
				char* insadr = intToHex(start+pc);
				char op[13];
				for(i=0; i<12; i++)
					op[i]=text[pc+i];
				op[12]='\0';
				pc++;
				if(text[pc]!='0'){
					fprintf(stderr, "ERROR: Invalid second opcode for 5\n");
					free(text);
					fclose(input);
					return 1;
				}
				pc++;
				int ra = text[pc]-48;
				if(text[pc]<'0' || text[pc]>'7'){
					fprintf(stderr, "ERROR: Invalid first register for 5\n");
					free(text);
					fclose(input);
					return 1;
				}
				pc++;
				int rb = text[pc]-48;
				if(text[pc]<'0' || text[pc]>'7'){
					fprintf(stderr, "ERROR: Invalid second register for 5\n");
					free(text);
					fclose(input);
					return 1;
				}
				pc++;
				char* displacement = (char*)malloc(sizeof(char)*9);
				displacement[8]='\0';
				for(i=0; i<4; i++){
					displacement[(8-((2*i)+1))-1]=text[pc];
					pc++;
					displacement[(8-(2*i))-1]=text[pc];
					pc++;
				}
				int disp = hexToInt(displacement);
				free(displacement);
				if(fine == 0){
					fprintf(stderr, "ERROR: Invalid hexadecimal displacement for 5\n");
					free(text);
					fclose(input);
					return 1;
				}
				printf("0x%s\tMOVL\t%d(%s),%s\t%s\n",insadr,disp,getReg(rb),getReg(ra),op);
				break;
			}
			//opX
			case '6':{
				char op[5];
				for(i=0; i<4; i++)
					op[i]=text[pc+i];
				op[4]='\0';
				char* insadr = intToHex(start+pc);
				pc++;
				switch(text[pc]){
					//addl
					case '0':{
						pc++;
						int ra = text[pc]-48;
						if(text[pc]<'0' || text[pc]>'7'){
							fprintf(stderr, "ERROR: Invalid first register for 60\n");
							free(text);
							fclose(input);
							return 1;
						}
						pc++;
						int rb = text[pc]-48;
						if(text[pc]<'0' || text[pc]>'7'){
							fprintf(stderr, "ERROR: Invalid second register for 60\n");
							free(text);
							fclose(input);
							return 1;
						}
						pc++;
						printf("0x%s\tADDL\t%s,%s\t%s\n",insadr,getReg(ra),getReg(rb),op);
						break;
					}
					//subl
					case '1':{
						pc++;
						int ra = text[pc]-48;
						if(text[pc]<'0' || text[pc]>'7'){
							fprintf(stderr, "ERROR: Invalid first register for 61\n");
							free(text);
							fclose(input);
							return 1;
						}
						pc++;
						int rb = text[pc]-48;
						if(text[pc]<'0' || text[pc]>'7'){
							fprintf(stderr, "ERROR: Invalid second register for 61\n");
							free(text);
							fclose(input);
							return 1;
						}
						pc++;
						printf("0x%s\tSUBL\t%s,%s\t%s\n",insadr,getReg(ra),getReg(rb),op);
						break;
					}
					//andl
					case '2':{
						pc++;
						int ra = text[pc]-48;
						if(text[pc]<'0' || text[pc]>'7'){
							fprintf(stderr, "ERROR: Invalid first register for 62\n");
							free(text);
							fclose(input);
							return 1;
						}
						pc++;
						int rb = text[pc]-48;
						if(text[pc]<'0' || text[pc]>'7'){
							fprintf(stderr, "ERROR: Invalid second register for 62\n");
							free(text);
							fclose(input);
							return 1;
						}
						pc++;
						printf("0x%s\tANDL\t%s,%s\t%s\n",insadr,getReg(ra),getReg(rb),op);
						break;
					}
					//xorl
					case '3':{
						pc++;
						int ra = text[pc]-48;
						if(text[pc]<'0' || text[pc]>'7'){
							fprintf(stderr, "ERROR: Invalid first register 63\n");
							free(text);
							fclose(input);
							return 1;
						}
						pc++;
						int rb = text[pc]-48;
						if(text[pc]<'0' || text[pc]>'7'){
							fprintf(stderr, "ERROR: Invalid second register 63\n");
							free(text);
							fclose(input);
							return 1;
						}
						pc++;
						printf("0x%s\tXORL\t%s,%s\t%s\n",insadr,getReg(ra),getReg(rb),op);
						break;
					}
					//mull
					case '4':{
						pc++;
						int ra = text[pc]-48;
						if(text[pc]<'0' || text[pc]>'7'){
							fprintf(stderr, "ERROR: Invalid first register for 64\n");
							free(text);
							fclose(input);
							return 1;
						}
						pc++;
						int rb = text[pc]-48;
						if(text[pc]<'0' || text[pc]>'7'){
							fprintf(stderr, "ERROR: Invalid second register for 64\n");
							free(text);
							fclose(input);
							return 1;
						}
						pc++;
						printf("0x%s\tIMULL\t%s,%s\t%s\n",insadr,getReg(ra),getReg(rb),op);
						break;
					}
					//cmpl
					case '5':{
						pc++;
						int ra = text[pc]-48;
						if(text[pc]<'0' || text[pc]>'7'){
							fprintf(stderr, "ERROR: Invalid first register for 65\n");
							free(text);
							fclose(input);
							return 1;
						}
						pc++;
						int rb = text[pc]-48;
						if(text[pc]<'0' || text[pc]>'7'){
							fprintf(stderr, "ERROR: Invalid second register for 65\n");
							free(text);
							fclose(input);
							return 1;
						}
						pc++;
						printf("0x%s\tCMPL\t%s,%s\t%s\n",insadr,getReg(ra),getReg(rb),op);
						break;
					}
					default:{
						fprintf(stderr, "ERROR: Invalid second opcode for 6\n");
						free(text);
						fclose(input);
						return 1;
					}
				}
				break;
			}
			//jXX
			case '7':{
				char op[11];
				for(i=0; i<10; i++)
					op[i]=text[pc+i];
				op[10]='\0';
				char* insadr = intToHex(start+pc);
				pc++;
				switch(text[pc]){
					//jmp
					case '0':{
						pc++;
						char* label = (char*)malloc(sizeof(char)*9);
						label[8]='\0';
						for(i=0; i<4; i++){
							label[(8-((2*i)+1))-1]=text[pc];
							pc++;
							label[(8-(2*i))-1]=text[pc];
							pc++;
						}
						hexToInt(label);
						if(fine == 0){
							fprintf(stderr, "ERROR: Invalid hexadecimal address for 70\n");
							free(text);
							fclose(input);
							return 1;
						}
						printf("0x%s\tJMP\t0x%s\t%s\n",insadr,label,op);
						free(label);
						break;
					}
					//jle
					case '1':{
						pc++;
						char* label = (char*)malloc(sizeof(char)*9);
						label[8]='\0';
						for(i=0; i<4; i++){
							label[(8-((2*i)+1))-1]=text[pc];
							pc++;
							label[(8-(2*i))-1]=text[pc];
							pc++;
						}
						hexToInt(label);
						if(fine == 0){
							fprintf(stderr, "ERROR: Invalid hexadecimal address for 70\n");
							free(text);
							fclose(input);
							return 1;
						}
						printf("0x%s\tJLE\t0x%s\t%s\n",insadr,label,op);
						free(label);
						break;
					}
					//jl
					case '2':{
						pc++;
						char* label = (char*)malloc(sizeof(char)*9);
						label[8]='\0';
						for(i=0; i<4; i++){
							label[(8-((2*i)+1))-1]=text[pc];
							pc++;
							label[(8-(2*i))-1]=text[pc];
							pc++;
						}
						hexToInt(label);
						if(fine == 0){
							fprintf(stderr, "ERROR: Invalid hexadecimal address for 70\n");
							free(text);
							fclose(input);
							return 1;
						}
						printf("0x%s\tJL\t0x%s\t%s\n",insadr,label,op);
						free(label);
						break;
					}
					//je
					case '3':{
						pc++;
						char* label = (char*)malloc(sizeof(char)*9);
						label[8]='\0';
						for(i=0; i<4; i++){
							label[(8-((2*i)+1))-1]=text[pc];
							pc++;
							label[(8-(2*i))-1]=text[pc];
							pc++;
						}
						hexToInt(label);
						if(fine == 0){
							fprintf(stderr, "ERROR: Invalid hexadecimal address for 70\n");
							free(text);
							fclose(input);
							return 1;
						}
						printf("0x%s\tJE\t0x%s\t%s\n",insadr,label,op);
						free(label);
						break;
					}
					//jne
					case '4':{
						pc++;
						char* label = (char*)malloc(sizeof(char)*9);
						label[8]='\0';
						for(i=0; i<4; i++){
							label[(8-((2*i)+1))-1]=text[pc];
							pc++;
							label[(8-(2*i))-1]=text[pc];
							pc++;
						}
						hexToInt(label);
						if(fine == 0){
							fprintf(stderr, "ERROR: Invalid hexadecimal address for 70\n");
							free(text);
							fclose(input);
							return 1;
						}
						printf("0x%s\tJNE\t0x%s\t%s\n",insadr,label,op);
						free(label);
						break;
					}
					//jge
					case '5':{
						pc++;
						char* label = (char*)malloc(sizeof(char)*9);
						label[8]='\0';
						for(i=0; i<4; i++){
							label[(8-((2*i)+1))-1]=text[pc];
							pc++;
							label[(8-(2*i))-1]=text[pc];
							pc++;
						}
						hexToInt(label);
						if(fine == 0){
							fprintf(stderr, "ERROR: Invalid hexadecimal address for 70\n");
							free(text);
							fclose(input);
							return 1;
						}
						printf("0x%s\tJGE\t0x%s\t%s\n",insadr,label,op);
						free(label);
						break;
					}
					//jg
					case '6':{
						pc++;
						char* label = (char*)malloc(sizeof(char)*9);
						label[8]='\0';
						for(i=0; i<4; i++){
							label[(8-((2*i)+1))-1]=text[pc];
							pc++;
							label[(8-(2*i))-1]=text[pc];
							pc++;
						}
						hexToInt(label);
						if(fine == 0){
							fprintf(stderr, "ERROR: Invalid hexadecimal address for 70\n");
							free(text);
							fclose(input);
							return 1;
						}
						printf("0x%s\tJG\t0x%s\t%s\n",insadr,label,op);
						free(label);
						break;
					}
					default:{
						fprintf(stderr, "ERROR: Invalid second opcode for 7\n");
						free(text);
						fclose(input);
						return 1;
					}
				}
				break;
			}
			//call
			case '8':{
				char op[11];
				for(i=0; i<10; i++)
					op[i]=text[pc+i];
				op[10]='\0';
				char* insadr = intToHex(start+pc);
				pc++;
				if(text[pc]!='0'){
					fprintf(stderr, "ERROR: Invalid second opcode for 8\n");
					free(text);
					fclose(input);
					return 1;
				}
				pc++;
				char* dest = (char*)malloc(sizeof(char)*9);
				dest[8]='\0';
				for(i=0; i<4; i++){
					dest[(8-((2*i)+1))-1]=text[pc];
					pc++;
					dest[(8-(2*i))-1]=text[pc];
					pc++;
				}
				free(dest);
				if(fine == 0){
					fprintf(stderr, "ERROR: Invalid hexadecimal address for 80\n");
					free(text);
					fclose(input);
					return 1;
				}
				printf("0x%s\tCALL\t0x%s\t\t%s\n",insadr, dest,op);
				break;
			}
			//ret
			case '9':{
				char op[3];
				for(i=0; i<2; i++)
					op[i]=text[pc+i];
				op[2]='\0';
				char* insadr = intToHex(start+pc);
				pc++;
				if(text[pc]!='0'){
					fprintf(stderr, "ERROR: Invalid second opcode for 9\n");
					free(text);
					fclose(input);
					return 1;
				}
				pc++;
				printf("0x%s\tRET\t\t\t%s\n",insadr,op);
				break;
			}
			//pushl
			case 'a':
			case 'A':{
				char op[5];
				for(i=0; i<4; i++)
					op[i]=text[pc+i];
				op[4]='\0';
				char* insadr = intToHex(start+pc);
				pc++;
				if(text[pc]!='0'){
					fprintf(stderr, "ERROR: Invalid second opcode for a\n");
					free(text);
					fclose(input);
					return 1;
				}
				pc++;
				int ra = text[pc]-48;
				if(text[pc]<'0' || text[pc]>'7'){
					fprintf(stderr, "ERROR: Invalid register for a\n");
					free(text);
					fclose(input);
					return 1;
				}
				pc++;
				if(text[pc]!='f' &&  text[pc]!='F'){
					fprintf(stderr, "ERROR: Invalid register for a (no f listed)\n");
					free(text);
					fclose(input);
					return 1;
				}
				pc++;
				printf("0x%s\tPUSHL\t%s\t\t%s\n", insadr,getReg(ra),op);
				break;
			}
			//popl
			case 'b':
			case 'B':{
				char* insadr = intToHex(start+pc);
				char op[5];
				for(i=0; i<4; i++)
					op[i]=text[pc+i];
				op[4]='\0';
				pc++;
				if(text[pc]!='0'){
					fprintf(stderr, "ERROR: Invalid second opcode for b\n");
					free(text);
					fclose(input);
					return 1;
				}
				pc++;
				int ra = text[pc]-48;
				if(text[pc]<'0' || text[pc]>'7'){
					fprintf(stderr, "ERROR: Invalid register for b\n");
					free(text);
					fclose(input);
					return 1;
				}
				pc++;
				if(text[pc]!='f' &&  text[pc]!='F'){
					fprintf(stderr, "ERROR: Invalid register for b (no f listed)\n");
					free(text);
					fclose(input);
					return 1;
				}
				pc++;
				printf("0x%s\tPOPL\t%s\t\t%s\n", insadr,getReg(ra),op);
				break;
			}
			//readX
			case 'c':
			case 'C':{
				char op[13];
				for(i=0; i<12; i++)
					op[i]=text[pc+i];
				op[12]='\0';
				char* insadr = intToHex(start+pc);
				pc++;
				switch(text[pc]){
					//readb
					case '0':{
						pc++;
						if(text[pc]<'0' || text[pc]>'7'){
							fprintf(stderr, "ERROR: Invalid register for c0\n");
							free(text);
							fclose(input);
							return 1;
						}
						pc++;
						if(text[pc]!='f' &&  text[pc]!='F'){
							fprintf(stderr, "ERROR: Invalid register for c0 (no f listed)\n");
							free(text);
							fclose(input);
							return 1;
						}
						pc++;
						char* displacement = (char*)malloc(sizeof(char)*9);
						displacement[8]='\0';
						for(i=0; i<4; i++){
							displacement[(8-((2*i)+1))-1]=text[pc];
							pc++;
							displacement[(8-(2*i))-1]=text[pc];
							pc++;
						}
						free(displacement);
						if(fine == 0){
							fprintf(stderr, "ERROR: Invalid hexadecimal displacement for c0\n");
							free(text);
							fclose(input);
							return 1;
						}
						printf("0x%s\tWHATEVER READB IS\t%s\n", insadr,op);
						break;
					}
					//readl
					case '1':{
						pc++;
						if(text[pc]<'0' || text[pc]>'7'){
							fprintf(stderr, "ERROR: Invalid register for c1\n");
							free(text);
							fclose(input);
							return 1;
						}
						pc++;
						if(text[pc]!='f' &&  text[pc]!='F'){
							fprintf(stderr, "ERROR: Invalid register for c1 (no f listed)\n");
							free(text);
							fclose(input);
							return 1;
						}
						pc++;
						char* displacement = (char*)malloc(sizeof(char)*9);
						displacement[8]='\0';
						for(i=0; i<4; i++){
							displacement[(8-((2*i)+1))-1]=text[pc];
							pc++;
							displacement[(8-(2*i))-1]=text[pc];
							pc++;
						}
						free(displacement);
						if(fine == 0){
							fprintf(stderr, "ERROR: Invalid hexadecimal displacement for c1\n");
							free(text);
							fclose(input);
							return 1;
						}
						printf("0x%s\tWHATEVER READL IS\t%s\n", insadr,op);
						break;
					}
					default:{
						fprintf(stderr, "ERROR: Invalid second opcode for c\n");
						free(text);
						fclose(input);
						return 1;
					}
				}
				break;
			}
			//writeX
			case 'd':
			case 'D':{
				char op[13];
				for(i=0; i<12; i++)
					op[i]=text[pc+i];
				op[12]='\0';
				char* insadr = intToHex(start+pc);
				pc++;
				switch(text[pc]){
					//writeb
					case '0':{
						pc++;
						if(text[pc]<'0' || text[pc]>'7'){
							fprintf(stderr, "ERROR: Invalid register for d0\n");
							free(text);
							fclose(input);
							return 1;
						}
						pc++;
						if(text[pc]!='f' &&  text[pc]!='F'){
							fprintf(stderr, "ERROR: Invalid register for d0 (no f listed)\n");
							free(text);
							fclose(input);
							return 1;
						}
						pc++;
						char* displacement = (char*)malloc(sizeof(char)*9);
						displacement[8]='\0';
						for(i=0; i<4; i++){
							displacement[(8-((2*i)+1))-1]=text[pc];
							pc++;
							displacement[(8-(2*i))-1]=text[pc];
							pc++;
						}
						free(displacement);
						if(fine == 0){
							fprintf(stderr, "ERROR: Invalid hexadecimal displacement for d0\n");
							free(text);
							fclose(input);
							return 1;
						}
						printf("0x%s\tWHATEVER WRITEB IS\t%s\n", insadr,op);
						break;
					}
					//writel
					case '1':{
						pc++;
						if(text[pc]<'0' || text[pc]>'7'){
							fprintf(stderr, "ERROR: Invalid register for d1\n");
							free(text);
							fclose(input);
							return 1;
						}
						pc++;
						if(text[pc]!='f' &&  text[pc]!='F'){
							fprintf(stderr, "ERROR: Invalid register for d1 (no f listed)\n");
							free(text);
							fclose(input);
							return 1;
						}
						pc++;
						char* displacement = (char*)malloc(sizeof(char)*9);
						displacement[8]='\0';
						for(i=0; i<4; i++){
							displacement[(8-((2*i)+1))-1]=text[pc];
							pc++;
							displacement[(8-(2*i))-1]=text[pc];
							pc++;
						}
						free(displacement);
						if(fine == 0){
							fprintf(stderr, "ERROR: Invalid hexadecimal displacement for d1\n");
							free(text);
							fclose(input);
							return 1;
						}
						printf("0x%s\tWHATEVER WRITEL IS\t%s\n", insadr,op);
						break;
					}
					default:{
						fprintf(stderr, "ERROR: Invalid second opcode for d\n");
						free(text);
						fclose(input);
						return 1;
					}
				}
				break;
			}
			//movsbl
			case 'e':
			case 'E':{
				char op[13];
				for(i=0; i<12; i++)
					op[i]=text[pc+i];
				op[12]='\0';
				char* insadr = intToHex(start+pc);
				pc++;
				if(text[pc]!='0'){
					fprintf(stderr, "ERROR: Invalid second opcode for e\n");
					free(text);
					fclose(input);
					return 1;
				}
				pc++;
				int ra = text[pc]-48;
				if(text[pc]<'0' || text[pc]>'7'){
					fprintf(stderr, "ERROR: Invalid first register for e0\n");
					free(text);
					fclose(input);
					return 1;
				}
				pc++;
				int rb = text[pc]-48;
				if(text[pc]<'0' || text[pc]>'7'){
					fprintf(stderr, "ERROR: Invalid second register for e0\n");
					free(text);
					fclose(input);
					return 1;
				}
				pc++;
				char* displacement = (char*)malloc(sizeof(char)*9);
				displacement[8]='\0';
				for(i=0; i<4; i++){
					displacement[(8-((2*i)+1))-1]=text[pc];
					pc++;
					displacement[(8-(2*i))-1]=text[pc];
					pc++;
				}
				int disp = hexToInt(displacement);
				free(displacement);
				if(fine == 0){
					fprintf(stderr, "ERROR: Invalid hexadecimal displacement for e0\n");
					free(text);
					fclose(input);
					return 1;
				}
				printf("0x%s\tMOVSX\t%d(%s),%s\t%s\n", insadr,disp,getReg(rb),getReg(ra),op);
				break;
			}
			default:{
				fprintf(stderr, "ERROR: Invalid first opcode\n");
				free(text);
				fclose(input);
				return 1;
			}
		}
	}
	
	free(text);
	fclose(input);
	return 0;
}