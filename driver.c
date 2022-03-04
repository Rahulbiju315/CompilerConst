/****   GROUP NUMBER: 43
Name: Rahul B
ID: 2019A7PS0134P

Name: Asish Juttu
ID: 2019A7PS0039P

Name: Narasimha Gupta Jangala
ID: 2019A7PS0138P

Name: Praneeth Chaitanya Jonnavithula
ID: 2019A7PS1111P

Name: Damargidda Rohan Raj Goud
ID: 2019A7PS0065P
****/

#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

#include "lexer.h"
#include "log.h"
#include "parser.h"
#include "parserDef.h"
#include "symbolTable.h"
#include "token.h"
#include "twinBuffer.h"

int main(int argc, char* argv[]){

    if(argc!=3){
        printf("%d",argc);
		printf("Check command: $./stage1exe  testcase.txt  parsetreeOutFile.txt\n");
		return 0;
	}

    printf("\n\nIMPLEMENTATION DETAILS: STAGE 1\n");
	printf("\t==> FIRST and FOLLOW set automated\n");
	printf("\t==> Both lexical and syntax analysis modules implemented\n");
	printf("\t==> Modules work with all the test cases\n");
    printf("\t==> Test case 6 expected list of errors do not exactly match with the output");

    /****************************************Implementation Starts*****************************************/


int ex=0;

    while(1){
        printf("\nPlease enter the task you want to perform:\n\n");
		printf("0: To exit\n");
        printf("1: For printing the comment free code on the console\n");
		printf("2: For printing the token list(on the console) generated by the lexer\n");
		printf("3: For parsing to verify the syntactic correctness of the input source code and printing the parse tree appropriately in the file mentioned\n");
		printf("4: For printing (on the console) the total time taken by your stage 1 code of lexer and parser to verify the syntactic correctness\n");

        printf("\n\nTask Number: ");
        int task;
		scanf("%d",&task);
        FILE *fp;
        char ch;
        TwinBuffer tb;
        TwinBuffer* tbuf = &tb;
        TokenInfo tinf;

        Grammar grammar;
        initGrammar(&grammar);
        initSymbolTable(&symbolTable);
        FirstAndFollow fnf;
        initFirstAndFollow(&fnf, &grammar);
        ParseTable ptable;
        initParseTable(&grammar, &fnf, &ptable);

        clock_t start_time, end_time;
        double total_CPU_time, total_CPU_time_in_seconds;

        switch(task){
            case 0:
                ex = 1;
            break;

            case 1:
                printf("%s",argv[1]);
                //exit(0);
                fp = fopen(argv[1],"r");
                if(fp==NULL){
                    printf("Error reading file\n");
                }
                while(1){
                    //printf("Entered loop\n");
                    ch = fgetc(fp);
                    if(ch=='%'){
                        while(ch != '\n' && ch != EOF){
                            //printf("%c\n",ch);
                            ch = fgetc(fp);
                        }
                    }
                    //printf("%c",ch);
                    if(ch==EOF) break;
                    printf("%c", ch);
                }
                fclose(fp);
               
            break;

            case 2:
                initTwinBuffer(tbuf, argv[1]);
                while((tinf = getNextToken(tbuf)).token != EOF_TOKEN){
                    if(tinf.token != ERROR_TOKEN){
                        if(tinf.token == TK_ID){
                            if(strlen(tinf.lexeme) <= 20)
                                printf("Line No.  %d \t Lexeme : %s \t Token : %s\n", tinf.lineNumber, tinf.lexeme, tokToStr(tinf.token));
                            else
                                printf("Line No: %d Error :Variable Identifier is longer than the prescribed length of 20 characters.\n",tinf.lineNumber);
                        }
                        else if(tinf.token == TK_FUNID){
                            if(strlen(tinf.lexeme) <= 30)
                                printf("Line No.  %d \t Lexeme : %s \t Token : %s\n", tinf.lineNumber, tinf.lexeme,tokToStr(tinf.token));
                            else
                                printf("Line No: %d Error :Function Identifier is longer than the prescribed length of 30 characters.\n",tinf.lineNumber);
                        }
                        else printf("Line No.  %d \t Lexeme : %s \t Token : %s\n", tinf.lineNumber, tinf.lexeme,tokToStr(tinf.token));
                    }
                    else{
                    //printf("Line Number : %d Error : Unknow pattern %s\n", tinf.lineNumber, tinf.lexeme);
                    if(strlen(tinf.lexeme) == 1)
                        printf("Line No. %d Error : Unknow Symbol <%s>\n", tinf.lineNumber, tinf.lexeme);
                    else 
                        printf("Line No. %d Error : Unknow Pattern <%s>\n", tinf.lineNumber, tinf.lexeme);
                    }
                }
                break;

            case 3:
                // CHECK PARSER CORRECTNESS
                initTwinBuffer(tbuf, argv[1]);
                ParseTree parse = initParseTree(&grammar, &ptable, tbuf);

                FILE* fptr = fopen(argv[2], "w");
                Inorder(parse.head, fptr);
                fclose(fptr);
            break;

            case 4:
                start_time = clock();
                // invoke your lexer and parser here
                initTwinBuffer(tbuf, argv[1]);
                ParseTree parse1 = initParseTree(&grammar, &ptable, tbuf);
                end_time = clock();
                total_CPU_time = (double) (end_time - start_time);
                total_CPU_time_in_seconds = total_CPU_time / CLOCKS_PER_SEC;
                printf("Total CPU time is: %lf",total_CPU_time);
                printf("Total CPU time in seconds is: %lf",total_CPU_time_in_seconds);
            break;
        }
        if(ex==1) break;
    }
    return 0;
}