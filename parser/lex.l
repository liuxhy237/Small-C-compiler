%{
#include<stdio.h>
#include<stdlib.h>
#include "main.h"
#include "y.tab.h"

						
extern void  yyerror(char *s);//�ú�������y.tab.c�ﶨ��ģ�������extern��������
extern TreeNode * newNode (char* node_name,int line); //ͬ�� 
int yywrap(void);
int Line=1; 	//�������������������Ϣ

%}

/*�������ɴ�Сд��ĸ���»������*/
nondigit	([_A-Za-z])

/*һλ���֣�������0��9*/
digit		([0-9])

/*A sequence of digits or digits followed by ��0x(0X)�� or ��0�� without spaces*/
integer		(({digit}+)|(0x|0X|0)[0-9a-fA-F]+)

/*��ʶ�����Է����ֿ�ͷ�����0��������ֻ������*/
identifier	({nondigit}({nondigit}|{digit})*)

/*һ����һ�������Ŀհ׷�*/
blank_chars	([ \f\r\t\v]+)

/*�����*/
%%

\n                        {Line++;}  							

int                    { yylval.treeNode= newNode(yytext,Line);
                          return TYPE;}  
struct                  { yylval.treeNode= newNode(yytext,Line);  
                          return STRUCT; }  
return                  { yylval.treeNode= newNode(yytext,Line);  
                          return RETURN; }  
if                      { yylval.treeNode= newNode(yytext,Line);  
                          return IF ; }  
else                    { yylval.treeNode= newNode(yytext,Line);  
                          return ELSE ;}  
break                   { yylval.treeNode= newNode(yytext,Line);  
                          return BREAK;}  
cont                    { yylval.treeNode= newNode(yytext,Line);  
                          return CONT; }  
for                     { yylval.treeNode= newNode(yytext,Line);  
                          return FOR;  }  
read                    { yylval.treeNode= newNode(yytext,Line);  
                          return READ;  }  
write                    { yylval.treeNode= newNode(yytext,Line);  
                          return WRITE;  }    
						  
{identifier}			{yylval.treeNode= newNode(yytext,Line);
						return IDENTIFIER;}		//��yacc����: ʶ����ļǺ�������IDENTIFIER
				

{integer}				{yylval.treeNode = newNode(yytext,Line);
						return INTEGER;}			//��yacc����: ʶ����ļǺ�������INTEGER
						  
[;]                    {  
                       yylval.treeNode= newNode(yytext,Line);  
                       return SEMI;  
                      }  
[,]                    {  
                       yylval.treeNode= newNode(yytext,Line);  
                       return COMMA;  
                      }  
[.]                    {  
                       yylval.treeNode= newNode(yytext,Line);  
                       return DOT;  
                      }  
(!)|(\+\+)|(\-\-)|(~)   {  
                       yylval.treeNode= newNode(yytext,Line);  
                       return UNARYOP;                    
                      }  
[-]                 {yylval.treeNode = newNode(yytext, Line);  
                    return (SUB);}  
(\*)|(\/)|(%)         {  
                       yylval.treeNode= newNode(yytext,Line);  
                       return BINARYOP1;  
                       }  
(\+)                  {  
                       yylval.treeNode= newNode(yytext,Line);  
                       return BINARYOP2;  
                       }  
(<<)|(>>)             {  
                       yylval.treeNode= newNode(yytext,Line);  
                       return BINARYOP3;  
                       }  
(>)|(>=)|(<)|(<=)     {  
                       yylval.treeNode= newNode(yytext,Line);  
                       return BINARYOP4;  
                       }  
(==)|(!=)             {  
                       yylval.treeNode= newNode(yytext,Line);  
                       return BINARYOP5;  
                       }  
[&]                   {  
                       yylval.treeNode= newNode(yytext,Line);  
                       return BINARYOP6;  
                       }  
"^"                   {  
                       yylval.treeNode= newNode(yytext,Line);  
                       return BINARYOP7;  
                       }  
(\|)                    {  
                       yylval.treeNode= newNode(yytext,Line);  
                       return BINARYOP8;  
                       }  
(&&)                     {  
                       yylval.treeNode= newNode(yytext,Line);  
                       return BINARYOP9;  
                       }  
(\|\|)                  {  
                       yylval.treeNode= newNode(yytext,Line);  
                       return BINARYOP10;  
                       }  
(\+=)|(-=)|(\*=)|(\/=)|(&=)|(^=)|(\|=)|(<<=)|(>>=)    {  
                       yylval.treeNode= newNode(yytext,Line);  
                       return BINARYOP11;  
                       }  
[=]                     { yylval.treeNode= newNode(yytext,Line);  
                        return ASSIGNOP;}  
[(]                     { yylval.treeNode= newNode(yytext,Line);  
                          return LP;  }  
[)]                     { yylval.treeNode= newNode(yytext,Line);  
                          return RP;  }  
"["                     { yylval.treeNode= newNode(yytext,Line);  
                          return LB;  }  
[]]                     { yylval.treeNode= newNode(yytext,Line);  
                          return RB;  }  
[{]                     { yylval.treeNode= newNode(yytext,Line);  
                          return LC;  }  
[}]                     { yylval.treeNode= newNode(yytext,Line);  
                          return RC;  }  
						  
				
{blank_chars}	{							//���հ׷�ʱ��ʲôҲ��������������
				}
				


.				{//.��ʾ��\n����������ַ���ע���������Ҫ���������Ϊһ��ƥ����.�Ͳ���ƥ�����Ĺ�����(������״̬<>��ͷ�Ĺ������)
					 yyerror("Error:invalid input.\n"); 
				}



%%

//��3�Σ�C���������
int yywrap(void)
{
	puts("Success!");
	return 1;	//����1��ʾ��ȡȫ�����������Ҫ���Ŷ������ļ�����������fopen���ļ����ļ�ָ�븳��yyin��������0
}