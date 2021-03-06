#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include "main.h"

struct symbol
{
    char* word;
    char type;
    int arrSize;
    char* structName;
    int structMem;
};
struct symbol* symTable[27][20]; //symbol Table

int rNum, callNum, ifNum, forNum, arridxNum; //for register allocation
int paraFlag = 0; //paremetres flag
int paraPoint = 0; //parameters point
char* paraArr[10]; //parametres array
int entryDepth = 0; //depth of stmtblocks
char* arrName; //array Name
int arrPtr; // which element in array
int arrSize; //array size
int loadFlag = 1; //load or not?
char* strName; //struct name
int structMemNum; //Number of struct members
FILE *fout;

void argsFunc(TreeNode* p);
char* Exps(TreeNode* p);
char* Exp(TreeNode* p);
void stmt(TreeNode* p);
void stmts(TreeNode* p);
void ExpInner(TreeNode* p);
void InnerArgs(TreeNode* p);
void decStrIdINT(TreeNode* p);
void initArgs(TreeNode* p);
void args(TreeNode* p);
void para(TreeNode* p);
void paras(TreeNode* p);
void ExpArgs(TreeNode* p);
void stmtblock(TreeNode* p);
void extvarsType(TreeNode* p);
void defs(TreeNode* p);
void def(TreeNode* p);
void decInner(TreeNode* p);
void decsInner(TreeNode* p);
void decStrId(TreeNode* p);
void extvarsStrId(TreeNode* p);
void extdefStrID(TreeNode* p);
void defInStr(TreeNode* p);
void defsInStr(TreeNode* p);
void extdefStrOp(TreeNode *p);
void extvarsStruct(TreeNode * p);
void extdef(TreeNode *p);
void extdefs(TreeNode * p);
void func(TreeNode* p);


void program(TreeNode * root,char *filename)
{
	
	fout = fopen(filename,"w");
	fprintf(fout,"@.str = private unnamed_addr constant [3 x i8] c\"%%d\\00\", align 1\n");
    fprintf(fout,"@.str1 = private unnamed_addr constant [2 x i8] c\"\\0A\\00\", align 1\n"); 
	extdefs(root->child);
	fprintf(fout,"declare i32 @__isoc99_scanf(i8*, ...) #2\ndeclare i32 @printf(i8*, ...) #2\nattributes #0 = { nounwind uwtable \"disable-tail-calls\"=\"false\" \"less-precise-fpmad\"=\"false\" \"no-frame-pointer-elim\"=\"true\" \"no-frame-pointer-elim-non-leaf\" \"no-infs-fp-math\"=\"false\" \"no-nans-fp-math\"=\"false\" \"stack-protector-buffer-size\"=\"8\" \"target-cpu\"=\"x86-64\" \"target-features\"=\"+fxsr,+mmx,+sse,+sse2\" \"unsafe-fp-math\"=\"false\" \"use-soft-float\"=\"false\" }\nattributes #1 = { argmemonly nounwind }\nattributes #2 = { \"disable-tail-calls\"=\"false\" \"less-precise-fpmad\"=\"false\" \"no-frame-pointer-elim\"=\"true\" \"no-frame-pointer-elim-non-leaf\" \"no-infs-fp-math\"=\"false\" \"no-nans-fp-math\"=\"false\" \"stack-protector-buffer-size\"=\"8\" \"target-cpu\"=\"x86-64\" \"target-features\"=\"+fxsr,+mmx,+sse,+sse2\" \"unsafe-fp-math\"=\"false\" \"use-soft-float\"=\"false\" }");
}

void extdefs(TreeNode * p)
{
	if(p->child!= NULL)
	{
	extdef(p->child);
	extdefs(p->child->brother);
	}

}

void extdef(TreeNode *p)
{
	if(!strcmp(p->child->brother->name,"func"))//spec func stmtblock
	{
		//We don't need to check SPEC anymore, cause all the functions in Small-C returns INT
		func(p->child->brother);
		stmtblock(p->child->brother->brother);
	}
	else if(!strcmp(p->child->name,"type"))
	{	
		extvarsType(p->child->brother);
	}
	else if(!strcmp(p->child->name,"spec"))
	{
		extvarsStruct(p);
	}
	else
		printf("error in %d",p->Line);
}

void extvarsStruct(TreeNode * p)
{
	if(p->child->child->child->brother->brother != NULL)//STRUCT opttag LC defs RC
	{
		extdefStrOp(p);
	}
	else//STRUCT IDENTIFIER
	{
		extdefStrID(p);
	}
}

void extdefStrOp(TreeNode *p)
{
	TreeNode *tmp = p->child->child->child->brother->child;//IDENTIFIER
	fprintf(fout,"%%struct.%s = type { ",tmp->name);
	structMemNum = 0;
	defsInStr(p->child->child->child->brother->brother->brother);//DEFS in STRUCT opttag LC defs RC
	structMemNum = 0;
	fprintf(fout," } \n");
	//假设struct定义之后不会立即声明。 ASSUME there is no  STRUCT IDENTIFIER extvars SEMI;
}

void defsInStr(TreeNode* p) //definitons, for STSPEC -> STRUCT OPTTAG LC DEFS RC case
{
	if(p->child!=NULL)
	{
		defInStr(p->child);
		structMemNum++;
		if(p->child->brother->child != NULL) fprintf(fout,", ");
		defsInStr(p->child->brother);
	}

}

void defInStr(TreeNode* p)//definiton, for STSPEC -> STRUCT OPTTAG LC DEFS RC case
{
	TreeNode* tmpvar = p->child->brother->child->child->child;//IDENTIFIER
	int dim1=0;
	if(tmpvar->name[0]<'A' || tmpvar->name[0]>'z') dim1=26;
	else dim1 = (tmpvar->name[0] <= 'Z') ? tmpvar->name[0]-'A' : tmpvar->name[0]-'a';
	
	int i=0;
    
    while (symTable[dim1][i])//find an empty place AND CHECK ERRORS
    {
    	if(!strcmp(symTable[dim1][i]->word,tmpvar->name))//repeat define
    	{
    		FILE* errdir=NULL;  
     		errdir=fopen("stderr","w");  
     		fprintf(fout,"Error.");  
     		fprintf(errdir,"Line %d error: variables %s re-declared\n",tmpvar->Line,tmpvar->name);  
     		fprintf(stderr,"Line %d error: variables %s re-declared\n",tmpvar->Line,tmpvar->name); 
     		fclose(fout);  
     		fclose(errdir);  
     		exit(1); 
    	}
    	i++;
    }
    
    symTable[dim1][i] = (struct symbol*)malloc(sizeof(struct symbol));
    struct symbol* s = symTable[dim1][i];
    s->word = (char*)malloc(sizeof(char)*200);
    strcpy(s->word,tmpvar->name); //don't need s->type here
    s->structMem = structMemNum;
    s->arrSize = 0;

    fprintf(fout,"i32");
}

void extdefStrID(TreeNode* p)//external definiton for STRUCT IDENTIFIER
{
	//CAN DO CHECK HERE, TO SEE IF IDENTIFIER REALLY EXISTS
	strName = (char*)malloc(sizeof(char)*200);

    TreeNode* nodeId = p->child->child->child->brother;
    strcpy(strName,nodeId->name);
    extvarsStrId(p->child->brother);

    free(strName);
}

void extvarsStrId(TreeNode* p) //external variables for STRUCT ID
{
    if(p->child == NULL){}
    else if (p->child->brother!=NULL)
    {
        decStrId(p->child);
        extvarsStrId(p->child->brother->brother);
    }
    else decStrId(p->child);
}

void decStrId(TreeNode* p)//declaration
{
	TreeNode* tmp = p->child;//var
	if(tmp->brother == NULL)//without init
	{
		TreeNode* id = tmp->child;// IDENTIFIER, ASSUME there is no a[] in struct;
		fprintf(fout,"@%s = common global %%struct.%s zeroinitializer, align 4\n",id->name,strName);
		
		int dim1=0;
		if(id->name[0]<'A' || id->name[0]>'z') dim1 = 26;
		else dim1 = (id->name[0]<='Z')? id->name[0]-'A':id->name[0]-'a';
		int i=0;

		if(!strcmp(id->name,"int") || !strcmp(id->name,"read") || !strcmp(id->name,"write") || !strcmp(id->name,"struct"))
		{
				FILE* errdir=NULL;  
     			errdir=fopen("stderr","w");  
     			fprintf(fout,"Error.");  
     			fprintf(errdir,"Line %d error: Can not use reserved word!\n",id->Line);  
     			fprintf(stderr,"Line %d error: Can not use reserved word!\n",id->Line); 
     			fclose(fout);  
     			fclose(errdir);  
     			exit(1);
		}
		while (symTable[dim1][i])//find an empty place AND CHECK ERRORS
    	{
    		if(!strcmp(symTable[dim1][i]->word,id->name) && symTable[dim1][i]->type == 'g')//repeat define
    		{
    			FILE* errdir=NULL;  
     			errdir=fopen("stderr","w");  
     			fprintf(fout,"Error.");  
     			fprintf(errdir,"Line %d error: variables %s re-declared\n",id->Line,id->name); 
     			fprintf(stderr,"Line %d error: variables %s re-declared\n",id->Line,id->name);  
     			fclose(fout);  
     			fclose(errdir);  
     			exit(1); 
    		}
    		i++;
    	}

		symTable[dim1][i] = (struct symbol*)malloc(sizeof(struct symbol));
		struct symbol* s = symTable[dim1][i];
		s->word = (char*)malloc(sizeof(char)*200);
		strcpy(s->word,tmp->child->name);
		s->structName = (char*)malloc(sizeof(char)*200);
		strcpy(s->structName,strName);
		s->type = 'g';
		s->arrSize = 0;
	}
	else{//dec = var ASSIGNOP init
		//WAIT FOR FUTURE WORK, TO INIT VAR
	}
}

void extvarsType(TreeNode* p)
{
	if(p->child->brother == NULL)
	{
		decStrIdINT(p->child);
	}
	else
	{
		decStrIdINT(p->child);
		extvarsType(p->child->brother->brother);
	}
}

void decStrIdINT(TreeNode* p)//p is dec
{


	if(p->child->brother == NULL)//no init
	{
		if(p->child->child->brother ==NULL ) //IDENTIFIER
		{
			TreeNode* id = p->child->child;
			fprintf(fout,"@%s = common global i32 0, align 4\n",id->name);
			int dim1 = 0;
			if(id->name[0]<'A' || id->name[0]>'z') dim1 = 26;
			else dim1 = (id->name[0]<='Z')? id->name[0]-'A':id->name[0]-'a';
			int i=0;
			if(!strcmp(id->name,"int") || !strcmp(id->name,"read") || !strcmp(id->name,"write") || !strcmp(id->name,"struct"))
				{
					FILE* errdir=NULL;  
     				errdir=fopen("stderr","w");  
     				fprintf(fout,"Error.");  
     				fprintf(errdir,"Line %d error: Can not use reserved word!\n",id->Line);  
     				fprintf(stderr,"Line %d error: Can not use reserved word!\n",id->Line);
     				fclose(fout);  
     				fclose(errdir);  
     				exit(1);
				}
			while (symTable[dim1][i])//find an empty place AND CHECK ERRORS
    		{
    			if(!strcmp(symTable[dim1][i]->word,id->name) && symTable[dim1][i]->type == 'g')//repeat define
    			{
    				FILE* errdir=NULL;  
     				errdir=fopen("stderr","w");  
     				fprintf(fout,"Error.");  
     				fprintf(errdir,"Line %d error: variables %s re-declared\n",id->Line,id->name);  
     				fprintf(stderr,"Line %d error: variables %s re-declared\n",id->Line,id->name);
     				fclose(fout);  
     				fclose(errdir);  
     				exit(1); 
    			}
    			i++;
    		}
			symTable[dim1][i] = (struct symbol*)malloc(sizeof(struct symbol));
			struct symbol* s = symTable[dim1][i];
			s->word = (char*)malloc(sizeof(char)*60);
            strcpy(s->word,id->name);
            s->type = 'g';
            s->arrSize = 0;
		}
		else//var LB INTEGER RB , ASSUME there is only one dimension of array
		{
			TreeNode* id = p->child->child->child;//IDENTIFIER
			TreeNode* var = p->child->child;//var LB INTEGER RB
			fprintf(fout,"@%s = common global [ %d x i32] zeroinitializer,",id->name,atoi(var->brother->brother->name));
			fprintf(fout, " align %d\n",16);
			
			int dim1 = 0;
			if(id->name[0]<'A' || id->name[0]>'z') dim1 = 26;
			else dim1 = (id->name[0]<='Z')? id->name[0]-'A':id->name[0]-'a';
			int i=0;
			if(!strcmp(id->name,"int") || !strcmp(id->name,"read") || !strcmp(id->name,"write") || !strcmp(id->name,"struct"))
				{
					FILE* errdir=NULL;  
     				errdir=fopen("stderr","w");  
     				fprintf(fout,"Error.");  
     				fprintf(errdir,"Line %d error: Can not use reserved word!\n",id->Line); 
     				fprintf(stderr,"Line %d error: Can not use reserved word!\n",id->Line); 
     				fclose(fout);  
     				fclose(errdir);  
     				exit(1);
				}
			while (symTable[dim1][i])//find an empty place AND CHECK ERRORS
    		{
    			if(!strcmp(symTable[dim1][i]->word,id->name) && symTable[dim1][i]->type == 'g')//repeat define
    			{
    				FILE* errdir=NULL;  
     				errdir=fopen("stderr","w");  
     				fprintf(fout,"Error.");  
     				fprintf(errdir,"Line %d error: variables %s re-declared\n",id->Line,id->name); 
     				fprintf(stderr,"Line %d error: variables %s re-declared\n",id->Line,id->name); 
     				fclose(fout);  
     				fclose(errdir);  
     				exit(1); 
    			}
    			i++;
    		}

			symTable[dim1][i] = (struct symbol*)malloc(sizeof(struct symbol));
			struct symbol* s = symTable[dim1][i];
			s->word = (char*)malloc(sizeof(char)*60);
            strcpy(s->word,id->name);
            s->type = 'g';
			s->arrSize = atoi(var->brother->brother->name);
			if(s->arrSize <= 0)
			{
    			FILE* errdir=NULL;  
     			errdir=fopen("stderr","w");  
     			fprintf(fout,"Error.");  
     			fprintf(errdir,"Line %d error: Size of array must larger than 0.\n",id->Line); 
     			fprintf(stderr,"Line %d error: Size of array must larger than 0.\n",id->Line); 
     			fclose(fout);  
     			fclose(errdir);  
     			exit(1); 				
			}
		}
	}

	else //need to init
	{
		if(p->child->child->brother == NULL ) //IDENTIFIER with init value
		{
			TreeNode* id = p->child->child;
			TreeNode* initVal = p->child->brother->brother->child->child;//ASSUME there is only one integer in the init.
			fprintf(fout,"@%s = global i32 %d, align 4\n",id->name, atoi(initVal->name));
			
			int dim1 = 0;
			if(id->name[0]<'A' || id->name[0]>'z') dim1 = 26;
			else dim1 = (id->name[0]<='Z')? id->name[0]-'A':id->name[0]-'a';
			int i=0;
			if(!strcmp(id->name,"int") || !strcmp(id->name,"read") || !strcmp(id->name,"write") || !strcmp(id->name,"struct"))
				{
					FILE* errdir=NULL;  
     				errdir=fopen("stderr","w");  
     				fprintf(fout,"Error.");  
     				fprintf(errdir,"Line %d error: Can not use reserved word!\n",id->Line);  
     				fprintf(stderr,"Line %d error: Can not use reserved word!\n",id->Line); 
     				fclose(fout);  
     				fclose(errdir);  
     				exit(1);
				}
			while (symTable[dim1][i])//find an empty place AND CHECK ERRORS
    		{
    			if(!strcmp(symTable[dim1][i]->word,id->name) && symTable[dim1][i]->type == 'g')//repeat define
    			{
    				FILE* errdir=NULL;  
     				errdir=fopen("stderr","w");  
     				fprintf(fout,"Error.");  
     				fprintf(errdir,"Line %d error: variables %s re-declared\n",id->Line,id->name); 
     				fprintf(stderr,"Line %d error: variables %s re-declared\n",id->Line,id->name);  
     				fclose(fout);  
     				fclose(errdir);  
     				exit(1); 
    			}
    			i++;
    		}
			symTable[dim1][i] = (struct symbol*)malloc(sizeof(struct symbol));
			struct symbol* s = symTable[dim1][i];
			s->word = (char*)malloc(sizeof(char)*60);
            strcpy(s->word,id->name);
            s->type = 'g';
            s->arrSize = 0;
		}
		else//var LB INTEGER RB , ASSUME there is only one dimension of array
		{
			TreeNode* id = p->child->child->child;
			TreeNode* var = p->child->child;

			fprintf(fout,"@%s = global [%d x i32] [", id->name,atoi(var->brother->brother->name));
			initArgs(p->child->brother->brother);//init
			fprintf(fout,"], align %d\n",atoi(var->brother->brother->name));

			int dim1 = 0;
			if(id->name[0]<'A' || id->name[0]>'z') dim1 = 26;
			else dim1 = (id->name[0]<='Z')? id->name[0]-'A':id->name[0]-'a';
			int i=0;
			if(!strcmp(id->name,"int") || !strcmp(id->name,"read") || !strcmp(id->name,"write") || !strcmp(id->name,"struct"))
				{
					FILE* errdir=NULL;  
     				errdir=fopen("stderr","w");  
     				fprintf(fout,"Error.");  
     				fprintf(errdir,"Line %d error: Can not use reserved word!\n",id->Line);
     				fprintf(stderr,"Line %d error: Can not use reserved word!\n",id->Line);  
     				fclose(fout);  
     				fclose(errdir);  
     				exit(1);
				}
			while (symTable[dim1][i])//find an empty place AND CHECK ERRORS
    		{
    			if(!strcmp(symTable[dim1][i]->word,id->name) && symTable[dim1][i]->type == 'g')//repeat define
    			{
    				FILE* errdir=NULL;  
     				errdir=fopen("stderr","w");  
     				fprintf(fout,"Error.");  
     				fprintf(errdir,"Line %d error: variables %s re-declared\n",id->Line,id->name);  
     				fprintf(stderr,"Line %d error: variables %s re-declared\n",id->Line,id->name);
     				fclose(fout);  
     				fclose(errdir);  
     				exit(1); 
    			}
    			i++;
    		}
			symTable[dim1][i] = (struct symbol*)malloc(sizeof(struct symbol));
			struct symbol* s = symTable[dim1][i];
			s->word = (char*)malloc(sizeof(char)*60);
            strcpy(s->word,id->name);
            s->type = 'g';
			s->arrSize = atoi(var->brother->brother->name);
			if(s->arrSize <= 0)
			{
    			FILE* errdir=NULL;  
     			errdir=fopen("stderr","w");  
     			fprintf(fout,"Error.");  
     			fprintf(errdir,"Line %d error: Size of array must larger than 0.\n",id->Line); 
     			fprintf(stderr,"Line %d error: Size of array must larger than 0.\n",id->Line); 
     			fclose(fout);  
     			fclose(errdir);  
     			exit(1); 				
			}
		}
	}
}

void initArgs(TreeNode* p)
{
	args(p->child->brother);//args
}

void args(TreeNode* p)//ASSUME the Exp in args is just INTEGER
{
	if(p->child->brother == NULL)
	{
		ExpArgs(p->child);
	}
	else
	{
		ExpArgs(p->child);
		fprintf(fout,", ");
		args(p->child->brother->brother);
	}
}

void ExpArgs(TreeNode* p)//Exp in args
{
	fprintf(fout,"i32 %d", atoi(p->child->name));
}

void func(TreeNode* p)//func
{
	fprintf(fout,"define i32 @%s(",p->child->name);
	paras(p->child->brother->brother);
	fprintf(fout,") #0 {\n");
}

void paras(TreeNode* p)
{
	if(!strcmp(p->name,"NULL")){
		paraFlag = 0;
	}//para is empty
	else if(p->child->brother == NULL )//PARA
	{
		paraFlag = 1;
		para(p->child);
	}
	else//PARA COMMA PARAS
	{
		paraFlag = 1;
		para(p->child);
		fprintf(fout,", ");
		paras(p->child->brother->brother);
	}
}

void para(TreeNode* p)
{
	TreeNode* id = p->child->brother->child;//ASSUME the type is always INT and the var is always IDENTIFIER
	fprintf(fout, "i32 %%%s",id->name);
	
	int dim1 = 0;
	if(id->name[0]<'A' || id->name[0]>'z') dim1 = 26;
	else dim1 = (id->name[0]<='Z')? id->name[0]-'A':id->name[0]-'a';
	int i=0;
	if(!strcmp(id->name,"int") || !strcmp(id->name,"read") || !strcmp(id->name,"write") || !strcmp(id->name,"struct"))
	{
		FILE* errdir=NULL;  
   		errdir=fopen("stderr","w");  
     	fprintf(fout,"Error.");  
     	fprintf(errdir,"Line %d error: Can not use reserved word!\n",id->Line);  
     	fprintf(stderr,"Line %d error: Can not use reserved word!\n",id->Line);
     	fclose(fout);  
     	fclose(errdir);  
     	exit(1);
	}
	while (symTable[dim1][i])//find an empty place AND CHECK ERRORS
    {
    	if(!strcmp(symTable[dim1][i]->word,id->name) && symTable[dim1][i]->type == 'a')//repeat define
    	{
    		FILE* errdir=NULL;  
     		errdir=fopen("stderr","w");  
     		fprintf(fout,"Error.");  
     		fprintf(errdir,"Line %d error: variables %s re-declared\n",id->Line,id->name);
     		fprintf(stderr,"Line %d error: variables %s re-declared\n",id->Line,id->name);  
     		fclose(fout);  
     		fclose(errdir);  
     		exit(1); 
    	}
    	i++;
    }
	symTable[dim1][i] = (struct symbol*)malloc(sizeof(struct symbol));
	struct symbol* s = symTable[dim1][i];
	s->word = (char*)malloc(sizeof(char)*60);
    strcpy(s->word,id->name);
    s->type = 'a';
    s->arrSize = 0;
	
	paraArr[paraPoint] = (char*)malloc(sizeof(char)*60);
    strcpy(paraArr[paraPoint],id->name);
    paraPoint++;
}

void stmtblock(TreeNode* p)
{
	if (!entryDepth)
    {
        fprintf(fout,"entry:\n");
    }

    if (paraFlag)
    {
        int i=0;
        while (paraArr[i])
        {
            fprintf(fout,"  %%%s.addr = alloca i32, align 4\n",paraArr[i]);
            fprintf(fout,"  store i32 %%%s, i32* %%%s.addr, align 4\n",paraArr[i],paraArr[i]);
            free(paraArr[i]);
            i++;
        }
        paraFlag = 0;
        paraPoint = 0;
    }

    defs(p->child->brother);
    stmts(p->child->brother->brother);

    if (!entryDepth) fprintf(fout, "}\n");
}

void defs(TreeNode* p)
{
	if(!strcmp(p->name,"NULL")){}
	else
	{
		def(p->child);
		defs(p->child->brother);
	}
}

void def(TreeNode* p)
{
	//ASSUME the spec is TYPE
	decsInner(p->child->brother);
}

void decsInner(TreeNode* p)
{
	if(p->child->brother == NULL )//dec case
	{
		decInner(p->child);
	}
	else//DEC COMMA DECS case
	{
		decInner(p->child);
		decsInner(p->child->brother->brother);
	}
}

void decInner(TreeNode* p)
{
	if(p->child->brother == NULL)//without init
	{
		if(p->child->child->brother == NULL) //var is IDENTIFIER
		{
			TreeNode* id = p->child->child;
			fprintf(fout,"%%%s = alloca i32 , align 4\n",id->name);
			int dim1 = 0;
			if(id->name[0]<'A' || id->name[0]>'z') dim1 = 26;
			else dim1 = (id->name[0]<='Z')? id->name[0]-'A':id->name[0]-'a';
			int i=0;
			//while (symTable[dim1][i]) i++;
			if(!strcmp(id->name,"int") || !strcmp(id->name,"read") || !strcmp(id->name,"write") || !strcmp(id->name,"struct"))
				{
					FILE* errdir=NULL;  
     				errdir=fopen("stderr","w");  
     				fprintf(fout,"Error.");  
     				fprintf(errdir,"Line %d error: Can not use reserved word!\n",id->Line);  
     				fprintf(stderr,"Line %d error: Can not use reserved word!\n",id->Line);
     				fclose(fout);  
     				fclose(errdir);  
     				exit(1);
				}
			while (symTable[dim1][i])//find an empty place AND CHECK ERRORS
    		{
    			if(!strcmp(symTable[dim1][i]->word,id->name) && symTable[dim1][i]->type == 'l')//repeat define
    			{
    				FILE* errdir=NULL;  
     				errdir=fopen("stderr","w");  
     				fprintf(fout,"Error.");  
     				fprintf(errdir,"Line %d error: variables %s re-declared\n",id->Line,id->name);
     				fprintf(stderr,"Line %d error: variables %s re-declared\n",id->Line,id->name);  
     				fclose(fout);  
     				fclose(errdir);  
     				exit(1); 
    			}
    			i++;
    		}
			symTable[dim1][i] = (struct symbol*)malloc(sizeof(struct symbol));
			struct symbol* s = symTable[dim1][i];
			s->word = (char*)malloc(sizeof(char)*60);
            strcpy(s->word,id->name);
            s->type = 'l';
            s->arrSize = 0;
		}
		else//var is var LB INTEGER RB 
		{
			TreeNode* id = p->child->child->child;//IDENTIFIER
			TreeNode* var = p->child->child;//var LB INTEGER RB
			fprintf(fout,"%%%s = alloca [ %d x i32], align 4\n",id->name,atoi(var->brother->brother->name));
			
			int dim1 = 0;
			if(id->name[0]<'A' || id->name[0]>'z') dim1 = 26;
				else dim1 = (id->name[0]<='Z')? id->name[0]-'A':id->name[0]-'a';
			int i=0;
			//while (symTable[dim1][i]) i++;
			if(!strcmp(id->name,"int") || !strcmp(id->name,"read") || !strcmp(id->name,"write") || !strcmp(id->name,"struct"))
				{
					FILE* errdir=NULL;  
     				errdir=fopen("stderr","w");  
     				fprintf(fout,"Error.");  
     				fprintf(errdir,"Line %d error: Can not use reserved word!\n",id->Line);
     				fprintf(stderr,"Line %d error: Can not use reserved word!\n",id->Line);  
     				fclose(fout);  
     				fclose(errdir);  
     				exit(1);
				}
			while (symTable[dim1][i])//find an empty place AND CHECK ERRORS
    		{
    			if(!strcmp(symTable[dim1][i]->word,id->name) && symTable[dim1][i]->type == 'l')//repeat define
    			{
    				FILE* errdir=NULL;  
     				errdir=fopen("stderr","w");  
     				fprintf(fout,"Error.");  
     				fprintf(errdir,"Line %d error: variables %s re-declared\n",id->Line,id->name); 
     				fprintf(stderr,"Line %d error: variables %s re-declared\n",id->Line,id->name); 
     				fclose(fout);  
     				fclose(errdir);  
     				exit(1); 
    			}
    			i++;
    		}
			symTable[dim1][i] = (struct symbol*)malloc(sizeof(struct symbol));
			struct symbol* s = symTable[dim1][i];
			s->word = (char*)malloc(sizeof(char)*60);
            strcpy(s->word,id->name);
            s->type = 'l';
			s->arrSize = atoi(var->brother->brother->name);
			if(s->arrSize <= 0)
			{
    			FILE* errdir=NULL;  
     			errdir=fopen("stderr","w");  
     			fprintf(fout,"Error.");  
     			fprintf(errdir,"Line %d error: Size of array must larger than 0.\n",id->Line); 
     			fprintf(stderr,"Line %d error: Size of array must larger than 0.\n",id->Line); 
     			fclose(fout);  
     			fclose(errdir);  
     			exit(1); 				
			}
		}
	}
	else//with init
	{
		if(p->child->child->brother == NULL)
		{
			TreeNode* id = p->child->child;
			TreeNode* initVal = p->child->brother->brother->child->child;//ASSUME there is only one integer in the init.
			fprintf(fout,"%s = alloca i32, align 4\n",id->name);
			fprintf(fout,"store i32 %d, i32* %s, align 4\n",atoi(initVal->name),id->name);
			
			int dim1 = 0;
			if(id->name[0]<'A' || id->name[0]>'z') dim1 = 26;
			else dim1 = (id->name[0]<='Z')? id->name[0]-'A':id->name[0]-'a';
			int i=0;
			//while (symTable[dim1][i]) i++;
			if(!strcmp(id->name,"int") || !strcmp(id->name,"read") || !strcmp(id->name,"write") || !strcmp(id->name,"struct"))
				{
					FILE* errdir=NULL;  
     				errdir=fopen("stderr","w");  
     				fprintf(fout,"Error.");  
     				fprintf(errdir,"Line %d error: Can not use reserved word!\n",id->Line);  
     				fprintf(stderr,"Line %d error: Can not use reserved word!\n",id->Line);
     				fclose(fout);  
     				fclose(errdir);  
     				exit(1);
				}
			while (symTable[dim1][i])//find an empty place AND CHECK ERRORS
    		{
    			if(!strcmp(symTable[dim1][i]->word,id->name) && symTable[dim1][i]->type == 'l')//repeat define
    			{
    				FILE* errdir=NULL;  
     				errdir=fopen("stderr","w");  
     				fprintf(fout,"Error.");  
     				fprintf(errdir,"Line %d error: variables %s re-declared\n",id->Line,id->name);  
     				fprintf(stderr,"Line %d error: variables %s re-declared\n",id->Line,id->name);
     				fclose(fout);  
     				fclose(errdir);  
     				exit(1); 
    			}
    			i++;
    		}
			symTable[dim1][i] = (struct symbol*)malloc(sizeof(struct symbol));
			struct symbol* s = symTable[dim1][i];
			s->word = (char*)malloc(sizeof(char)*60);
            strcpy(s->word,id->name);
            s->type = 'l';
            s->arrSize = 0;
		}
		else
		{
			TreeNode* id = p->child->child->child;
			TreeNode* var = p->child->child;
			fprintf(fout,"  %%%s = alloca [%d x i32], align 4\n", id->name,atoi(var->brother->brother->name));
			
			int dim1 = 0;
			if(id->name[0]<'A' || id->name[0]>'z') dim1 = 26;
			else dim1 = (id->name[0]<='Z')? id->name[0]-'A':id->name[0]-'a';
			int i=0;
			//while (symTable[dim1][i]) i++;
			if(!strcmp(id->name,"int") || !strcmp(id->name,"read") || !strcmp(id->name,"write") || !strcmp(id->name,"struct"))
				{
					FILE* errdir=NULL;  
     				errdir=fopen("stderr","w");  
     				fprintf(fout,"Error.");  
     				fprintf(errdir,"Line %d error: Can not use reserved word!\n",id->Line);  
     				fprintf(stderr,"Line %d error: Can not use reserved word!\n",id->Line);
     				fclose(fout);  
     				fclose(errdir);  
     				exit(1);
				}
			while (symTable[dim1][i])//find an empty place AND CHECK ERRORS
    		{
    			if(!strcmp(symTable[dim1][i]->word,id->name) && symTable[dim1][i]->type == 'l')//repeat define
    			{
    				FILE* errdir=NULL;  
     				errdir=fopen("stderr","w");  
     				fprintf(fout,"Error.");  
     				fprintf(errdir,"Line %d error: variables %s re-declared\n",id->Line,id->name); 
     				fprintf(stderr,"Line %d error: variables %s re-declared\n",id->Line,id->name);  
     				fclose(fout);  
     				fclose(errdir);  
     				exit(1); 
    			}
    			i++;
    		}
			symTable[dim1][i] = (struct symbol*)malloc(sizeof(struct symbol));
			struct symbol* s = symTable[dim1][i];
			s->word = (char*)malloc(sizeof(char)*60);
            strcpy(s->word,id->name);
            s->type = 'l';
			s->arrSize = atoi(var->brother->brother->name);
			if(s->arrSize <= 0)
			{
    			FILE* errdir=NULL;  
     			errdir=fopen("stderr","w");  
     			fprintf(fout,"Error.");  
     			fprintf(errdir,"Line %d error: Size of array must larger than 0.\n",id->Line); 
     			fprintf(stderr,"Line %d error: Size of array must larger than 0.\n",id->Line); 
     			fclose(fout);  
     			fclose(errdir);  
     			exit(1); 				
			}
						
			arrName = (char*)malloc(sizeof(char)*60);
			strcpy(arrName,id->name);
			arrSize = s->arrSize;
			InnerArgs(p->child->brother->brother->child->brother);//args under init
			free(arrName);
			arrSize = 0;
		}
	}
}

void InnerArgs(TreeNode* p)
{
	if(p->child->brother == NULL )
	{
		ExpInner(p->child);
		arrPtr = 0;//指向数组中第几个元素
	}
	else{
		ExpInner(p->child);
		arrPtr++;
		InnerArgs(p->child->brother->brother);
	}
}

void ExpInner(TreeNode* p)
{
		char* val = (char*)malloc(sizeof(char)*60);
        val = Exp(p);
        fprintf(fout,"  %%arrayidx%d = getelementptr inbounds [%d x i32], [%d x i32]* %%%s, i64 0, i64 %d\n",arridxNum,arrSize,arrSize,arrName,arrPtr);
        fprintf(fout,"  store i32 %s, i32* %%arrayidx%d, align 4\n",val,arridxNum);
        arridxNum++;
}

void stmts(TreeNode* p)
{
	if(!strcmp(p->name,"NULL"))
	{
		//do nothing
	}
	else{
		stmt(p->child);
		stmts(p->child->brother);
	}

}

void stmt(TreeNode* p)
{
	if(p->child->name[0]=='e' && p->child->name[1]=='x' && p->child->name[2]=='p')
	{
		Exp(p->child);
	}
	else if(!strcmp(p->child->name,"stmtblock"))
	{

		entryDepth++;
		stmtblock(p->child);
		entryDepth--;
	}
	else if(!strcmp(p->child->name,"return"))
	{

        char* tmp = (char*)malloc(sizeof(char)*60);
        tmp = Exps(p->child->brother);//EXPS will not be empty
        fprintf(fout,"  ret i32 %s\n",tmp);
        rNum++;
	}
	else if(!strcmp(p->child->name,"if"))
	{
        if (p->child->brother->brother->brother->brother->brother->child!= NULL) //ESTMT not null
        {
            char* tmp = (char*)malloc(sizeof(char)*60);
            tmp = Exps(p->child->brother->brother);


            if (!strcmp(p->child->brother->brother->child->brother->name,"."))//DOT, special case
            {
                char num[10];
                sprintf(num, "%d", rNum++);
                char* tmpReg = (char*)malloc(sizeof(char)*60);
                strcpy(tmpReg,"%r");
                strcat(tmpReg,num);

                fprintf(fout,"  %s = icmp ne i32 %s, 0\n",tmpReg,tmp);
                strcpy(tmp,tmpReg);
            }


            fprintf(fout,"  br i1 %s, label %%if%d.then, label %%if%d.else\n\n",tmp, ifNum, ifNum);

            fprintf(fout,"if%d.then:\n",ifNum);
            stmt(p->child->brother->brother->brother->brother);
            fprintf(fout,"  br label %%if%d.end\n\n",ifNum);

            fprintf(fout,"if%d.else:\n",ifNum);
            stmt(p->child->brother->brother->brother->brother->brother->child->brother);
            fprintf(fout,"  br label %%if%d.end\n\n",ifNum);

            fprintf(fout,"if%d.end:\n",ifNum);

            ifNum++;
        }
        else
        {

            char* tmp = (char*)malloc(sizeof(char)*60);
            tmp = Exps(p->child->brother->brother);

            if (!strcmp(p->child->brother->brother->child->brother->name,"."))//DOT, special case
            {
                char num[10];
                sprintf(num, "%d", rNum++);
                char* tmpReg = (char*)malloc(sizeof(char)*60);
                strcpy(tmpReg,"%r");
                strcat(tmpReg,num);

                fprintf(fout,"  %s = icmp ne i32 %s, 0\n",tmpReg,tmp);
                strcpy(tmp,tmpReg);
            }


            fprintf(fout,"  br i1 %s, label %%if%d.then, label %%if%d.end\n\n",tmp, ifNum, ifNum);

            fprintf(fout,"if%d.then:\n",ifNum);
            stmt(p->child->brother->brother->brother->brother);
            fprintf(fout,"  br label %%if%d.end\n\n",ifNum);

            fprintf(fout,"if%d.end:\n",ifNum);

            ifNum++;

        }		
	}
	else if(!strcmp(p->child->name,"for"))
	{
        Exp(p->child->brother->brother);//EXP can be empty
        fprintf(fout,"  br label %%for%d.cond\n\n",forNum);

        fprintf(fout,"for%d.cond:\n",forNum);
        char* tmp = (char*)malloc(sizeof(char)*60);
		TreeNode* Exp2 = p->child->brother->brother->brother->brother;
 
        tmp = Exp(Exp2);
	
        if (!strcmp(Exp2->child->brother->name,"arrs")) //special case, ID ARRS
        {
            fprintf(fout,"  %%r%d = icmp ne i32 %s, 0\n",rNum,tmp);
            fprintf(fout,"  br i1 %%r%d, label %%for%d.body, label %%for%d.end\n\n",rNum,forNum,forNum);
            rNum++;
        }
        else fprintf(fout,"  br i1 %s, label %%for%d.body, label %%for%d.end\n\n",tmp,forNum,forNum);

        fprintf(fout,"for%d.body:\n",forNum);
        stmt(p->child->brother->brother->brother->brother->brother->brother->brother->brother);
        fprintf(fout,"  br label %%for%d.inc\n\n",forNum);
        fprintf(fout,"for%d.inc:\n",forNum);

        Exp(p->child->brother->brother->brother->brother->brother->brother);
        fprintf(fout,"  br label %%for%d.cond\n\n",forNum);
        fprintf(fout,"for%d.end:\n",forNum);

        forNum++;
	
	}
	else if(!strcmp(p->child->name,"cont"))
	{
		
	}
	else if(!strcmp(p->child->name,"break"))
	{
		
	}
	else if(!strcmp(p->child->name,"write"))
	{
			char* tmp = (char*)malloc(sizeof(char)*60);
            tmp = Exp(p->child->brother->brother);
            int trans;
            if (strlen(tmp)>1 && (tmp[0]=='0' || (tmp[0]=='-' && tmp[1]=='0')))
            {
                trans = strtol(tmp,NULL,0);
                fprintf(fout,"  %%call%d = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8],[3 x i8]* @.str, i32 0, i32 0), i32 %d)\n",callNum,trans);
                callNum++;
                callNum++;
            }
            else
            {
                fprintf(fout,"  %%call%d = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8],[3 x i8]* @.str, i32 0, i32 0), i32 %s)\n",callNum,tmp);
                callNum++;
                callNum++;
            }
	}
	else//read
	{
			char* tmp = (char*)malloc(sizeof(char)*200);
            loadFlag = 0;
            tmp = Exp(p->child->brother->brother);
            loadFlag = 1;

            fprintf(fout,"  %%call%d = call i32 (i8*, ...) @__isoc99_scanf(i8* getelementptr inbounds ([3 x i8],[3 x i8]* @.str, i32 0, i32 0), i32* %s)\n",callNum,tmp);
            callNum++;

	}
}


char* Exp(TreeNode* p)
{
	if(!strcmp(p->name,"exp-null"))
		return NULL;
	else
		return Exps(p);
}

char* Exps(TreeNode* p)
{
	if (p->child->brother==NULL) //EXP->INT
    {
        char* tmp = (char*)malloc(sizeof(char)*60);
        strcpy(tmp,p->child->name);
        return tmp;
    }

    else if (!strcmp(p->child->name,"++")) //++
    {
        char* op = (char*)malloc(sizeof(char)*60);
        loadFlag = 0;
        op = Exp(p->child->brother);
        loadFlag = 1;

        fprintf(fout,"  %%r%d = load i32, i32* %s, align 4\n",rNum,op);
        fprintf(fout,"  %%r%d = add nsw i32 %%r%d, 1\n",rNum+1,rNum);
        fprintf(fout,"  store i32 %%r%d, i32* %s, align 4\n",rNum+1,op);

        rNum+=2;
        return NULL;
    }
	else if (!strcmp(p->child->name,"--")) //--
	{
		//The same like ++
	}
    else if (!strcmp(p->child->name,"-")) //-
    {
        char* tmp = (char*)malloc(sizeof(char)*60);
        strcpy(tmp,"-");
	strcat(tmp,Exp(p->child->brother));
        return tmp;
    }
    else if (!strcmp(p->child->name,"!")) //!
    {
        char* op = (char*)malloc(sizeof(char)*60);
        op = Exp(p->child->brother);

        char num[10];
        sprintf(num, "%d", rNum++);
        char* tmpReg = (char*)malloc(sizeof(char)*60);
		char* tmpReg1 = (char*)malloc(sizeof(char)*60);
		char* retReg = (char*)malloc(sizeof(char)*60);
        strcpy(tmpReg,"%r");
        strcat(tmpReg,num);
        fprintf(fout,"  %s = icmp ne i32 %s, 0\n",tmpReg,op);
		
		strcpy(tmpReg1,tmpReg);
        strcat(tmpReg1,"lnot");
		fprintf(fout,"  %s = xor i1 %s, true\n",tmpReg1,tmpReg);
		return tmpReg1;
    }

    else if (!strcmp(p->child->brother->name,"=")) //EXP->EXP ASSIGNOP EXP
    {

        char* op2 = (char*)malloc(sizeof(char)*200);
        op2 = Exp(p->child->brother->brother);
        //这里需要判断op1是不是左值
        loadFlag = 0;
        char* op1 = (char*)malloc(sizeof(char)*200);
        op1 = Exp(p->child);
        if(op1[0]>='0' && op1[0]<='9')
        {
        	FILE* errdir=NULL;  
     		errdir=fopen("stderr","w");  
     		fprintf(fout,"Error.");  
     		fprintf(errdir,"Line %d error: %s is not a left value\n",p->child->Line,p->child->child->name); 
     		fprintf(stderr,"Line %d error: %s is not a left value\n",p->child->Line,p->child->child->name); 
     		fclose(fout);  
     		fclose(errdir);  
     		exit(1); 
        }
        loadFlag = 1;

        fprintf(fout,"  store i32 %s, i32* %s, align 4\n",op2,op1);
        return NULL;

    }
    else if (!strcmp(p->child->name,"(")) //LP EXP RP
    {
        return Exp(p->child->brother);
    }
    else if (!strcmp(p->child->brother->name,"arrs")) //IDENTIFIER arrs           
    {
        TreeNode* nodeArrs = p->child->brother;
        if (nodeArrs->child == NULL) //ARRS->NULL, ID case
        {
            char* tmp = (char*)malloc(sizeof(char)*60);
	    char* tmp1 = (char*)malloc(sizeof(char)*60);
            TreeNode* nodeId = p->child;
	    strcpy(tmp1,nodeId->name);
			int dim1 = 0;
			if(nodeId->name[0]<'A' || nodeId->name[0]>'z') dim1 = 26;
			else dim1 = (nodeId->name[0]<='Z')? nodeId->name[0]-'A':nodeId->name[0]-'a';

            int i=0;
            while (!symTable[dim1][i] || strcmp(nodeId->name,symTable[dim1][i]->word)) 
			{
				i++;
				if(i>=20)
				{
        			FILE* errdir=NULL;  
     				errdir=fopen("stderr","w");  
     				fprintf(fout,"Error.");  
     				fprintf(errdir,"Line %d error: Variable %s undefined\n",nodeId->Line,nodeId->name);  
     				fprintf(stderr,"Line %d error: Variable %s undefined\n",nodeId->Line,nodeId->name);
     				fclose(fout);  
     				fclose(errdir);  
     				exit(1);
				}
			}
            struct symbol* id = symTable[dim1][i];
            switch (id->type)
            {
                case 'g':
		for (i=strlen(tmp1);i>=0;i--) tmp[i+1] = tmp1[i];
                tmp[0] = '@';
                break;

                case 'l':
                tmp[0] = '%';
				strcat(tmp,tmp1);
                break;

                case 'a':
                tmp[0] = '%';
				strcat(tmp,tmp1);
                strcat(tmp,".addr");
                break;
            }
	    free(tmp1);
            if (loadFlag)
            {
                char num[10];
                sprintf(num, "%d", rNum++);
                char* tmpReg = (char*)malloc(sizeof(char)*60);
                strcpy(tmpReg,"%r");
                strcat(tmpReg,num);

                fprintf(fout,"  %s = load i32, i32* %s, align 4\n",tmpReg,tmp);
                return tmpReg;
            }
            else return tmp;
        }
        else //we need to return arrindex
        {
            char* tmp = (char*)malloc(sizeof(char)*60);
            TreeNode* nodeId = p->child;
            strcpy(tmp,nodeId->name);

            char* arrsIndex = (char*)malloc(sizeof(char)*60);
            if (loadFlag==0)
            {
                loadFlag = 1;
                arrsIndex = Exp(p->child->brother->child->brother); //what we obtained could be register or INT
                loadFlag = 0;
            }
            else arrsIndex = Exp(p->child->brother->child->brother);

            char* ret = (char*)malloc(sizeof(char)*60);
            strcpy(ret,"%arrayidx");

            char num[10];
            sprintf(num, "%d", arridxNum++);
            strcat(ret,num);

			int dim1 = 0;
			if(nodeId->name[0]<'A' || nodeId->name[0]>'z') dim1 = 26;
			else dim1 = (nodeId->name[0]<='Z')? nodeId->name[0]-'A':nodeId->name[0]-'a';

            int i=0;
            while (!symTable[dim1][i] || strcmp(nodeId->name,symTable[dim1][i]->word)) //error checking
			{
				i++;
				if(i>=20)
				{
					FILE* errdir=NULL;  
     				errdir=fopen("stderr","w");  
     				fprintf(fout,"Error.");  
     				fprintf(errdir,"Line %d error: Array %s undefined\n",nodeId->Line,nodeId->name);  
     				fprintf(stderr,"Line %d error: Array %s undefined\n",nodeId->Line,nodeId->name); 
     				fclose(fout);  
     				fclose(errdir);  
     				exit(1); 
				}
			}
			if(!symTable[dim1][i]->arrSize)
			{	
				FILE* errdir=NULL;  
     			errdir=fopen("stderr","w");  
     			fprintf(fout,"Error.");  
     			fprintf(errdir,"Line %d error: %s is not an array\n",nodeId->Line,nodeId->name);  
     			fprintf(stderr,"Line %d error: %s is not an array\n",nodeId->Line,nodeId->name);
     			fclose(fout);  
   				fclose(errdir);  
   				exit(1);
			}			
            struct symbol* id = symTable[dim1][i];
            switch (id->type)
            {
                case 'g':
                for (i=strlen(tmp);i>=0;i--) tmp[i+1] = tmp[i];
                tmp[0] = '@';
                break;

                case 'l':
                for (i=strlen(tmp);i>=0;i--) tmp[i+1] = tmp[i];
                tmp[0] = '%';
                break;

                case 'a':
                for (i=strlen(tmp);i>=0;i--) tmp[i+1] = tmp[i];
                tmp[0] = '%';
                strcat(tmp,".addr");
                break;
            }

            fprintf(fout,"  %s = getelementptr inbounds [%d x i32] ,[%d x i32]* %s, i32 0, i32 %s\n",ret,id->arrSize,id->arrSize,tmp,arrsIndex);

            if (loadFlag)
            {
                char num[10];
                sprintf(num, "%d", rNum++);
                char* tmpReg = (char*)malloc(sizeof(char)*60);
                strcpy(tmpReg,"%r");
                strcat(tmpReg,num);

                fprintf(fout,"  %s = load i32, i32* %s, align 4\n",tmpReg,ret);
                return tmpReg;
            }
            else return ret;
        }
    }

    else if (!strcmp(p->child->brother->name,".")) ////EXP->EXP DOT THEID
    {
        TreeNode* nodeId = p->child->child;

        int dim1 = 0;
		if(nodeId->name[0]<'A' || nodeId->name[0]>'z') dim1 = 26;
		else dim1 = (nodeId->name[0]<='Z')? nodeId->name[0]-'A':nodeId->name[0]-'a';

        int i=0;
        while (!symTable[dim1][i] || strcmp(nodeId->name,symTable[dim1][i]->word)) //Error checking
		{
			i++;
			if(i>=20) 
			{
				FILE* errdir=NULL;  
     			errdir=fopen("stderr","w");  
     			fprintf(fout,"Error.");  
     			fprintf(errdir,"Line %d error:Struct %s undefined\n",nodeId->Line,nodeId->name);  
     			fprintf(stderr,"Line %d error:Struct %s undefined\n",nodeId->Line,nodeId->name);
     			fclose(fout);  
   				fclose(errdir);  
   				exit(1);
			}
		}
		if(!symTable[dim1][i]->structName)
		{
				FILE* errdir=NULL;  
     			errdir=fopen("stderr","w");  
     			fprintf(fout,"Error.");  
     			fprintf(errdir,"Line %d error: %s is not a struct\n",nodeId->Line,nodeId->name);  
     			fprintf(stderr,"Line %d error: %s is not a struct\n",nodeId->Line,nodeId->name);
     			fclose(fout);  
   				fclose(errdir);  
   				exit(1);
		}

        struct symbol* id = symTable[dim1][i];

        char* op1 = (char*)malloc(sizeof(char)*200);
        strcpy(op1,nodeId->name);

        char* opStr = (char*)malloc(sizeof(char)*200);
        strcpy(opStr,id->structName); //opStr, doubleO


        nodeId = p->child->brother->brother;
		
        dim1 = 0;
		if(nodeId->name[0]<'A' || nodeId->name[0]>'z') dim1 = 26;
		else dim1 = (nodeId->name[0]<='Z')? nodeId->name[0]-'A':nodeId->name[0]-'a';

        i=0;
        while (!symTable[dim1][i] || strcmp(nodeId->name,symTable[dim1][i]->word)) //Error checking
		{
			i++;
			if(i>=20)
			{
				FILE* errdir=NULL;  
     			errdir=fopen("stderr","w");  
     			fprintf(fout,"Error.");  
     			fprintf(errdir,"Line %d error: Struct %s has no element called %s\n",nodeId->Line,p->child->child->name,nodeId->name);
     			fprintf(stderr,"Line %d error: Struct %s has no element called %s\n",nodeId->Line,p->child->child->name,nodeId->name);  
     			fclose(fout);  
   				fclose(errdir);  
   				exit(1);
			}
		}
        id = symTable[dim1][i];

        int op2 = id->structMem; //op2, 0

        char* ret = (char*)malloc(sizeof(char)*200);
        strcpy(ret,"getelementptr inbounds (%struct.");
        strcat(ret,opStr);
		strcat(ret,",");
		strcat(ret,"%struct.");
        strcat(ret,opStr);
        strcat(ret,"* @");
        strcat(ret,op1);
        strcat(ret,", i32 0, i32 ");
        char indTmp = '0'+op2;
        char* ind = (char*)malloc(sizeof(char)*70); ind[0] = indTmp; ind[1] = '\0';
        strcat(ret,ind);
        strcat(ret,")");

        if (loadFlag)
        {
            char num[10];
            sprintf(num, "%d", rNum++);
            char* tmpReg = (char*)malloc(sizeof(char)*200);
            strcpy(tmpReg,"%r");
            strcat(tmpReg,num);

            fprintf(fout,"  %s = load i32, i32* %s, align 4\n",tmpReg,ret);
            return tmpReg;
        }
        else return ret;
    }
    else if (!strcmp(p->child->brother->name,"==")) //EXP->EXP == EXP
    {
        char* op1 = (char*)malloc(sizeof(char)*60);
        op1 = Exp(p->child);
        char* op2 = (char*)malloc(sizeof(char)*60);
        op2 = Exp(p->child->brother->brother);

        char num[10];
        sprintf(num, "%d", rNum++);
		
        char* tmpReg = (char*)malloc(sizeof(char)*60);
        strcpy(tmpReg,"%r");
        strcat(tmpReg,num);
        fprintf(fout,"  %s = icmp eq i32 %s, %s\n",tmpReg,op1,op2);
		return tmpReg;
    }
    else if (!strcmp(p->child->brother->name,">")) //EXP->EXP GREATER EXP
    {
        char* op1 = (char*)malloc(sizeof(char)*60);
        op1 = Exp(p->child);
        char* op2 = (char*)malloc(sizeof(char)*60);
        op2 = Exp(p->child->brother->brother);

        char num[10];
        sprintf(num, "%d", rNum++);
		
        char* tmpReg = (char*)malloc(sizeof(char)*60);
        strcpy(tmpReg,"%r");
        strcat(tmpReg,num);
        fprintf(fout,"  %s = icmp sgt i32 %s, %s\n",tmpReg,op1,op2);
		return tmpReg;
    }
    else if (!strcmp(p->child->brother->name,"<")) //EXP->EXP LESS EXP
    {
        char* op1 = (char*)malloc(sizeof(char)*60);
        op1 = Exp(p->child);
        char* op2 = (char*)malloc(sizeof(char)*60);
        op2 = Exp(p->child->brother->brother);

        char num[10];
        sprintf(num, "%d", rNum++);
		
        char* tmpReg = (char*)malloc(sizeof(char)*60);
        strcpy(tmpReg,"%r");
        strcat(tmpReg,num);
        fprintf(fout,"  %s = icmp slt i32 %s, %s\n",tmpReg,op1,op2);
		return tmpReg;
    }
    else if (!strcmp(p->child->brother->name,"&&")) //EXP->EXP LOGICAND EXP
    {
        char* op1 = (char*)malloc(sizeof(char)*60);
        op1 = Exp(p->child);
        char* op2 = (char*)malloc(sizeof(char)*60);
        op2 = Exp(p->child->brother->brother);

        int reg1 = rNum, reg2 = rNum+1; rNum+=2;
        fprintf(fout,"  %%r%d = icmp ne i1 %s, 0\n",reg1,op1);
        fprintf(fout,"  %%r%d = icmp ne i1 %s, 0\n",reg2,op2);

        int reg3 = rNum; rNum++;
        fprintf(fout,"  %%r%d = and i1 %%r%d, %%r%d\n",reg3,reg1,reg2);

        char num[10];
        sprintf(num, "%d", reg3);
        char* tmpReg = (char*)malloc(sizeof(char)*60);
        strcpy(tmpReg,"%r");
        strcat(tmpReg,num);

        return tmpReg;
    }
    else if (!strcmp(p->child->brother->name,"+")) //EXP ADD EXP
    {
        char* op1 = (char*)malloc(sizeof(char)*60);
        op1 = Exp(p->child);
        char* op2 = (char*)malloc(sizeof(char)*60);
        op2 = Exp(p->child->brother->brother);

        char num[10];
        sprintf(num, "%d", rNum++);
        char* tmpReg = (char*)malloc(sizeof(char)*60);
        strcpy(tmpReg,"%r");
        strcat(tmpReg,num);

        fprintf(fout,"  %s = add nsw i32 %s, %s\n",tmpReg,op1,op2);
        return tmpReg;
    }
    else if (!strcmp(p->child->brother->name,"-")) //EXP -> EXP - Exp
    {
        char* op1 = (char*)malloc(sizeof(char)*60);
        op1 = Exp(p->child);

        char* op2 = (char*)malloc(sizeof(char)*60);
        op2 = Exp(p->child->brother->brother);

        char num[10];
        sprintf(num, "%d", rNum++);
        char* tmpReg = (char*)malloc(sizeof(char)*60);
        strcpy(tmpReg,"%r");
        strcat(tmpReg,num);

        fprintf(fout,"  %s = sub nsw i32 %s, %s\n",tmpReg,op1,op2);
        return tmpReg;
    }
    else if (!strcmp(p->child->brother->name,"*")) //EXP MULT EXP
    {
        char* op1 = (char*)malloc(sizeof(char)*60);
        op1 = Exp(p->child);
        char* op2 = (char*)malloc(sizeof(char)*60);
        op2 = Exp(p->child->brother->brother);

        char num[10];
        sprintf(num, "%d", rNum++);
        char* tmpReg = (char*)malloc(sizeof(char)*60);
        strcpy(tmpReg,"%r");
        strcat(tmpReg,num);

        fprintf(fout,"  %s = mul nsw i32 %s, %s\n",tmpReg,op1,op2);
        return tmpReg;
    }
    else if (!strcmp(p->child->brother->name,"%"))//MOD srem
    {
        char* op1 = (char*)malloc(sizeof(char)*60);
        op1 = Exp(p->child);
        char* op2 = (char*)malloc(sizeof(char)*60);
        op2 = Exp(p->child->brother->brother);

        char num[10];
        sprintf(num, "%d", rNum++);
        char* tmpReg = (char*)malloc(sizeof(char)*60);
        strcpy(tmpReg,"%r");
        strcat(tmpReg,num);

        fprintf(fout,"  %s = srem i32 %s, %s\n",tmpReg,op1,op2);
        return tmpReg;
    }
    else if (!strcmp(p->child->brother->name,"&"))//BITAND
    {
        char* op1 = (char*)malloc(sizeof(char)*60);
        op1 = Exp(p->child);
        char* op2 = (char*)malloc(sizeof(char)*60);
        op2 = Exp(p->child->brother->brother);

        char num[10];
        sprintf(num, "%d", rNum++);
        char* tmpReg = (char*)malloc(sizeof(char)*60);
        strcpy(tmpReg,"%r");
        strcat(tmpReg,num);

        fprintf(fout,"  %s = and i32 %s, %s\n",tmpReg,op1,op2);
        sprintf(num, "%d", rNum++);
        strcpy(tmpReg,"%r");
        strcat(tmpReg,num);

        fprintf(fout,"  %s = icmp ne i32 %%r%d, 0\n",tmpReg,rNum-2);
        return  tmpReg;
    }
    else if (!strcmp(p->child->brother->name,"^"))//BITXOR
    {
        char* op1 = (char*)malloc(sizeof(char)*60);
        op1 = Exp(p->child);
        char* op2 = (char*)malloc(sizeof(char)*60);
        op2 = Exp(p->child->brother->brother);

        char num[10];
        sprintf(num, "%d", rNum++);
        char* tmpReg = (char*)malloc(sizeof(char)*60);
        strcpy(tmpReg,"%r");
        strcat(tmpReg,num);

        fprintf(fout,"  %s = xor i32 %s, %s\n",tmpReg,op1,op2);
        return tmpReg;
    }
    else if (!strcmp(p->child->brother->name,">>=")) //EXP SHIFTRA EXP
    {
        char* op1 = (char*)malloc(sizeof(char)*60);
        loadFlag = 0;
        op1 = Exp(p->child);
        loadFlag = 1;
        char* op2 = (char*)malloc(sizeof(char)*60);
        op2 = Exp(p->child->brother->brother);

       fprintf(fout,"%%r%d = load i32, i32* %s, align 4\n",rNum,op1);
        fprintf(fout,"  %%r%d = ashr i32 %%r%d, %s\n",rNum+1,rNum,op2);
        fprintf(fout,"  store i32 %%r%d, i32* %s, align 4\n",rNum+1,op1);
        rNum+=2;
        return NULL;
    }
    else if (!strcmp(p->child->brother->brother->name,"args")) //ID LP ARGS RP
    {
        argsFunc(p->child->brother->brother);

        char num[10];
        sprintf(num, "%d", callNum++);
        char* tmpReg = (char*)malloc(sizeof(char)*60);
        strcpy(tmpReg,"%call");
        strcat(tmpReg,num);

        TreeNode* id = p->child;

        fprintf(fout,"  %s = call i32 @%s(",tmpReg,id->name);
		int i;

        for (i=0;i<paraPoint-1;i++)
        {
            fprintf(fout,"i32 %s, ",paraArr[i]);
            free(paraArr[i]);
        }
        if (paraPoint>0)
        {
            fprintf(fout,"i32 %s",paraArr[paraPoint-1]);
            free(paraArr[i]);
            paraPoint = 0;
        }
        fprintf(fout,")\n");

        return tmpReg;
    }
}

void argsFunc(TreeNode* p) //ARGS for function call
{
    if (p->child->brother == NULL) //EXP
    {
        char* tmp = (char*)malloc(sizeof(char)*60);
        tmp = Exp(p->child);
        paraArr[paraPoint] = (char*)malloc(sizeof(char)*60);
        strcpy(paraArr[paraPoint],tmp);
        paraPoint++;
    }
    else //EXP COMMA ARGS
    {
        char* tmp = (char*)malloc(sizeof(char)*60);
        tmp = Exp(p->child);
        paraArr[paraPoint] = (char*)malloc(sizeof(char)*60);
        strcpy(paraArr[paraPoint],tmp);
        paraPoint++;

        argsFunc(p->child->brother->brother);
    }
}
