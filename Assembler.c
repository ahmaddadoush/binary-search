#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>


typedef struct Symbol
{
	char* name;
	int mAddress;
	struct Symbol* next;
}Symbol;
void toHack(FILE* symbolLess, FILE* output);
void removeWhtieSP(FILE *source, FILE *dest);
void toSymbolLess(FILE* source, FILE* dest);
char* itoa(int val, int base);
int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		printf("Usage:\n Assembler [file name]\n");
		return -1;
	}
	else
	{
		// parsing the file name
		char* token = malloc(sizeof(char) * strlen(argv[1]) + 3);
		token = strcpy(token, argv[1]);
		//printf("%s\n" , token);
		char* fileName = strtok(token, ".");
		fileName = strcat(fileName, ".hack\0");
		//printf("%s\n" , fileName);
		//printf("%s\n" , argv[1]);
		FILE* source = fopen(argv[1], "r");
		if (source == NULL)
		{
			printf("Couldn't Open the file: %s\n", argv[1]);
			return -1;
		}
		FILE* dest = fopen(fileName, "w");
		if (dest == NULL)
		{
			printf("Couldn't Create the file: %s\n", fileName);
			return -1;
		}

		FILE* temp = fopen("temp.asm", "w");
		if (temp == NULL)
		{
			printf("Unable to Create a temporary file\n");
			return -1;
		}

		removeWhtieSP(source, temp);
		fclose(temp);

		FILE* temp2 = fopen("temp.asm", "r");
		FILE* temp3 = fopen("temp2.asm", "w");
		if (temp2 == NULL || temp3 == NULL)
		{
			printf("Unable to create Temporary files\n");
			return -1;
		}


		toSymbolLess(temp2, temp3);
		fclose(temp2);
		fclose(temp3);
		remove("temp.asm");


		FILE* temp4 = fopen("temp2.asm", "r");
		toHack(temp4, dest);

		// close files , delete temp file and free memory
		fclose(temp4);
		remove("temp2.asm");
		fclose(source);
		fclose(dest);
		free(token);
	}

	return 0;
}

// takes a symbolless file and write it in hack machine language on output
void toHack(FILE* symbolLess, FILE* output)
{
	char* line = calloc(16, sizeof(char));
	char* out = calloc(16, sizeof(char));
	int outSize = 16;
	int lineNum = 0;

	while (feof(symbolLess) == 0)
	{ 
		fgets(line, 17, symbolLess);
		if (line != NULL)
		{
			int lineLen = strlen(line);
			//printf("the line: %s" , line);
			if(strchr(line , '\r') != NULL) lineLen--;

			if (line[0] == '@')
			{
				// A instruction
				// Remeber value of A
				int Aval = atoi(strtok(line, "@"));
				// copy binary value of A as String to temp
				char* malloc1 = itoa(Aval, 2);
				// initialize first 0 (A instruction op)
				out[0] = '0';
				// Write zeros for not used bits
				int i;
				for (i = 1; i < outSize - strlen(malloc1); i++) { out[i] = '0'; }
				// continue the previous loop to copy the temp string to out
				int k = 0;
				int j;
				for (j = i; j<outSize; j++)
				{
					out[j] = malloc1[k];
					k++;
				}
				// if it's the first line don't write newline
				if (lineNum > 0) fputc('\n', output);
				// write the out string to output file
				//printf("out is%s\n" , out);
				fwrite(out, sizeof(char), (size_t)outSize, output);
			}
			else
			{
				// C instruction
				// 111 a c1 c2 c3 c4 c5 c6 d1 d2 d3 j1 j2 j3
				// dest = comp ; jump

				// first 3 bits are always 111
				out[0] = '1'; out[1] = '1'; out[2] = '1';

				// take a copy frome line
				char* lineCpy = malloc(sizeof(char)*lineLen+1);
				// x is for determine how many characters should i copy from line excluding '\n'
				int x = (line[lineLen - 1] == '\n') ? 1 : 0;
				strncpy(lineCpy, line, (size_t)(lineLen - x));
				lineCpy[lineLen - x] = '\0';
				// check for jump conditions
				char* jump = strrchr(lineCpy, ';');
				//printf("line copy %s",lineCpy);
				//printf("JUMP: %s\n" , jump);
				if (jump == NULL) { out[15] = '0'; out[14] = '0'; out[13] = '0'; }
				else if (strcmp(jump, ";JGT") == 0) { out[13] = '0'; out[14] = '0'; out[15] = '1'; }
				else if (strcmp(jump, ";JEQ") == 0) { out[13] = '0'; out[14] = '1'; out[15] = '0'; }
				else if (strcmp(jump, ";JGE") == 0) { out[13] = '0'; out[14] = '1'; out[15] = '1'; }
				else if (strcmp(jump, ";JLT") == 0) { out[13] = '1'; out[14] = '0'; out[15] = '0'; }
				else if (strcmp(jump, ";JNE") == 0) { out[13] = '1'; out[14] = '0'; out[15] = '1'; }
				else if (strcmp(jump, ";JLE") == 0) { out[13] = '1'; out[14] = '1'; out[15] = '0'; }
				else if (strcmp(jump, ";JMP") == 0) { out[13] = '1'; out[14] = '1'; out[15] = '1'; }

				// if there is DEST in the instruction and pars it's bits
				// (Don't move this block of code the order matters coz in next block pointers of line copy will change it's again)
				// i could solve this but it needs second copy from line or take values from line directly
				char* dest = (strchr(lineCpy, '=') != NULL) ? strtok(lineCpy, "=") : NULL;
				//printf("dest: %s\n" , dest);
				if (dest == NULL) { out[10] = '0'; out[11] = '0'; out[12] = '0'; }
				else
				{
					out[10] = (strchr(dest, 'A') != NULL) ? (char) '1' : (char) '0';
					out[11] = (strchr(dest, 'D') != NULL) ? (char) '1' : (char) '0';
					out[12] = (strchr(dest, 'M') != NULL) ? (char) '1' : (char) '0';
				}


				// here the copy of line again to parse the COMP part
				lineCpy = strncpy(lineCpy, line, (size_t)(lineLen - x));
				lineCpy[lineLen - x] = '\0';
				//printf("this line copy seconed time %s\n" , lineCpy);
				char* comp = (strchr(lineCpy, ';') != NULL) ? strtok(lineCpy, ";") : lineCpy;
				comp = (strchr(comp, '=') != NULL) ? strchr(comp, '=') : comp;
				int compLen;
				compLen = strlen(comp);

				if (comp[0] == '=')
				{
					int i;
					for(i = 0 ; i < compLen -1 ; i++)
					{
						comp[i] = comp[i+1];
					}
					compLen--;
				}
				comp[compLen] = '\0';
				//printf("Comp: %s" , comp);
				if (strcmp(comp, "0") == 0)        { out[3] = '0'; out[4] = '1'; out[5] = '0'; out[6] = '1'; out[7] = '0'; out[8] = '1'; out[9] = '0'; }
				else if (strcmp(comp, "1") == 0)   { out[3] = '0'; out[4] = '1'; out[5] = '1'; out[6] = '1'; out[7] = '1'; out[8] = '1'; out[9] = '1'; }
				else if (strcmp(comp, "-1") == 0)  { out[3] = '0'; out[4] = '1'; out[5] = '1'; out[6] = '1'; out[7] = '0'; out[8] = '1'; out[9] = '0'; }
				else if (strcmp(comp, "D") == 0)   { out[3] = '0'; out[4] = '0'; out[5] = '0'; out[6] = '1'; out[7] = '1'; out[8] = '0'; out[9] = '0'; }
				else if (strcmp(comp, "A") == 0)   { out[3] = '0'; out[4] = '1'; out[5] = '1'; out[6] = '0'; out[7] = '0'; out[8] = '0'; out[9] = '0'; }
				else if (strcmp(comp, "M") == 0)   { out[3] = '1'; out[4] = '1'; out[5] = '1'; out[6] = '0'; out[7] = '0'; out[8] = '0'; out[9] = '0'; }
				else if (strcmp(comp, "!D") == 0)  { out[3] = '0'; out[4] = '0'; out[5] = '0'; out[6] = '1'; out[7] = '1'; out[8] = '0'; out[9] = '1'; }
				else if (strcmp(comp, "!A") == 0)  { out[3] = '0'; out[4] = '1'; out[5] = '1'; out[6] = '0'; out[7] = '0'; out[8] = '0'; out[9] = '1'; }
				else if (strcmp(comp, "!M") == 0)  { out[3] = '1'; out[4] = '1'; out[5] = '1'; out[6] = '0'; out[7] = '0'; out[8] = '0'; out[9] = '1'; }
				else if (strcmp(comp, "-D") == 0)  { out[3] = '0'; out[4] = '0'; out[5] = '0'; out[6] = '1'; out[7] = '1'; out[8] = '1'; out[9] = '1'; }
				else if (strcmp(comp, "-A") == 0)  { out[3] = '0'; out[4] = '1'; out[5] = '1'; out[6] = '0'; out[7] = '0'; out[8] = '1'; out[9] = '1'; }
				else if (strcmp(comp, "-M") == 0)  { out[3] = '1'; out[4] = '1'; out[5] = '1'; out[6] = '0'; out[7] = '0'; out[8] = '1'; out[9] = '1'; }
				else if (strcmp(comp, "D+1") == 0) { out[3] = '0'; out[4] = '0'; out[5] = '1'; out[6] = '1'; out[7] = '1'; out[8] = '1'; out[9] = '1'; }
				else if (strcmp(comp, "A+1") == 0) { out[3] = '0'; out[4] = '1'; out[5] = '1'; out[6] = '0'; out[7] = '1'; out[8] = '1'; out[9] = '1'; }
				else if (strcmp(comp, "M+1") == 0) { out[3] = '1'; out[4] = '1'; out[5] = '1'; out[6] = '0'; out[7] = '1'; out[8] = '1'; out[9] = '1'; }
				else if (strcmp(comp, "D-1") == 0) { out[3] = '0'; out[4] = '0'; out[5] = '0'; out[6] = '1'; out[7] = '1'; out[8] = '1'; out[9] = '0'; }
				else if (strcmp(comp, "A-1") == 0) { out[3] = '0'; out[4] = '1'; out[5] = '1'; out[6] = '0'; out[7] = '0'; out[8] = '1'; out[9] = '0'; }
				else if (strcmp(comp, "M-1") == 0) { out[3] = '1'; out[4] = '1'; out[5] = '1'; out[6] = '0'; out[7] = '0'; out[8] = '1'; out[9] = '0'; }
				else if (strcmp(comp, "D+A") == 0) { out[3] = '0'; out[4] = '0'; out[5] = '0'; out[6] = '0'; out[7] = '0'; out[8] = '1'; out[9] = '0'; }
				else if (strcmp(comp, "D+M") == 0) { out[3] = '1'; out[4] = '0'; out[5] = '0'; out[6] = '0'; out[7] = '0'; out[8] = '1'; out[9] = '0'; }
				else if (strcmp(comp, "D-A") == 0) { out[3] = '0'; out[4] = '0'; out[5] = '1'; out[6] = '0'; out[7] = '0'; out[8] = '1'; out[9] = '1'; }
				else if (strcmp(comp, "D-M") == 0) { out[3] = '1'; out[4] = '0'; out[5] = '1'; out[6] = '0'; out[7] = '0'; out[8] = '1'; out[9] = '1'; }
				else if (strcmp(comp, "A-D") == 0) { out[3] = '0'; out[4] = '0'; out[5] = '0'; out[6] = '0'; out[7] = '1'; out[8] = '1'; out[9] = '1'; }
				else if (strcmp(comp, "M-D") == 0) { out[3] = '1'; out[4] = '0'; out[5] = '0'; out[6] = '0'; out[7] = '1'; out[8] = '1'; out[9] = '1'; }
				else if (strcmp(comp, "D&A") == 0) { out[3] = '0'; out[4] = '0'; out[5] = '0'; out[6] = '0'; out[7] = '0'; out[8] = '0'; out[9] = '0'; }
				else if (strcmp(comp, "D&M") == 0) { out[3] = '1'; out[4] = '0'; out[5] = '0'; out[6] = '0'; out[7] = '0'; out[8] = '0'; out[9] = '0'; }
				else if (strcmp(comp, "D|A") == 0) { out[3] = '0'; out[4] = '0'; out[5] = '1'; out[6] = '0'; out[7] = '1'; out[8] = '0'; out[9] = '1'; }
				else if (strcmp(comp, "D|M") == 0) { out[3] = '1'; out[4] = '0'; out[5] = '1'; out[6] = '0'; out[7] = '1'; out[8] = '0'; out[9] = '1'; }

				if (lineNum > 0) fputc('\n', output);
				// write the out string to output file
				//printf("out is%s\n" , out);
				fwrite(out, sizeof(char), (size_t)outSize, output);
				free(lineCpy);
			}
		}
		lineNum++;
	}
	free(line);
	free(out);
}

void removeWhtieSP(FILE *source, FILE *dest) {
	int lineLen = 0;
	char out[60];
	int outLen = 0;
	char* line2 = calloc(300, sizeof(char));
	while (feof(source) == 0)
	{
		// read a line
		// allocate memory for line variable
		
		fgets(line2, 251, source);
		if (line2 != NULL)
		{
			//printf("%s eol" , line);
			lineLen = strlen(line2);
			//if(strchr(line2 , '\r') != NULL) lineLen--;
			// if the line is only new line ignore it
			if (isspace(line2[0])) continue;
			else
			{
				int i;
				int o = 0;
				for (i = 0; i < lineLen; i++)
				{
					//printf("this line i%c\n" , line[i]);
					// if the character is space ignore it
					if (line2[i] == ' ') { continue; }
					else if (line2[i] == '\t') { continue; }
					else if (line2[i] == '\r') {continue;}
					else if (line2[i] == '\n' && o == 0) { break; }  // if we reach new line and no characters found move to next line
					else if ((line2[i] == '/' && line2[i + 1] == '/') && o == 0) { break; } // if start of a line comment stop and move to next line
					else if ((line2[i] == '/' && line2[i + 1] == '/') && o > 0) { out[o] = '\n'; o++; break; }  // if start if inline comment write a new line
					else { out[o] = line2[i]; o++; } // if normal character copy it
				}
				// terminate the out string
				out[o] = '\0';
				outLen = o ;
				//printf("out length: %i\n" , outLen);
				//printf("this is out index: %i\n" , o);
				//printf("%s" , line);
				//printf("this is out:%s\n" , out);

			}
			//printf("NE LINE\n");
			if (outLen > 0) { fputs(&out[0], dest); }
		}
		
	}
	free(line2);
}

void toSymbolLess(FILE* source, FILE* dest)
{
	// initialize symbol table with predefined symbols

	int index = 0;
	Symbol* table = malloc(sizeof(Symbol));
	table->name = malloc(sizeof(char) * 50);
	Symbol* trav = table;
	strcpy(trav->name, "SCREEN"); trav->mAddress = 16384; trav->next = malloc(sizeof(Symbol));trav->next->name = malloc(sizeof(char)*50); trav = trav->next; index++;
	strcpy(trav->name, "KBD"); trav->mAddress = 24576; trav->next = malloc(sizeof(Symbol));trav->next->name = malloc(sizeof(char)*50); trav = trav->next; index++;
	strcpy(trav->name, "SP"); trav->mAddress = 0; trav->next = malloc(sizeof(Symbol));trav->next->name = malloc(sizeof(char)*50); trav = trav->next; index++;
	strcpy(trav->name, "LCL"); trav->mAddress = 1; trav->next = malloc(sizeof(Symbol));trav->next->name = malloc(sizeof(char)*50); trav = trav->next; index++;
	strcpy(trav->name, "ARG"); trav->mAddress = 2; trav->next = malloc(sizeof(Symbol));trav->next->name = malloc(sizeof(char)*50); trav = trav->next; index++;
	strcpy(trav->name, "THIS"); trav->mAddress = 3; trav->next = malloc(sizeof(Symbol));trav->next->name = malloc(sizeof(char)*50); trav = trav->next; index++;
	strcpy(trav->name, "THAT"); trav->mAddress = 4; trav->next = malloc(sizeof(Symbol));trav->next->name = malloc(sizeof(char)*50); trav = trav->next; index++;
	strcpy(trav->name, "R0"); trav->mAddress = 0; trav->next = malloc(sizeof(Symbol));trav->next->name = malloc(sizeof(char)*50); trav = trav->next; index++;
	strcpy(trav->name, "R1"); trav->mAddress = 1; trav->next = malloc(sizeof(Symbol));trav->next->name = malloc(sizeof(char)*50); trav = trav->next; index++;
	strcpy(trav->name, "R2"); trav->mAddress = 2; trav->next = malloc(sizeof(Symbol));trav->next->name = malloc(sizeof(char)*50); trav = trav->next; index++;
	strcpy(trav->name, "R3"); trav->mAddress = 3; trav->next = malloc(sizeof(Symbol));trav->next->name = malloc(sizeof(char)*50); trav = trav->next; index++;
	strcpy(trav->name, "R4"); trav->mAddress = 4; trav->next = malloc(sizeof(Symbol));trav->next->name = malloc(sizeof(char)*50); trav = trav->next; index++;
	strcpy(trav->name, "R5"); trav->mAddress = 5; trav->next = malloc(sizeof(Symbol));trav->next->name = malloc(sizeof(char)*50); trav = trav->next; index++;
	strcpy(trav->name, "R6"); trav->mAddress = 6; trav->next = malloc(sizeof(Symbol));trav->next->name = malloc(sizeof(char)*50); trav = trav->next; index++;
	strcpy(trav->name, "R7"); trav->mAddress = 7; trav->next = malloc(sizeof(Symbol));trav->next->name = malloc(sizeof(char)*50); trav = trav->next; index++;
	strcpy(trav->name, "R8"); trav->mAddress = 8; trav->next = malloc(sizeof(Symbol));trav->next->name = malloc(sizeof(char)*50); trav = trav->next; index++;
	strcpy(trav->name, "R9"); trav->mAddress = 9; trav->next = malloc(sizeof(Symbol));trav->next->name = malloc(sizeof(char)*50); trav = trav->next; index++;
	strcpy(trav->name, "R10"); trav->mAddress = 10; trav->next = malloc(sizeof(Symbol));trav->next->name = malloc(sizeof(char)*50); trav = trav->next; index++;
	strcpy(trav->name, "R11"); trav->mAddress = 11; trav->next = malloc(sizeof(Symbol));trav->next->name = malloc(sizeof(char)*50); trav = trav->next; index++;
	strcpy(trav->name, "R12"); trav->mAddress = 12; trav->next = malloc(sizeof(Symbol));trav->next->name = malloc(sizeof(char)*50); trav = trav->next; index++;
	strcpy(trav->name, "R13"); trav->mAddress = 13; trav->next = malloc(sizeof(Symbol));trav->next->name = malloc(sizeof(char)*50); trav = trav->next; index++;
	strcpy(trav->name, "R14"); trav->mAddress = 14; trav->next = malloc(sizeof(Symbol));trav->next->name = malloc(sizeof(char)*50); trav = trav->next; index++;
	strcpy(trav->name, "R15"); trav->mAddress = 15; trav->next = malloc(sizeof(Symbol));trav->next->name = calloc(50,sizeof(char)); trav = trav->next; index++;


	//printf("Name: %s Address: %i\n" , (table+0)->name , (table+0)->mAddress);

	int lineNum = 0;
	int lables = 0;
	while (feof(source) == 0)
	{
		char* line = calloc(100, sizeof(char));
		fgets(line, 101, source);
		if (line != NULL)
		{
			int lineLen = strlen(line);
			if(strchr(line , '\r') != NULL) lineLen--;
			//printf("%i\n" , lineLen);
			//printf("%s" , line);

			if ((line[0] == '(') && (strchr(line, ')') != NULL)) // LABEL found
			{
				// printf("LABEL FOUND\n");

				// label is found
				// copy the label to symbol table

				//printf("%s\n" , &temp[0]);
				//printf("LINE NUM: %i\n" , lineNum);
				
				strncpy(trav->name, &line[1], (size_t)(lineLen - 3));
				trav->name[lineLen - 2] = '\0';
				trav->mAddress = lineNum - lables; // labels is to know how many line will be skipped in third pass
				trav->next = malloc(sizeof(Symbol));
				trav->next->name = calloc(50,sizeof(char));
				trav = trav->next;
				lables++;
				index++;
			}
		}
		lineNum++;
		free(line);
	}
	// second pass over file to add variables to table
	rewind(source);
	int n = 16;
	bool found;
	while (feof(source) == 0)
	{
		char* line = calloc(100, sizeof(char));
		fgets(line, 51, source);
		//printf("line in seconed pass %s" , line);
		if (line != NULL)
		{
			int lineLen = strlen(line);
			if(strchr(line , '\r') != NULL) lineLen--;
			if (line[0] == '@' && isalpha(line[1]))
			{
				Symbol* trav1 = table;
				found = false;
				int i;
				for (i = 0; i < index; i++)
				{
					if (trav1->name != NULL && strncmp(&line[1], trav1->name, (size_t)(lineLen - 2)) == 0){found = true;} 
					trav1 = trav1->next;
				}
				if (!found) // if not found in table so declare at as variable and give addrees in memory starting from 16
				{
					//printf("new Variable\n");


					// add it to table
					
					strncpy(trav1->name, &line[1], (size_t)(lineLen - 2));
					trav1->name[lineLen - 2] = '\0';
					trav1->mAddress = n;
					trav1->next = malloc(sizeof(Symbol));
					trav1->next->name = calloc(50,sizeof(char));
					trav1 = trav1->next;
					n++;
					index++;
				}
			}
		}
		free(line);
	}
	 /*Symbol* trav3 = table;
	int j;
	for(j=0;j < index ; j++)
	{
	printf("Name: %s Value: %i\n" , trav3->name , trav3->mAddress);
	trav3 = trav3->next;
	}*/
	
	// third pass over file to read and subtitute the lables and variables with their adresses
	rewind(source);

	while (feof(source) == 0)
	{
		char* line = calloc(100, sizeof(char));
		fgets(line, 51, source);

		int lineLen = 0;
		if (line != NULL)
		{   
			lineLen = strlen(line);
			if(strchr(line , '\r') != NULL) lineLen--;
			if (line[0] == '@' && isalpha(line[1]))
			{    
				Symbol* trav2 = table;
				int i;
				for (i = 0; i < index; i++)
				{
					if (strncmp(&line[1], trav2->name, (size_t)(lineLen - 2)) == 0)
					{
					    
						// this to know how many digits are there in the relative address
						int nDigits = (int)((trav2->mAddress == 0) ? 1 : floor(log10(abs(trav2->mAddress))) + 1);
                        char* temp = itoa(trav2->mAddress, 10);
                        if(strcmp(temp , "\0") == 0) temp="0";
                        
						strncpy(&line[1],temp , (size_t)nDigits);
						line[nDigits + 1] = '\n';
						line[nDigits + 2] = '\0';
						//printf("line %s" , line);
					}
					trav2 = trav2->next;
				}
			}
			else if ((line[0] == '(') && (strchr(line, ')') != NULL)) { line[0] = '\0'; } // if lable ignore it
			lineLen = strlen(line);
		    if (lineLen>0) fputs(line, dest);
		}
		
		free(line);
	}
	Symbol* trav4 = table;
	int y;
	for (y = 0; y<=index; y++)
	{
		trav4 = trav4->next;
		free(table->name);
		free(table);
		table = trav4;
	}
}






char* itoa(int val, int base){

	static char buf[32] = {0};

	int i = 30;

	for(; val && i ; --i, val /= base)

		buf[i] = "0123456789abcdef"[val % base];

	return &buf[i+1];

}
