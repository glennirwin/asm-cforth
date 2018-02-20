#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#define	SYSPATH	"system.f"
//#define	DEBUG_ON  OFF

#ifndef	DEBUG_ON 
#define	DEBUG(str)		printf("[DEBUG]ENTERING:%s\n",str);
#define	DEBUG2(str1,str2)	printf("[DEBUG]%s %s\n",str1,str2);
#else
#define	DEBUG(str)	
#define	DEBUG2(str1,str2)	
#endif 

#define	P(N)	printf(N);

typedef void(*fnp)();		//function pointer

#define	CELL	long		// a cell is the basic data type and word size
int cellsize=sizeof(CELL); 

#define	NEXT		TMPR=(CELL)*IP;IP++;goto *(CELL*)TMPR;
#define	_PUSH		++DP;*DP=TOS;
#define	_POP		TOS=*DP;DP--;
#define	PUSH(N)		_PUSH; TOS=N;
#define	POP(N)		N=TOS; _POP;
#define	TMPLP_NEXT(N)	*(tmpLp++)=(CELL*)N;

char EOS=' ';//end of string

#define	STACK_LEN	256
CELL DS[STACK_LEN], RS[STACK_LEN], XS[STACK_LEN];  //(data | return | X)stack
CELL*RP;	//stack pointer
CELL*XP;	//stack pointer	

CELL *_showSTACK;
char *new_name=NULL;

CELL**pushh;
CELL**zbranchh;
CELL**branchh;
CELL**torr;
CELL**casee;
CELL**droprr;
CELL**dropr44;
CELL**doo;
CELL**breakk;
CELL**loopp;
CELL**forr;
CELL**nextt;

struct _word {
	char *name;
	char checkCode;
	CELL **addr;
	CELL **list;
	struct _word *next;
};
typedef struct _word word;

#define	CMDSTR_LEN	1024
char cmdstr[CMDSTR_LEN];
CELL* tmpList[CMDSTR_LEN/4];  // 
CELL** tmpLp=tmpList;
CELL** callList;  // List of instructions for a word call

word *immeDictHead=NULL, *codeDictHead=NULL, *colonDictHead=NULL;

char computeCheckCode(char* s) {
	char code=0;
	while (*s) code^=*s++;
	return *(--s)+code;
}

word * createWord(char* s, CELL** addr) {
	word *w=(word*)malloc(sizeof(word));
	w->checkCode=computeCheckCode(s);
	w->name=s;
	w->addr=addr;
	return w;
}

void immediate(char*s, CELL** addr) {
	word *w=createWord(s,addr);
	w->next=immeDictHead;
	immeDictHead=w;
}

CELL** code(char*s, CELL** addr) {
	word *w=createWord(s,addr);
	w->next=codeDictHead;
	codeDictHead=w;
	return w->addr;
}
CELL** word_call_addr;  // address of the call function to invoke forth words

void colon(char* s, CELL** addr) {  // add to dictionary

	printf("adding to dict: %s @%p\n",s,addr);

	word *w=(word*)malloc(sizeof(word));
	w->checkCode=computeCheckCode(s);

	w->name=(char*)malloc(strlen(s)+1);
	strcpy(w->name,s);

	CELL n=(CELL)tmpLp-(CELL)tmpList;
	printf("n=%ld\n",n);

	printf("tmplist> ");
	CELL** j=tmpList;
	int i=0;
	while (i<n/8) { printf("%p ",(void *)*j++); i++; }
	printf("\n");

	w->addr=word_call_addr;
	w->list=(CELL**)malloc(n);
	memcpy(w->list, addr, n);   // Copy the tmpList
	printf("added to dict: %s @%p\n",s,w->addr);
	
	w->next=colonDictHead;
	colonDictHead=w;
}

void markAddr(){ *(++XP)=(CELL)tmpLp++;}

void _if()	{TMPLP_NEXT(zbranchh);	markAddr();}
void _endif()	{*(CELL*)*XP=(CELL)tmpLp-(CELL)*XP;	XP--;}
void _else() {
	TMPLP_NEXT(branchh);
	tmpLp++; _endif();
	tmpLp--; markAddr();
}

void _switch()	{TMPLP_NEXT(doo); *(++RP)=0; markAddr();}
void _case()	{TMPLP_NEXT(casee); markAddr();}
void _break()	{TMPLP_NEXT(breakk); if(*RP==0){_endif();} }
void _ends() {
	TMPLP_NEXT((*RP==4?dropr44:droprr)); 
	tmpLp--; _endif(); tmpLp++;
	RP--;
}

void __do()	{TMPLP_NEXT(doo); *(++RP)=1; markAddr();}
void __loop()	{TMPLP_NEXT(loopp); _ends();}
void __for()	{TMPLP_NEXT(forr); *(++RP)=4; markAddr();}
void __next()	{TMPLP_NEXT(nextt); _ends();}

int is_blankchar(char c)	{ return (c==' ' || c=='	' || c==10 || c==13 ); }

char * ignore_blankchar(char *s) {
	while (is_blankchar(*s)) s++;
	return s;
}

char * until_wordend(char *s) {
	while ( !is_blankchar(*s)  && *s!='\0') s++;
	return s;
}

char * split_word(char *s) {
	s=until_wordend(s);
	if (*s=='\0') return s;
	*s='\0';
	s++;
	return s;
}

int is_num(char *s) {
	if(*s=='-') s++;
	while (*s !=0) {
		if (!isdigit(*s)) return 0;
		s++;
	}
	return 1;
}

#define	dictNum	3
word * dict[dictNum];
void dictIndexInit() {
	dict[0]=immeDictHead;
	dict[1]=codeDictHead;
	dict[2]=colonDictHead;
}

long search_word(char *w) {
	char check_code;
	check_code=computeCheckCode(w);
	dictIndexInit();
	for (int d=0; d<dictNum; d++) {
		if (dict[d]==NULL) break;
		do {
			if (check_code==dict[d]->checkCode && !strcmp(dict[d]->name,w)) {
				DEBUG2("found:",w)
				callList = dict[d]->list;
				TMPLP_NEXT(dict[d]->addr);
				return 1;
			}
		} while (dict[d] = dict[d]->next);
	}		
	return 0;
}

int compile(char *s) {
	s=ignore_blankchar( s);
	
	new_name = NULL;
	if (*s == ':') {
		s++;
		new_name=ignore_blankchar(s);
		DEBUG2("defining word:",new_name)
		s=new_name;
		s=split_word(s);
		s=ignore_blankchar(s);
	}
	char *w, *charp;
	int len;
	while (*s != 0) {
		w=s;
		if (*s=='/') {
			if (*(s+1)=='/') {
				do { s++; } while (!(*s==10 || *s==13));
			}
			else if(*(s+1)=='*') {
				do { s++; } while(!(*s=='/' && *(s-1)=='*'));
				s++;
			}
			s=ignore_blankchar(s);
			w=s;
		} else if(*w=='"') {
			++w;
			while(1) {
				++s;
				if(*s=='"' && *(s+1)==' ') {
					 *(++s)='\0';
					 ++s;
					 break;
				}
			}
			len=strlen(w);
			charp=(char*)malloc(len+1);
			strcpy(charp,w);
			TMPLP_NEXT((CELL**)charp);
			*(charp+len-1)='\0';
			continue;
		}
		s=split_word(s);

		if(!search_word(w) ) {
			if (is_num(w)) {	//change to number
				TMPLP_NEXT(pushh);
				TMPLP_NEXT(atol(w));
			} else {
				printf("[%s]? unknown word\n",w);
				return 0;
			}
		}
		s=ignore_blankchar(s);
	}
	*tmpLp=_showSTACK; 

	if (new_name!=NULL) colon(new_name,tmpList);
	tmpLp=tmpList;
	return 1;
}

void checkcmd(char *s) {
	strcat(cmdstr," ;");
	s=ignore_blankchar(s);
	char c=*s;
	while ( *s != ';') s++;
	if (c == ':') s++;
	*s=0;
}

int main() {
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
	
	register CELL TOS=0;
	register CELL TMPR=0;	//eax?
	register CELL*DP=0;     //stack pointer
	register CELL**IP=0;

	_showSTACK=&&showSTACK;
	word_call_addr=(&&call);

	code("push",&&push);
	code("bye",&&bye);
	code("words",&&words);
	code("timeStart",&&timeStart);
	code("timeEnd",&&timeEnd);
	code("malloc",&&_malloc);

	code(".f",&&printfloat);
	code(".\"",&&printstr);
	code(".",&&printnum);

	code("sameAs",&&sameAs);
	code("exec",&&exec);

	code("i",&&_i);
	code("continue",&&_continue);
	code("while",&&_while);
	code("_loop",&&_loop);

	code("branch",&&branch);
	code("0branch",&&zbranch);

	code("c!",&&cwrite);
	code("c@",&&cread);
	code("!",&&write);
	code("@",&&read);

	code("u>",&&uabove);
	code(">",&&above);
	code("<",&&below);
	code("==",&&equ);
	code("!=",&&nequ);

	
	code(">r",&&tor);
	code("r>",&&rto);
	code("r@",&&rat);
	code("dropr",&&dropr);
	code("4dropr",&&dropr4);

	code(">x",&&tox);
	code("x>",&&xto);
	code("x@",&&xat);
	code("xdrop",&&xdrop);
	code(">>x",&&varx);
	code("x4",&&x4);
	code("x3",&&x3);
	code("x2",&&x2);
	code("x1",&&xat);

	code("(",&&parenl);
	code(")",&&parenr);
	code("=",&&assign);
	code("+",&&add);
	code("-",&&sub);
	code("*",&&mul);
	code("/",&&divv);
	code("%",&&mod);
	code("mod",&&mod);
	code("++s",&&adds1);
	code("--s",&&subs1);
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
	immediate("if",(CELL**)_if);
	immediate("endif",(CELL**)_endif);
	immediate("else",(CELL**)_else);
	immediate("switch",(CELL**)_switch);
	immediate("case",(CELL**)_case);
	immediate("break",(CELL**)_break);
	immediate("ends",(CELL**)_ends);
	immediate("do",(CELL**)__do);
	immediate("loop",(CELL**)__loop);
	immediate("for",(CELL**)__for);
	immediate("next",(CELL**)__next);

	DP=DS-1;
	RP=RS-1;
	XP=XS-1;
	tmpLp=tmpList;

	FILE *fp;
	char ch=0;
	char *chp;
	char *loadInf="success";
	char *getsRet;

loadsys:
	fp=fopen(SYSPATH,"r");
	if (fp==NULL)
		loadInf="failed";
	else {
		while(1) {
			while(ch!=':' && ch!=EOF) {
				ch=fgetc(fp);  // putchar(ch);
			}
			if(ch==EOF) break;
			chp=cmdstr;
			while (1) {
				*chp=ch;
				ch=fgetc(fp);    // putchar(ch);
				if (*chp==';' && (is_blankchar(ch) || ch==EOF) ) {
					*(chp+1)='\0';
					if (compile(cmdstr)) break;
					loadInf="failed";
					goto fileclose;					
				}
				chp++;
			}
		}
	}

fileclose:
	fclose(fp);
	printf("---------------------- System load: %s ----------------------\n",loadInf);
	printf("asm-cforth version 0.2(ear) - input 'words' to see the dictionary\n");
init:
	DP=DS-1;
	RP=RS-1;
	XP=XS-1;
	tmpLp=tmpList;
	DEBUG("INIT()")

showSTACK:
	printf("tmplist> ");
	CELL** j=tmpList;
	while (*j != (&&showSTACK) ) printf("%p ",(void *)*j++);
	printf("\n");

	printf("DS> ");
	CELL*i;
	i=DS+1;
	if (DP>=DS) {
		while (i<=DP) printf("%ld ",*i++);
		printf("%ld ",TOS);
	}
	printf("\n");

	printf("RS> ");
	CELL *k=RS;
	for (;k<=RP ;k++ ) printf("%ld ",*k);
	printf("\n");

	printf("XS> ");
	CELL *L=XS;
	for (;L<=XP ;L++ ) printf("%ld ",*L);
	printf("\n");


cmd_line:
	printf(">>>");	getsRet=fgets(cmdstr,CMDSTR_LEN, stdin);
	checkcmd(cmdstr);
	if (!compile(cmdstr)) goto init;
	else {
		if (new_name != NULL) {
		//	goto cmd_line;
			goto showSTACK;
		}
		//printf("tmpList=%p",tmpList);
		IP=tmpList;
		NEXT
	}

//DATA STACK OPERATE 
push:	DEBUG("push")    // -- N
	PUSH((CELL)*IP)	
	IP++;
	NEXT

dup:	DEBUG("dup")   // N -- N N
	_PUSH
	NEXT

over:	//N1 N2 -- N1 N2 N1
	TMPR=*DP;
	_PUSH
	TOS=TMPR;
	NEXT

drop:	//N -- 
	_POP
	NEXT

drops:	//N1 N2 -- N2
	DP--;
	NEXT

dups:	//N1 N2 -- N1 N1 N2
	TMPR=*DP;
	DP++;
	*DP=TMPR;
	NEXT

swap:	//N1 N2 -- N2 N1
	TMPR=*DP;
	*DP=TOS;
	TOS=TMPR;	
	NEXT

swaps:	//N1 N2 N3 -- N2 N1 N3
	TMPR=*DP;
	*DP=*(DP-1);
	*(DP-1)=TMPR;
	NEXT

//RETURN STACK
tor:	//N --//>r
	RP++;
	POP(*RP)
	NEXT

rto:	// -- N //r>
	PUSH(*RP)
	RP--;
	NEXT

rat:	// -- N //r@
	PUSH(*RP)	NEXT
dropr:	//addr -R- 
	RP--;	NEXT
dropr4: RP-=4;	NEXT
//X STACK
tox:	XP++;
	POP(*XP)
	NEXT

xto:	PUSH(*XP)
	XP--;
	NEXT

xat:	PUSH(*XP)
	NEXT

//+*-/=
add1:	TOS++;	NEXT
sub1:	TOS--;	NEXT
adds1:	(*DP)++;	NEXT
subs1:	(*DP)--;	NEXT
//N1 N2 -- N
add:	TOS+=(*DP); DP--; NEXT
sub:	TMPR=TOS;
	_POP
	TOS-=TMPR;
	NEXT

mul:	TMPR=*DP;
	DP--;
	TMPR*=TOS;
	TOS=TMPR;
	NEXT

divv:	if (!TOS){printf("[error]: 0/ \n");goto init;}
	TMPR=*DP;
	DP--;
	TOS=TMPR/TOS;
	NEXT

mod:	if (!TOS){printf("[error]: 0/ \n");goto init;}
	TMPR=*DP;
	DP--;
	TOS=TMPR%TOS;
	NEXT

assign:	//N -- 
	TMPR=(CELL)*IP;
	TMPR+=9;		//neck_len+push_len=9
	*(CELL*)TMPR=TOS;
	_POP
	IP++;
	NEXT

write:	DEBUG("!")//N ADDR -- 
	TMPR=TOS;
	_POP
	*(CELL*)TMPR=TOS;
	_POP;
	NEXT

read:	DEBUG("@")//ADDR -- N
	TMPR=TOS;
	TOS=*(CELL*)TMPR;
	NEXT

cwrite:	DEBUG("c!")//N ADDR -- 
	TMPR=TOS;
	_POP
	*(char*)TMPR=(char)TOS;
	_POP;
	NEXT

cread:	DEBUG("c@")//ADDR -- N
	TMPR=TOS;
	TOS=*(char*)TMPR;
	NEXT

branch:// -- 
	TMPR=(CELL)*IP;
	IP=(CELL**)( (CELL)IP+TMPR );
	NEXT

zbranch://FLAG -- 
	TMPR=TOS;
	_POP
	if(TMPR)
zbranch1:	IP++;
	else
zbranch2:	goto branch;
	NEXT

_casee:	DEBUG("case")//N N' -- (N==N'?  || N )
	if(*DP==TOS) {DP--; _POP; goto zbranch1;}
	else {_POP; goto branch;}

_break:	DEBUG("break")
	IP=(CELL**)*RP; IP--; goto branch;

_continue: DEBUG("continue")
	IP=(CELL**)*RP; IP--;
	IP=(CELL**)*IP; IP--;
	//IP=(CELL**)( (CELL)IP+(CELL)(*(IP))-cell );
	NEXT

_do:	DEBUG("do")// -R- ADDR
	RP++;
	IP++;
	*RP=(CELL)IP;
	NEXT

_loop:	DEBUG("loop")
	IP=(CELL**)*RP;
	NEXT

_while:	DEBUG("while")  //FLAG --
	TMPR=TOS;
	_POP 
	if(TMPR==0) goto _break;
	NEXT

_for:	DEBUG("for")  //BEGIN UNTIL STEP -- 
	TMPR=*DP;
	if( *(DP-1)==TMPR ) {
		DP-=2; _POP; RP+=4; goto branch;
		//IP=(CELL**)( (CELL)IP+(CELL)(*(IP))+cell ); NEXT
	}
	*(++RP)=TOS; _POP;
	*(++RP)=TOS; _POP;
	*(++RP)=TOS; _POP;
	goto _do;

_next:	DEBUG("next") 
	TMPR=*(RP-3);	*(RP-1)+=TMPR;//BEGIN += STEP
	TMPR=*(RP-2);
	if(*(RP-1)!=TMPR)
		goto _loop;
	NEXT

_i:	// -- i
	_PUSH; TOS=*(RP-1); NEXT

_malloc:
	TOS=(CELL)malloc(TOS);
	if (TOS) NEXT;

	printf("[error]:malloc \n");
	goto init;

parenl:	// -R- ADDR
	*(++RP)=(CELL)DP;
	NEXT
parenr:	// -- N		// ADDR -R-
	TMPR=(CELL)DP;
	TMPR--;
	TMPR-=*RP;
	RP--;
	TMPR/=cellsize;
	_PUSH;
	TOS=TMPR;
	//TOS=((CELL)DP-1-*(RP--))/cell;
	NEXT
varx:
//	TMPR=TOS;
	while (TOS--) {
		*(++XP)=*(DP--);
	}
//	*(++XP)=TMPR;
	_POP; NEXT
xdrop:
	XP-=TOS; _POP; NEXT
x2:
	PUSH(*(XP-1)); NEXT
x3:
	PUSH(*(XP-2)); NEXT
x4:
	PUSH(*(XP-3)); NEXT

//cmp: !=  ==  >  <  u>
//N1 N2 -- FLAG
nequ:	TOS-=*DP--;	NEXT
equ:	TOS-=*DP--; TOS= !TOS;	NEXT
above:	TOS=((*DP--)>TOS);	NEXT
below:	TOS=((*DP--)<TOS);	NEXT
uabove:	TOS=((unsigned CELL)*(DP--) > (unsigned CELL)TOS);
	NEXT

exec:	TMPR=TOS; _POP; goto *(CELL*)TMPR;
sameAs:	TMPR=(CELL)*IP;
	IP=(CELL**)*RP;
	RP--;
	goto *(CELL*)TMPR;

printstr:DEBUG("printstr")
	printf("%s",(char*)*IP++);
	NEXT
printnum:DEBUG("printnum")
	printf("%ld ",TOS); _POP;
	NEXT

printfloat:DEBUG("printfloat")
	_PUSH
	printf("%f ",*(float*)DP);
	DP--;
	_POP
	NEXT

words:	dictIndexInit();
	for (int d=0; d<dictNum ; d++) {
		if (dict[d]==NULL) break;
		do { printf("%s ",dict[d]->name);
		} while(dict[d]=dict[d]->next);
		printf("\n");
	}
	NEXT
 
timeStart:
	*(++XP)=clock();
	NEXT;
timeEnd:
	_PUSH;
	*(float*)(++DP)=((float)(clock()-*(XP--))/CLOCKS_PER_SEC) ;
	_POP;
	NEXT;

ret:	DEBUG("entering: ret")	
	IP=(CELL**)*RP--;	
	NEXT

call:	DEBUG("entering: call")
	*(++RP)=(CELL)TMPR;
	printf("retp=%p ",RP);
	IP=(CELL**)callList;
	printf("IP=%p\n",IP);

	printf("RS> ");
	CELL *kk=RS;
	for (;kk<=RP ;kk++ ) printf("%ld ",*kk);
	printf("\n");

	NEXT

bye:	return 0;
}
