#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define	DEBUGER  CLOSE
#ifndef	DEBUGER 
#define	DEBUG(str)		printf("[DEBUG]ENTERING:%s\n",str);
#define	DEBUG2(str1,str2)	printf("[DEBUG]%s %s\n",str1,str2);
#else
#define	DEBUG(str)	
#define	DEBUG2(str1,str2)	
#endif 

#define	P(X)	printf(X);

typedef void(*fnp)();		//funcion pointer
#define	cell	int		//only for 32bits

cell CELL=sizeof(cell); 

#define	SYSPATH	".\\system.f"

#define	NEXT		tmpReg=(cell)*(IP++);goto *(cell*)tmpReg;
#define	_PUSH		*(++DP)=TOS;
#define	_POP		TOS=*(DP--);
#define	PUSH(X)		_PUSH; TOS=X;
#define	POP(X)		X=TOS; _POP;
#define	TMPLP_NEXT(X)	*(tmpLp++)=(cell*)X;

char EOS=' ';//end of string

#define	STACK_LEN	256
cell DS[STACK_LEN], RS[STACK_LEN], XS[STACK_LEN];//(data | return | X)stack
static	cell*RP;//stack pointer
static	cell*XP;//stack pointer	

cell *_showSTACK;
char *new_name=NULL;


cell**pushh;
cell**zbranchh;
cell**branchh;
cell**torr;
cell**casee;
cell**droprr;
cell**dropr44;
cell**doo;
cell**breakk;
cell**loopp;
cell**forr;
cell**nextt;


#include "adict.h"

#include "str.h"

#define	dictNum	3
word * dict[dictNum];
void dictIndexInit()
{
	dict[0]=immeDictHead;
	dict[1]=codeDictHead;
	dict[2]=colonDictHead;
}

int search_word(char *w)
{
	char check_code;
	check_code=computeCheckCode(w);
	dictIndexInit();
	int d=0;
	for (; d<dictNum; d++)
	{
		if (dict[d]==NULL) break;
		do
		{
			if (check_code==dict[d]->checkCode && !strcmp(dict[d]->name,w))
			{
				DEBUG2("success find:",w)
				if (d==0)
					((fnp)(dict[d]->addr))();
				else
					TMPLP_NEXT(dict[d]->addr);
				return 1;
			}
		}while(dict[d]=dict[d]->next);
	}
				
	return 0;

}



int compile(char *s)
{
	s=ignore_blankchar( s);
	
	new_name=NULL;
	if (*s==':')
	{
		s++;
		new_name=ignore_blankchar(s);
		s=new_name;
		s=split_word(s);
		s=ignore_blankchar(s);
	}
	char *w, *charp;
	int len;
	while (*s!=0)
	{
		w=s;
		if(*w=='"')
		{
			++w;
			while(1)
			{
				++s;
				if(*s=='"' && *(s+1)==' ')
				{
					 *(++s)='\0';
					 ++s;
					 break;
				}
			}
			len=strlen(w);
			charp=(char*)malloc(len+1);
			strcpy(charp,w);
			TMPLP_NEXT((cell**)charp);
			*(charp+len-1)='\0';
			continue;
		}
		s=split_word(s);

		if(!search_word(w) )
		{
			if (is_num(w))
			{//change to number
				TMPLP_NEXT(pushh);
				TMPLP_NEXT(atoi(w));
			}
			else
			{
				printf("[%s]?\n",w);
				return 0;
			}
		}
		s=ignore_blankchar(s);
	}
	*tmpLp=_showSTACK; 

	if (new_name!=NULL)
		colon(new_name,tmpList);
	tmpLp=tmpList;
	return 1;
}


void checkcmd(char*s)
{
	strcat(cmdstr," ;");
	s=ignore_blankchar(s);
	char c=*s;
	while ( *s!=';')
		s++;
	if (c==':') 
		s++;
	*s=0;
}


int main() 
{
pushh	=&&push;
zbranchh=&&zbranch;
branchh	=&&branch;
torr	=&&tor;
casee	=&&_casee;
droprr	=&&dropr;
dropr44	=&&dropr4;
doo	=&&_do;
breakk	=&&_break;
loopp	=&&_loop;
forr	=&&_for;
nextt	=&&_next;

static	register cell tmpReg=0;	
static	register cell TOS;	
static	register cell** IP;
static	register cell*DP;//stack pointer

	_showSTACK=&&showSTACK;
	word_call_addr=(&&call);
//	wordNeck=(&&_wordNeck);


	code("push",&&push);
	code("bye",&&bye);
	code("words",&&words);
	code(".\"",&&printstr);
	code(".",&&printnum);


	code("sameAs",&&sameAs);
	code("exec",&&exec);

	code("i",&&_i);
	code("continue",&&_continue);
	code("while",&&_while);

	code("branch",&&branch);
	code("0branch",&&zbranch);

	code("u>",&&uabove);
	code(">",&&above);
	code("==",&&equ);

	
	code(">r",&&tor);
	code("r>",&&rto);
	code("r@",&&rat);
	code("dropr",&&dropr);
	code("4dropr",&&dropr4);

	code(">x",&&tox);
	code("x>",&&xto);
	code("x@",&&xat);
	code("dropx",&&dropx);

	code("+",&&add);
	code("-",&&sub);
	code("*",&&mul);
	code("/",&&divv);
	code("++",&&add1);
	code("--",&&sub1);


	code("ret",&&ret);
	code(";",&&ret);

	code("swaps",&&swaps);
	code("swap",&&swap);
	code("drops",&&drops);
	code("drop",&&drop);
	code("over",&&over);
	code("dups",&&dups);
	code("dup",&&dup);



	//immeDict
	immediate("if",(cell**)_if);
	immediate("endif",(cell**)_endif);
	immediate("else",(cell**)_else);
	immediate("switch",(cell**)_switch);
	immediate("case",(cell**)_case);
	immediate("break",(cell**)_break);
	immediate("ends",(cell**)_ends);
	immediate("do",(cell**)__do);
	immediate("loop",(cell**)__loop);
	immediate("for",(cell**)__for);
	immediate("next",(cell**)__next);


	DP=DS-1;
	RP=RS-1;
	XP=XS-1;
	tmpLp=tmpList;

	FILE*fp;
	char ch;
	char*chp;
	char *loadInf="succeed";
loadsys:
	fp=fopen(SYSPATH,"r");
	if (fp==NULL)
		loadInf="FAIL";
	else
	{
		chp=cmdstr;
//*
		while(1)
		{
			ch=fgetc(fp);
			putchar(ch);
			*chp=ch;
			if (*(chp-1)==';' && is_blankchar(*(chp-2)) && (is_blankchar(ch) || ch==EOF) )
			{
				*chp='\0';//printf("%s\n",cmdstr);
				
				if (!compile(cmdstr))
				{
					loadInf="FAIL";
					break;
				}
				chp=cmdstr-1;
			}
			if(ch==EOF) break;
			chp++;
		}
/*/
		while (fgets(cmdstr,CMDSTR_LEN,fp))
		{
			printf("%s",cmdstr);
			checkcmd(cmdstr);
			if (!compile(cmdstr))
			{
				loadInf="FAIL";
				break;
			}
		}
//*/
	}
	fclose(fp);
	printf("\n-------------------------");
	printf(loadInf);	 printf(" to load system");
	printf("-------------------------\n");
	printf("asm-cforth version 0.1------made by ear\nplease input 'words' to see the dictionary\n");
init:
	DP=DS-1;
	RP=RS-1;
	XP=XS-1;
	tmpLp=tmpList;
	DEBUG("INIT()")
//	DS[0]=123;DS[1]=456;DP=DS+2;

showSTACK:
	printf("DS> ");
	cell*i;
	i=DS+1;
	if (DP>=DS)
	{
		while (i<=DP)
			printf("%d ",*i++);
		printf("%d ",TOS);
	}
	printf("\n");
//*
	printf("tmplist> ");
	cell** j=tmpList;
	while (*j != (&&showSTACK) )
		printf("%d ",*j++);
	printf("\n");
//*/
//*
	printf("RS> ");
	cell *k=RS;
	for (;k<=RP ;k++ )
		printf("%d ",*k);
	printf("\n");
//*/

cmd_line:
	printf(">>>");gets(cmdstr);
	checkcmd(cmdstr);
	if (!compile(cmdstr))
		goto init;
	else 
	{
		if (new_name !=NULL)
		//	goto cmd_line;
			goto showSTACK;

		DEBUG("entering: explain")
		IP=tmpList;
		NEXT
	}






//DATA STACK OPERATE 
push:	DEBUG("push")
	PUSH((cell)*IP++)	NEXT
dup:	DEBUG("dup")
	_PUSH			NEXT
over:	PUSH(*(DP-1))		NEXT
drop:	_POP			NEXT
drops:	DP--;			NEXT
dups:	tmpReg=*DP;*(++DP)=tmpReg;			NEXT
swap:	tmpReg=*DP; *DP=TOS; TOS=tmpReg;		NEXT
swaps:	tmpReg=*DP; *DP=*(DP-1); *(DP-1)=tmpReg;	NEXT
//RETURN STACK
tor:	POP(*(++RP))		NEXT
rto:	PUSH(*RP--)		NEXT
rat:	PUSH(*RP)		NEXT
dropr:	RP--;			NEXT
dropr4: RP-=4;			NEXT
//X STACK
tox:	POP(*(++XP))		NEXT
xto:	PUSH(*XP--)		NEXT
xat:	PUSH(*XP)		NEXT
dropx:	XP--;			NEXT
//+*- /
add1:	DEBUG("++")TOS++; NEXT
sub1:	DEBUG("--")TOS--; NEXT
add:	TOS+=(*DP); DP--;	NEXT
mul:	TOS*=(*DP); DP--;	NEXT
sub:	TOS=(*DP)-TOS; DP--;	NEXT
divv:	if (!TOS){printf("error: 0 / \n");goto init;}
	TOS=(*DP)/TOS; DP--;	NEXT

branch:		IP=(cell**)( (cell)IP+(cell)(*(IP)) );	NEXT

zbranch:	tmpReg=TOS;
		_POP
		if(tmpReg)
zbranch1:		IP++;
		else
zbranch2:		goto branch;
		NEXT

_casee:	DEBUG("case")
	if(*DP==TOS)
		{DP--; _POP; goto zbranch1;}
	else	{_POP; goto branch;}

_break:	DEBUG("break")
	IP=(cell**)*RP; IP--; goto branch;

_continue: DEBUG("break")
	IP=(cell**)*RP; IP--;
	IP=(cell**)( (cell)IP+(cell)(*(IP))-CELL );
	NEXT

_do:	DEBUG("do")
	*(++RP)=(cell)(++IP); NEXT

_loop:	DEBUG("loop")
	IP=(cell**)*RP; NEXT

_while:	DEBUG("while")
	tmpReg=TOS; _POP 
	if(tmpReg==0) goto _break;
	NEXT

_for:	DEBUG("for")//*
	if( !(*(DP-1)<*DP) ) 
	{
		DP-=2; _POP;
		IP=(cell**)( (cell)IP+(cell)(*(IP))+CELL );
		NEXT
	}//*/
	*(++RP)=TOS; _POP;
	*(++RP)=TOS; _POP;
	*(++RP)=TOS; _POP;
	goto _do;

_next:	DEBUG("next") //goto showSTACK;
	*(RP-1)+=*(RP-3);
	if(*(RP-1)<*(RP-2))
		goto _loop;
	NEXT

_i:	_PUSH; TOS=*(RP-1); NEXT



//cmp sign
equ:	TOS-=*DP--; TOS= !TOS;	NEXT
above:	TOS=((*DP--)>TOS);	NEXT
uabove:	TOS=((unsigned cell)*(DP--) > (unsigned cell)TOS);
	NEXT


exec:	tmpReg=TOS; _POP; goto *(cell*)tmpReg;
sameAs:	tmpReg=(cell)*IP; IP=(cell**)*RP--; goto *(cell*)tmpReg;

printstr:DEBUG("printstr")
	printf("%s\n",(char*)*IP++);
	NEXT
printnum:DEBUG("printnum")
	printf("%d ",TOS); _POP;
	NEXT
words:	dictIndexInit();
	int d;
	for (d=0;d<dictNum ; d++)
	{
		if (dict[d]==NULL) break;
		do{
			printf("[%d]%s ",dict[d]->checkCode,dict[d]->name);
		} while(dict[d]=dict[d]->next);
		printf("\n");
	}
	NEXT
		




//_wordNeck:	goto *(int*)word_call_addr;
ret:	DEBUG("entering: ret")	
	IP=(cell**)*RP--;	NEXT

call:	DEBUG("entering: call")
	*(++RP)=(cell)IP;
	IP=(cell**)( tmpReg+ wordNeck_len);
	NEXT

bye:	return 0;
}
