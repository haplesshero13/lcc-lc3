%{
/****************************************************************************
*
* lc3.md -- the LCC back end code for LC-3
*
*  "Copyright (c) 2003 by Ajay Ladsaria."
* 
*  See the CPYRIGHT file regarding the copyright that governs LCC.
*  The structure of this code is based on the other back ends of LCC.
*  
*  IN NO EVENT SHALL THE AUTHOR BE LIABLE TO ANY PARTY FOR DIRECT, 
*  INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT 
*  OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE AUTHOR 
*  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*  
*  THE AUTHOR SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT 
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
*  A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS" 
*  BASIS, AND THE AUTHOR NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, 
*  UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
*
*  Author:		Ajay Ladsaria 
*  Version:		1
*  Creation Date:	28 November 2003
*  Filename:		lc3.md
*  History:		
* 	SSL	1	28 November 2003
* 		Copyright notices and Warranty disclaimer added.
*
******************************************************************************/

#define INTTMP 0x0000000f
#define INTVAR 0x00000000
#define FLTTMP 0x000f0ff0
#define FLTVAR 0xfff00000

#define INTRET 0x00000080
#define FLTRET 0x00000003


#include "c.h"
#define NODEPTR_TYPE Node
#define OP_LABEL(p) ((p)->op)
#define LEFT_CHILD(p) ((p)->kids[0])
#define RIGHT_CHILD(p) ((p)->kids[1])
#define STATE_LABEL(p) ((p)->x.state)




/******************************************************************************
 * 	LC3 Instruction Macros
 * 		The following macros are here to make it easier and clearer to emit
 * 		basic and slightly non-basic LC3 instructions in the rest of the
 * 		back-end.
 *****************************************************************************/
#define lc3_push(j)  \
	print("ADD R6, R6, #-1\n"); \
	print("STR R%d, R6, #0\n",j);

#define lc3_pop(j)  \
	print("LDR R%d, R6, #0\n",j); \
	print("ADD R6, R6, #1\n"); 

#define lc3_neg(j)  \
	print("NOT R%d, R%d\n",j,j);  \
	print("ADD R%d, R%d, #1\n",j,j);

#define lc3_not(j)  \
	print("NOT R%d, R%d\n",j,j);  

#define lc3_add(i,j,k)  \
	print("ADD R%d, R%d, R%d\n",i,j,k);

#define lc3_sub(i,j,k)  \
	print("NOT R%d, R%d\n",k,k);  \
	print("ADD R%d, R%d, #1\n",k,k);  \
	print("ADD R%d, R%d, R%d\n",i,j,k);

#define lc3_addimm(i,j,k)  \
	print("ADD R%d, R%d, #%d\n",i,j,k);

#define lc3_and(i,j,k)  \
	print("AND R%d, R%d, R%d\n",i,j,k);

#define lc3_andimm(i,j,k)  \
	print("AND R%d, R%d, #%d\n",i,j,k);

#define lc3_load(i,j,k)  \
	print("LDR R%d, R%d, #%d\n",i,j,k);

#define lc3_store(i,j,k)  \
	print("STR R%d, R%d, #%d\n",i,j,k);

#define lc3_brz(i)  \
	print("BRz L%d\n",i);

#define lc3_brp(i)  \
	print("BRp L%d\n",i);

#define lc3_brn(i)  \
	print("BRn L%d\n",i);

#define lc3_brnz(i)  \
	print("BRnz L%d\n",i);

#define lc3_brnp(i)  \
	print("BRnp L%d\n",i);

#define lc3_brzp(i)  \
	print("BRzp L%d\n",i);

#define lc3_br(i)  \
	print("BRnzp L%d\n",i);

#define lc3_lab(i)  \
	print("L%d\n",i);

static void address(Symbol, Symbol, long);
static void blkfetch(int, int, int, int);
static void blkloop(int, int, int, int, int, int[]);
static void blkstore(int, int, int, int);
static void defaddress(Symbol);
static void defconst(int, int, Value);
static void defstring(int, char *);
static void defsymbol(Symbol);
static void doarg(Node);
static void emit2(Node);
static void export(Symbol);
static void clobber(Node);
static void function(Symbol, Symbol [], Symbol [], int);
static void global(Symbol);
static void import(Symbol);
static void local(Symbol);
static void progbeg(int, char **);
static void progend(void);
static void segment(int);
static void space(int);
static void target(Node);

static Symbol ireg[32], freg2[32], d6;
static Symbol iregw, freg2w;
static int tmpregs[] = {0, 1, 2};
static Symbol blkreg;

static int first_function = 1;
static int pic;

static int cseg;

/*
typedef struct {
	char name[30];
	int	 size;	//# of ints,chars,shorts in array
} Xglobal_stack;
*/

%}
%start stmt
%term CNSTI1=1045
%term CNSTP1=1047
%term CNSTU1=1046

%term ARGB=41
%term ARGI1=1061
%term ARGP1=1063
%term ARGU1=1062

%term ASGNB=57
%term ASGNI1=1077
%term ASGNP1=1079
%term ASGNU1=1078

%term INDIRB=73
%term INDIRI1=1093
%term INDIRP1=1095
%term INDIRU1=1094

%term CVII1=1157
%term CVIU1=1158
%term CVPU1=1174

%term CVUI1=1205
%term CVUP1=1207
%term CVUU1=1206

%term NEGI1=1221

%term CALLB=217
%term CALLI1=1237
%term CALLP1=1239
%term CALLU1=1238
%term CALLV=216

%term RETI1=1269
%term RETP1=1271
%term RETU1=1270
%term RETV=248

%term ADDRGP1=1287
%term ADDRFP1=1303
%term ADDRLP1=1319

%term ADDI1=1333
%term ADDP1=1335
%term ADDU1=1334

%term SUBI1=1349
%term SUBP1=1351
%term SUBU1=1350

%term LSHI1=1365
%term LSHU1=1366

%term MODI1=1381
%term MODU1=1382

%term RSHI1=1397
%term RSHU1=1398

%term BANDI1=1413
%term BANDU1=1414

%term BCOMI1=1429
%term BCOMU1=1430

%term BORI1=1445
%term BORU1=1446

%term BXORI1=1461
%term BXORU1=1462

%term DIVI1=1477
%term DIVU1=1478

%term MULI1=1493
%term MULU1=1494

%term EQI1=1509
%term EQU1=1510

%term GEI1=1525
%term GEU1=1526

%term GTI1=1541
%term GTU1=1542

%term LEI1=1557
%term LEU1=1558

%term LTI1=1573
%term LTU1=1574

%term NEI1=1589
%term NEU1=1590

%term JUMPV=584

%term LABELV=600

%term LOADB=233
%term LOADI1=1253
%term LOADP1=1255
%term LOADU1=1254

%term VREGP=711
%%
reg:  INDIRI1(VREGP)     "# read register\n"
reg:  INDIRU1(VREGP)     "# read register\n"
reg:  INDIRP1(VREGP)     "# read register\n"

stmt: ASGNI1(VREGP,reg)  "# write register\n"
stmt: ASGNU1(VREGP,reg)  "# write register\n"
stmt: ASGNP1(VREGP,reg)  "# write register\n"

stmt: reg  ""


c6: CNSTP1         "%a"                range(a,-32,31)
c6: CNSTI1         "%a"                range(a,-32,31)
c6: CNSTU1         "%a"                range(a,-32,31)

addr: ADDI1(reg,c6)  "%0, #%1"
addr: ADDU1(reg,c6)  "%0, #%1"
addr: ADDP1(reg,c6)  "%0, #%1"
addr: reg   "%0, #0"

addr: ADDRFP1 "R5, #%a"				range(a, -32, 31)
addr: ADDRLP1 "R5, #%a"				range(a, -32, 31)

reg: ADDRFP1  "# formal stack\n"	2
reg: ADDRLP1  "# local stack\n"	    2
reg: ADDRGP1  "# global var\n"		2

reg: CNSTI1   "# reg\n"  2
reg: CNSTU1   "# reg\n"  2
reg: CNSTP1   "# reg\n"  2

spill: ADDRLP1 "%a"					!range(a,-32,31)
stmt: ASGNI1(spill,reg)  "#str %1, %0\n"  3
stmt: ASGNU1(spill,reg)  "#str %1, %0\n"  3
stmt: ASGNP1(spill,reg)  "#str %1, %0\n"  3

stmt: ASGNI1(addr,reg)   "str %1, %0\n"  1
stmt: ASGNU1(addr,reg)   "str %1, %0\n"  1
stmt: ASGNP1(addr,reg)   "str %1, %0\n"  1

reg:  INDIRI1(addr)      "ldr %c, %0\n"  1
reg:  INDIRU1(addr)      "ldr %c, %0\n"  1
reg:  INDIRP1(addr)      "ldr %c, %0\n"  1

reg:  CVPU1(INDIRI1(addr))     "ldr %c, %0\n"  1
reg:  CVIU1(INDIRU1(addr))     "ldr %c, %0\n"  1
reg:  CVUI1(INDIRU1(addr))     "ldr %c, %0\n"  1

reg: DIVI1(reg,reg)   "#\n"   15
reg: DIVU1(reg,reg)   "#\n"   15
reg: MODI1(reg,reg)   "#\n"   15
reg: MODU1(reg,reg)   "#\n"   15
reg: MULI1(reg,reg)   "#\n"   10
reg: MULU1(reg,reg)   "#\n"   10

c5: CNSTP1            "%a"                range(a,-16,15)
c5: CNSTI1            "%a"                range(a,-16,15)
c5: CNSTU1            "%a"                range(a,-16,15)

subc5: CNSTP1         "%a"                range(a,-15,16)
subc5: CNSTI1         "%a"                range(a,-15,16)
subc5: CNSTU1         "%a"                range(a,-15,16)

reg: ADDI1(reg,c5)    "add %c, %0, #%1\n"  1
reg: ADDP1(reg,c5)    "add %c, %0, #%1\n"  1
reg: ADDU1(reg,c5)    "add %c, %0, #%1\n"  1
reg: BANDI1(reg,c5)   "and %c, %0, #%1\n"  1
reg: BANDU1(reg,c5)   "and %c, %0, #%1\n"  1

reg: ADDI1(reg,reg)   "add %c, %0, %1\n"   1
reg: ADDP1(reg,reg)   "add %c, %0, %1\n"   1
reg: ADDU1(reg,reg)   "add %c, %0, %1\n"   1
reg: BANDI1(reg,reg)  "and %c, %0, %1\n"   1
reg: BANDU1(reg,reg)  "and %c, %0, %1\n"   1

reg: BORI1(reg,reg)   "# \n"   5
reg: BORU1(reg,reg)   "# \n"   5
reg: BXORI1(reg,reg)  "# \n"   10
reg: BXORU1(reg,reg)  "# \n"   10

reg: SUBI1(reg,subc5) "add %c, %0, #-%1\n"  1
reg: SUBP1(reg,subc5) "add %c, %0, #-%1\n"  1
reg: SUBU1(reg,subc5) "add %c, %0, #-%1\n"  1

reg: SUBI1(reg,reg)   "#\n"  3
reg: SUBP1(reg,reg)   "#\n"  3
reg: SUBU1(reg,reg)   "#\n"  3

reg: LSHI1(reg,reg)   "#\n"  6
reg: LSHU1(reg,reg)   "#\n"  6
reg: RSHI1(reg,reg)   "#\n"  15
reg: RSHU1(reg,reg)   "#\n"  15

reg: BCOMI1(reg)  "not %c,%0\n"   1
reg: BCOMU1(reg)  "not %c,%0\n"   1
reg: NEGI1(reg)   "not %c,%0\nadd %c,%c,#1\n"  2

reg: LOADI1(reg)  "add %c, %0, #0\n"  move(a)
reg: LOADU1(reg)  "add %c, %0, #0\n"  move(a)
reg: LOADP1(reg)  "add %c, %0, #0\n"  move(a)

reg: CVPU1(reg)   "add %c, %0, #0\n"  move(a)
reg: CVUP1(reg)   "add %c, %0, #0\n"  move(a)
reg: CVII1(reg)   "add %c, %0, #0\n"  move(a)
reg: CVIU1(reg)   "add %c, %0, #0\n"  move(a)
reg: CVUI1(reg)   "add %c, %0, #0\n"  move(a)
reg: CVUU1(reg)   "add %c, %0, #0\n"  move(a)

stmt: LABELV	     "LC3_GFLAG %a LC3_GFLAG .FILL lc3_%a\nlc3_%a\n"
jaddr: ADDRGP1	     "%a"
stmt: JUMPV(jaddr)   ".LC3GLOBAL %0 0\nLDR R0, R0, #0\nJMP R0\n"  
stmt: JUMPV(reg)     "LDR %0, %0, #0\nJMP %0 ;unconditional\n"  

stmt: EQI1(reg,reg)  "#conditional branch\n"   1
stmt: EQU1(reg,reg)  "#conditional branch\n"   1
stmt: GEI1(reg,reg)  "#conditional branch\n"   1
stmt: GEU1(reg,reg)  "#conditional branch\n"   1
stmt: GTI1(reg,reg)  "#conditional branch\n"   1
stmt: GTU1(reg,reg)  "#conditional branch\n"   1
stmt: LEI1(reg,reg)  "#conditional branch\n"   1
stmt: LEU1(reg,reg)  "#conditional branch\n"   1
stmt: LTI1(reg,reg)  "#conditional branch\n"   1
stmt: LTU1(reg,reg)  "#conditional branch\n"   1
stmt: NEI1(reg,reg)  "#conditional branch\n"   1
stmt: NEU1(reg,reg)  "#conditional branch\n"   1

reg:  CALLI1(jaddr)  ".LC3GLOBAL %0 0\nLDR R0, R0, #0\njsrr R0\nLDR R7, R6, #0\nADD R6, R6, #1\n"  1
reg:  CALLP1(jaddr)  ".LC3GLOBAL %0 0\nLDR R0, R0, #0\njsrr R0\nLDR R7, R6, #0\nADD R6, R6, #1\n"  1
reg:  CALLU1(jaddr)  ".LC3GLOBAL %0 0\nLDR R0, R0, #0\njsrr R0\nLDR R7, R6, #0\nADD R6, R6, #1\n"  1
stmt: CALLV(jaddr)   ".LC3GLOBAL %0 0\nLDR R0, R0, #0\njsrr R0\n"  1

reg:  CALLI1(reg)    "jsrr %0\nLDR R7, R6, #0\nADD R6, R6, #1\n"  1
reg:  CALLP1(reg)    "jsrr %0\nLDR R7, R6, #0\nADD R6, R6, #1\n"  1
reg:  CALLU1(reg)    "jsrr %0\nLDR R7, R6, #0\nADD R6, R6, #1\n"  1
stmt: CALLV(reg)     "jsrr %0\n"  1

stmt: RETI1(reg)     "# ret\n"  1
stmt: RETU1(reg)     "# ret\n"  1
stmt: RETP1(reg)     "# ret\n"  1
stmt: RETV(reg)      "# ret\n"  1

stmt: ARGI1(reg)     "ADD R6, R6, #-1\nSTR %0, R6, #0\n"  2
stmt: ARGP1(reg)     "ADD R6, R6, #-1\nSTR %0, R6, #0\n"  2
stmt: ARGU1(reg)     "ADD R6, R6, #-1\nSTR %0, R6, #0\n"  2

stmt: ARGB(INDIRB(reg))       "#argb \n"  10
stmt: ASGNB(reg,INDIRB(reg))  "#asgnb\n"  10
%%

#define ck(i) return (i) ? 0 : LBURG_MAX


/******************************************************************************
 * 	prologue(p,&x,&y,&z,&yflag,&destflag)
 * 		This macro does nothing if the destination register is
 * 		independent from both of the source registers. Otherwise it finds an
 * 		independent register and saves it on the stack so that it can be used
 * 		as a temporary in the emit2() function.
 ******************************************************************************/

void prologue(Node p, int* x, int* y, int* z, int* yflag, int* destflag) {
	*y = getregnum(LEFT_CHILD(p));
	*z = getregnum(RIGHT_CHILD(p));
	*x = getregnum(p);	
	*yflag = (INTTMP | INTRET) & ~(1<<(*x) | 1<<(*z));  
	if(*y==*z) { 
		for(*y=0; *y<8; *y++)	
			if(*yflag & (1<<*y))	
				break;	
		*yflag=0; 
		lc3_push(*y); 
		lc3_addimm(*y,*z,0);
	} 

	*destflag = (INTTMP | INTRET) & ~(1<<(*y) | 1<<(*z));  
	for(*x=0; *x<8; (*x)++)	
		if(*destflag & (1<<(*x)))	
			break;	
	if(getregnum(p) != *y && getregnum(p) != *z)	
	{	
		*x = getregnum(p);	
		*destflag = 0;	
	} else { 
		lc3_push(*x);
	}
}

/******************************************************************************
 * 	epilogue(p,&x,&y,&yflag,&destflag)
 * 		If the prologue macro found a new independent register to be the
 * 		destination register, then this macro will move the result into the
 * 		original destination register and reload the temporary register with
 * 		its original value.
 *****************************************************************************/
void epilogue(Node p,int* x,int* y,int* yflag,int* destflag) {
	if(*destflag)
	{
		lc3_addimm(getregnum(p),*x,0);
		lc3_pop(*x);
	} 
	if(!(*yflag))
	{
		lc3_pop(*y);
	}
}

char* filename;
/************************************************************
  progend
  last thing called, can emit any back end specific things
 *************************************************************/
static void progend(void){
	printf(".END\n\n");
    free(filename);
}


/************************************************************
  progbeg
  first thing called, should set up reg structure and handle
  any back end specific command line arguments
 *************************************************************/
static void progbeg(int argc, char *argv[]) {
	int i;


    filename = (char*) malloc(strlen(firstfile));
    strcpy(filename, firstfile);
    i = 0;
    while(filename[i]!='.' && filename[i]!='\0')
        i++;
    filename[i] = '\0';

	{
		union {
			char c;
			int i;
		} u;
		u.i = 0;
		u.c = 1;
		swap = ((int)(u.i == 1)) != IR->little_endian;
	}
	/*
	   pic = !IR->little_endian;
	   parseflags(argc, argv);
	   for (i = 0; i < argc; i++)
	   if (strncmp(argv[i], "-G", 2) == 0)
	   gnum = atoi(argv[i] + 2);
	   else if (strcmp(argv[i], "-pic=1") == 0
	   ||       strcmp(argv[i], "-pic=0") == 0)
	   pic = argv[i][5]-'0';
	 */

	for (i = 0; i < 31; i += 2)
		freg2[i] = mkreg("R%d", i, 3, FREG);
	for (i = 0; i < 32; i++)
		ireg[i]  = mkreg("R%d", i, 1, IREG);
	freg2w = mkwildcard(freg2);
	iregw = mkwildcard(ireg);
	tmask[IREG] = INTTMP | INTRET; tmask[FREG] = FLTTMP;
	vmask[IREG] = INTVAR; vmask[FREG] = FLTVAR;

	print(".Orig x3000\n");
	print("INIT_CODE\n");
	print("LEA R6, #-1\n");		//puts x3000 in r6
	lc3_addimm(5,6,0);//r5=x3000
	lc3_add(6,6,6);//r6=x6000
	lc3_add(6,6,6);//r6=xc000
	lc3_add(6,6,5);//r6=xf000
	lc3_addimm(6,6,-1);//r6=xefff

	lc3_add(5,5,5);//r5=x6000
	lc3_addimm(5,6,0);//r5=xefff
	print("LD R4, GLOBAL_DATA_POINTER\n");		//puts x3000 in r6
	print("LD R7, GLOBAL_MAIN_POINTER\n");		//puts x3000 in r6
	print("jsrr R7\n");
	print("HALT\n\n");
	print("GLOBAL_DATA_POINTER .FILL GLOBAL_DATA_START\n");
	print("GLOBAL_MAIN_POINTER .FILL main\n");

}
/************************************************************
  symbol 
  Decides which set of registers to use based on optype
 *************************************************************/
static Symbol rmap(int opk) {
	return iregw;
}
/************************************************************
  target
  Calls setreg and rtarget for certain operations
  --setreg:	Makes dest register equal to passed in argument
  --rtarget:	Lets other instructions know that the passed in
  argument's dest is the 3rd passed in argument
 *************************************************************/
static void target(Node p) {
	assert(p);
	switch (specific(p->op)) {

		case CALL+I: case CALL+P: case CALL+U: case CALL+V:
			setreg(p, ireg[7]);
			break;
		case RET+I: case RET+U: case RET+P:
			rtarget(p, 0, ireg[7]);
			break;
		case ASGN+B: 
			rtarget(p, 0, ireg[1]); 
			rtarget(RIGHT_CHILD(p), 0, ireg[0]); 
			break;
		case ARG+B:  
			rtarget(LEFT_CHILD(p), 0, ireg[0]); 
			break;
	}
}
/************************************************************
  clobber
  Calls spill to let code generator know that certain registers
  will be modified by this operation
Note: CAN'T spill the dest register! Use in conjunction with target
 *************************************************************/
static void clobber(Node p) {
	assert(p);
	switch (specific(p->op)) {
		case GT+I: case GE+I: case LT+I: case LE+I: case EQ+I: case NE+I:
			spill(1<<7, IREG, p); break;

		case JUMP+V:
			spill(1<<0, IREG, p); break;

		case CALL+I: case CALL+P: case CALL+U:
			spill(INTTMP,          IREG, p); break;

		case CALL+V:
			spill(INTTMP /*| INTRET*/, IREG, p); break;
	}
}
/************************************************************
  emit2
  If the first character in the LBURG specification above is '#',
  then this function is called on that node.
  This allows the code generator to generate complicated instructions
 *************************************************************/
static void emit2(Node p) {
	int dst, n, src, sz, ty, i, j, offset;
	static int ty0;
	Symbol q;
	int destflag=0, x=0,y=0,z=0;
	int yflag=0;
	int labels[10];
	int foundflag=0;
	char* str;

	switch (specific(p->op)) {

/***********Handles spilling a register*********************************/
/*does it without telling the back end to allocate another register*/
		case ASGN+U: case ASGN+I:
			if( specific(LEFT_CHILD(p)->op) == VREG+ P)
				break;
			print(";spilling %d\n",atoi (LEFT_CHILD(p)->syms[0]->x.name));
			i=atoi(LEFT_CHILD(p)->syms[0]->x.name);
			lc3_store(4,6,-1);

			lc3_addimm(4,5,-16);

			for(i+=16;i<-16;i+=16)
				lc3_addimm(4,4,-16);

			lc3_store(getregnum(RIGHT_CHILD(p)),4,i);
			lc3_load(4,6,-1);
			break;

/***********Conditional Branches---note: unsigned ones dont always work*****/
		case GT+I: case GE+I: case LT+I: case LE+I: case EQ+I: case NE+I:
		case GT+U: case GE+U: case LT+U: case LE+U: case EQ+U: case NE+U:
			//FIXME: ignoring case where y=z
            //might pose a problem in rare cases
			y = getregnum(LEFT_CHILD(p));	
			z = getregnum(RIGHT_CHILD(p));	
			labels[0] = genlabel(1);
			if(y==7)
			{
				lc3_neg(7);
				lc3_add(7,7,z);
				switch (specific(p->op)) {

					case NE+I: 
					case NE+U: lc3_brz(labels[0]);  break;
					case EQ+I: 
					case EQ+U: lc3_brnp(labels[0]); break;
					case GT+I: 
					case GT+U: lc3_brzp(labels[0]); break;
					case GE+I: 
					case GE+U: lc3_brp(labels[0]);  break;
					case LT+I: 
					case LT+U: lc3_brnz(labels[0]); break;
					case LE+I: 
					case LE+U: lc3_brn(labels[0]);  break;

				}
				
			} else {
				if (z!=7)
					lc3_addimm(7,z,0);

				lc3_neg(7);
				lc3_add(7,7,y);
				switch (specific(p->op)) {
					case NE+I: 
					case NE+U: lc3_brz(labels[0]);  break;
					case EQ+I: 
					case EQ+U: lc3_brnp(labels[0]); break;
					case GT+I: 
					case GT+U: lc3_brnz(labels[0]); break;
					case GE+I: 
					case GE+U: lc3_brn(labels[0]);  break;
					case LT+I: 
					case LT+U: lc3_brzp(labels[0]); break;
					case LE+I: 
					case LE+U: lc3_brp(labels[0]);  break;

				}
			}
			print(".LC3GLOBAL %s 7\n",p->syms[0]->x.name);
			print("LDR R7, R7, #0\njmp R7\n");
			lc3_lab(labels[0]);
			break;

/**************** Load any constant into a 16-bit register *************/
/*not used currently, because all constants are loaded from memory*/
		case CNST+I: case CNST+U: case CNST+P:
			/*Constants need to be loaded by repeated adds*/
			x = getregnum(p);
			lc3_andimm(x,x,0);
			for(i=atoi(p->syms[0]->x.name); i>15; i-=15)
				lc3_addimm(x,x,15);
			for(; i<-16; i+=16)
				lc3_addimm(x,x,-16);
			if(i!=0)
				lc3_addimm(x,x,i);
			break;

/**************** Global, Local, Formal variables addresses ************/
		case ADDRG+P:  
			print(".LC3GLOBAL %s %d\n",p->syms[0]->x.name, getregnum(p));
			break;

		case ADDRL+P:  
			i=atoi(p->syms[0]->x.name);
            if(p->syms[2]==NULL) {
                print("R5, %d",i);
            } else {
                x = getregnum(p);
                if(i<-16)
                {
                    lc3_addimm(x,5,-16);
                    i+=16;
                    for(;i<-16; i+=16)
                        lc3_addimm(x,x,-16);
                    lc3_addimm(x,x,i);
                } else
                    lc3_addimm(x,5,i);
            }
			break;

		case ADDRF+P:
			i=atoi(p->syms[0]->x.name);
			x = getregnum(p);
			if(i>15)
			{
				lc3_addimm(x,5,15);
				i-=15;
				for(;i>15; i-=15)
					lc3_addimm(x,x,15);
				lc3_addimm(x,x,i);
			} else
				lc3_addimm(x,5,i);
			break;

/*************** Logical Operators: |,^,<<,>> ************************/
		case BOR+I:
			/*OR done with demorgan's theorem (a'b')'*/
			prologue(p,&x,&y,&z,&yflag,&destflag);	
			lc3_push(y);
			lc3_push(z);

			lc3_not(y);
			lc3_not(z);
			lc3_and(x,y,z);
			lc3_not(x);

			lc3_pop(z);
			lc3_pop(y);
			
			epilogue(p,&x,&y,&yflag,&destflag);
			break;

		case BXOR+I:
			prologue(p,&x,&y,&z,&yflag,&destflag);	
			lc3_push(y);

			/*XOR done with demorgan's theorem ((ab')'(a'b)')'*/
			lc3_addimm(x,z,0);
			lc3_not(x);
			lc3_and(x,x,y);

			lc3_not(y);
			lc3_and(y,y,z);

			lc3_not(x);
			lc3_not(y);
			lc3_and(x,x,y);
			lc3_not(x);

			lc3_pop(y);
			epilogue(p,&x,&y,&yflag,&destflag);
			break;

		case LSH+I: 
			print(";LSH\n");

			labels[0] = genlabel(1);
			labels[1] = genlabel(1);

			prologue(p,&x,&y,&z,&yflag,&destflag);	

			lc3_addimm(x,y,0);

			lc3_addimm(z,z,0);
			//print("BRz L%d\n",labels[1]);
			lc3_brz(labels[1]);

			//print("L%d\n",labels[0]);
			lc3_lab(labels[0]);
			lc3_add(x,x,x);
			lc3_addimm(z,z,-1);
			//print("BRnp L%d\n",labels[0]);
			lc3_brnp(labels[0]);

			//print("L%d\n",labels[1]);
			lc3_lab(labels[1]);

			epilogue(p,&x,&y,&yflag,&destflag);
			break;

		case RSH+I: 
			print(";RSHI\n");
			prologue(p,&x,&y,&z,&yflag,&destflag);

			lc3_push(z);

			labels[6] = genlabel(1);
			labels[7] = genlabel(1);

			lc3_andimm(x,x,0);
			lc3_addimm(x,x,1);

			lc3_addimm(z,z,0);
			print("BRz L%d\n",labels[6]);

			print("L%d\n",labels[7]);
			lc3_add(x,x,x);
			lc3_addimm(z,z,-1);
			print("BRnp L%d\n",labels[7]);

			print("L%d\n",labels[6]);
			lc3_addimm(z,x,0);

			/* need 6 labels for signed div!*/
			labels[0] = genlabel(1);
			labels[1] = genlabel(1);
			labels[2] = genlabel(1);
			labels[3] = genlabel(1);
			labels[4] = genlabel(1);
			labels[5] = genlabel(1);

			lc3_addimm(z,z,0);
			print("BRz L%d\n",labels[0]);

			lc3_push(y);
			lc3_push(z);
			lc3_andimm(x,x,0);
			lc3_addimm(y,y,0);
			print("BRzp L%d\n",labels[1]);

			lc3_neg(y);
			print("L%d\n",labels[1]);

			lc3_addimm(z,z,0);
			print("BRn L%d\n",labels[2]);

			lc3_neg(z);

			/*div loop finally*/
			print("L%d\n",labels[2]);
			lc3_add(y,y,z);
			print("BRn L%d\n",labels[3]);

			/*x is the quotient*/
			lc3_addimm(x,x,1);
			print("BR L%d\n",labels[2]);

			print("L%d\n",labels[3]);
			lc3_pop(z);
			lc3_pop(y);

			/*figure out if the quotient should be signed or not*/
			lc3_addimm(z,z,0);
			print("BRzp L%d\n",labels[4]);

			lc3_addimm(y,y,0);
			print("BRzp L%d\n",labels[5]);
			print("BRn L%d\n",labels[0]);

			print("L%d\n",labels[4]);
			lc3_addimm(y,y,0);
			print("BRzp L%d\n",labels[0]);

			print("L%d\n",labels[5]);
			lc3_neg(x);

			print("L%d\n",labels[0]);

			lc3_pop(z);
			epilogue(p,&x,&y,&yflag,&destflag);
			break;

/*************** Arithematic Functions  -,*, /,% ***********************/
		case SUB+I: case SUB+P: case SUB+U:
			if(specific(RIGHT_CHILD(p)->op) != CNST)
			{
				prologue(p,&x,&y,&z,&yflag,&destflag);

				lc3_push(z);

				lc3_neg(z);
				lc3_add(x,y,z);
				lc3_pop(z);
				epilogue(p,&x,&y,&yflag,&destflag);
			}
			break;

		case MUL+I:
			//print(";mul R%d, R%d, R%d\n",getregnum(p), getregnum(LEFT_CHILD(p)), getregnum(RIGHT_CHILD(p)));
			prologue(p,&x,&y,&z,&yflag,&destflag);

			lc3_push(z);

			/* need 3 labels for signed mul*/
			labels[0] = genlabel(1);
			labels[1] = genlabel(1);
			labels[2] = genlabel(1);

			lc3_andimm(x,x,0);
			lc3_addimm(z,z,0);

			//print("BRz L%d\n",labels[1]);
			lc3_brz(labels[1]);
			lc3_brp(labels[0]);
			//print("BRp L%d\n",labels[0]);

			lc3_neg(z);

			//print("L%d\n",labels[0]);
			lc3_lab(labels[0]);

			lc3_add(x,x,y);
			lc3_addimm(z,z,-1);

			//print("BRnp L%d\n",labels[0]);
			lc3_brnp(labels[0]);

			//print("L%d\n",labels[1]);
			lc3_lab(labels[1]);
			lc3_pop(z);

			lc3_addimm(z,z,0);
			//print("BRzp L%d\n",labels[2]);
			lc3_brzp(labels[2]);

			lc3_neg(x);

			//print("L%d\n",labels[2]);
			lc3_lab(labels[2]);

			print(";bef epilogue x=%d y=%d z=%d\n",x,y,z);
			epilogue(p,&x,&y,&yflag,&destflag);
			print(";aft epilogue x=%d y=%d z=%d\n",x,y,z);
			break;

		case DIV+I:
			//print(";diving\n");
			prologue(p,&x,&y,&z,&yflag,&destflag);

			/* need 6 labels for signed div!*/
			labels[0] = genlabel(1);
			labels[1] = genlabel(1);
			labels[2] = genlabel(1);
			labels[3] = genlabel(1);
			labels[4] = genlabel(1);
			labels[5] = genlabel(1);

			/* nop when divisor=0 */
			lc3_addimm(z,z,0);
			print("BRz L%d\n",labels[0]);

			lc3_push(y);
			lc3_push(z);
			lc3_andimm(x,x,0);
			lc3_addimm(y,y,0);
			print("BRzp L%d\n",labels[1]);

			lc3_neg(y);
			print("L%d\n",labels[1]);

			lc3_addimm(z,z,0);
			print("BRn L%d\n",labels[2]);

			lc3_neg(z);

			/*div loop finally*/
			print("L%d\n",labels[2]);
			lc3_add(y,y,z);
			print("BRn L%d\n",labels[3]);

			/*x is the quotient*/
			lc3_addimm(x,x,1);
			print("BRnzp L%d\n",labels[2]);

			print("L%d\n",labels[3]);
			lc3_pop(z);
			lc3_pop(y);

			/*figure out if the quotient should be signed or not*/
			lc3_addimm(z,z,0);
			print("BRzp L%d\n",labels[4]);

			lc3_addimm(y,y,0);
			print("BRzp L%d\n",labels[5]);
			print("BRn L%d\n",labels[0]);

			print("L%d\n",labels[4]);
			lc3_addimm(y,y,0);
			print("BRzp L%d\n",labels[0]);

			print("L%d\n",labels[5]);
			lc3_neg(x);

			print("L%d\n",labels[0]);

			epilogue(p,&x,&y,&yflag,&destflag);
			print(";div done\n");
			break;

		case MOD+I:
			//print(";modding R%d, R%d, R%d\n",getregnum(p), getregnum(LEFT_CHILD(p)), getregnum(RIGHT_CHILD(p)));
			prologue(p,&x,&y,&z,&yflag,&destflag);

			/* need 5 labels for signed mod!*/
			labels[0] = genlabel(1);
			labels[1] = genlabel(1);
			labels[2] = genlabel(1);
			labels[3] = genlabel(1);
			labels[4] = genlabel(1);
			labels[5] = genlabel(1);

			/* nop when divisor=0 */
			lc3_addimm(z,z,0);
			print("BRz L%d\n",labels[0]);

			lc3_push(y);
			lc3_push(z);
			lc3_andimm(x,x,0);
			lc3_addimm(y,y,0);
			print("BRzp L%d\n",labels[1]);

			lc3_neg(y);
			print("L%d\n",labels[1]);

			lc3_addimm(z,z,0);
			print("BRn L%d\n",labels[2]);

			lc3_neg(z);

			/*mod loop finally*/
			print("L%d\n",labels[2]);
			lc3_add(y,y,z);
			print("BRzp L%d\n",labels[2]);

			/*y=mod(y,z)-z*/
			lc3_addimm(x,y,0);
			lc3_sub(x,x,z);

			lc3_pop(z);
			lc3_pop(y);

			/*figure out if the mod should be signed or not*/
			lc3_addimm(z,z,0);
			print("BRzp L%d\n",labels[4]);

			lc3_addimm(y,y,0);
			print("BRzp L%d\n",labels[5]);
			print("BRn L%d\n",labels[0]);

			print("L%d\n",labels[4]);
			lc3_addimm(y,y,0);
			print("BRzp L%d\n",labels[0]);

			print("L%d\n",labels[5]);
			lc3_neg(x);

			print("L%d\n",labels[0]);

			epilogue(p,&x,&y,&yflag,&destflag);
			print(";modding done\n");
			break;

/******************Copy a block of memory---like movsb in x*************/
		case ASGN+B:
			print(";ASGNB\n");
			//if(atoi(p->syms[0]->x.name) == 0)
				//break;

			lc3_push(2);
			lc3_push(3);
			tmpregs[0] = 2;
			tmpregs[1] = 3;

			blkloop(1, 0, 0, 0, p->syms[0]->u.c.v.i, tmpregs);
			// blkloop(int dreg, int doff, int sreg, int soff, int size, int tmps[]) 
			lc3_pop(3);
			lc3_pop(2);
			break;

/******************Copy a block of memory to the stack*************/
/*used to pass in struct as argument to a function*/
		case ARG+B:
			print(";ARGB\n");
			//if(atoi(p->syms[0]->x.name) == 0)
			//	break;
			i = p->syms[0]->u.c.v.i;
			//allocate size space on the stack
			for(; i>16; i-=16)
				lc3_addimm(6,6,-16);
			lc3_addimm(6,6,-i);

			lc3_push(1);
			lc3_push(2);
			lc3_push(3);
			tmpregs[0] = 2;
			tmpregs[1] = 3;

			lc3_addimm(1,6,3);
			blkloop(1, 0, 0, 0, p->syms[0]->u.c.v.i, tmpregs);
			lc3_pop(3);
			lc3_pop(2);
			lc3_pop(1);
			break;
	}
}
/************************************************************
  doarg
  Computes the register or stack cell assigned to the next
  argument
  In LC-3's case the next byte on the stack suffices
 *************************************************************/
static void doarg(Node p) {
	assert(p && p->syms[0]);
	mkactual(1, p->syms[0]->u.c.v.i);
}
/************************************************************
  local
  Decides whether to put local variables in a register or
  on the stack.
 *************************************************************/
static void local(Symbol p) {
    if (askregvar(p, rmap(ttob(p->type))) == 0)
        mkauto(p);
}
/************************************************************
  function
  Emits function prologue and epilogue.
  Finds the size of arguments and local variables by calling
  gencode().
 *************************************************************/
static void function(Symbol f, Symbol caller[], Symbol callee[], int ncalls) {
	int i;

	usedmask[0] = usedmask[1] = 0;
	freemask[0] = freemask[1] = ~(unsigned)0;

	offset = 4;	//first location for incoming arguments
	for (i = 0; callee[i]; i++) {
		Symbol p = callee[i];
		Symbol q = caller[i];
		assert(q);
		p->x.offset = q->x.offset = offset;
		p->x.name = q->x.name = stringf("%d", p->x.offset);
		p->sclass = q->sclass = AUTO;
		offset += q->type->size;
	}
	assert(caller[i] == 0);
	offset = maxoffset = maxargoffset = 0;
	offset = -1;
	gencode(caller, callee);

	framesize = maxoffset+1;

	print(";;;;;;;;;;;;;;;;;;;;;;;;;;;;%s;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n", f->x.name);
	if(strcmp("main",f->x.name)==0)
		print("%s\n", f->x.name);
	else {
		print("LC3_GFLAG %s LC3_GFLAG .FILL lc3_%s\n", f->x.name, f->x.name);
		print("lc3_%s\n", f->x.name);
	}

	lc3_addimm(6,6,-2);//allocate space for return val
	lc3_store(7,6,0);//store return addr

	lc3_push(5); //save old base ptr

	lc3_addimm(5,6,-1);//setup base ptr, diff from x86

	printf("\n");

	if(framesize != 0)
	{
		for(i=framesize;i>16;i-=16)
			lc3_addimm(6,6,-16);
		lc3_addimm(6,6,-i);
	}

	emitcode();

	lc3_store(7,5,3); 	//store ret val on stack	
	lc3_addimm(6,5,1); //pop locals off stack

	lc3_pop(5); 	//restoring base ptr
	lc3_pop(7); 	//loading ret addr into r7
	print("RET\n\n");
}
/************************************************************
  defconst
  Emits code to declare a constant or address constant.
 *************************************************************/
static void defconst(int suffix, int size, Value v) {
    if (suffix == P)
        print("LC3_GFLAG .FILL x%x\n", (unsigned)v.p);
    else
        print("LC3_GFLAG .FILL #%d\n", (unsigned)(suffix == I ? v.i : v.u));
}

/************************************************************
  defaddress
  Emits code to declare address.
 *************************************************************/
static void defaddress(Symbol p) {
	print("LC3_GFLAG .FILL %s\n", p->x.name);
}
/************************************************************
  defstring
  Emits code to declare string.
 *************************************************************/
static void defstring(int n, char *str) {
	char *s;

	if(n == 1)
        //treat single character strings as a defconst
		print("LC3_GFLAG .FILL #%d\n", (*str)&0377);
	else {
		print("LC3_GFLAG .STRINGZ %d \"", n);
		for (s = str; s < str + n-1; s++) {
            switch((char) *s) { 
                //set of C escape characters
                //\xhh and \ooo have been excluded
                case '\a':  print("\\a"); break;
                case '\b':  print("\\b"); break;
                case '\f':  print("\\f"); break;
                case '\n':  print("\\n"); break;
                case '\r':  print("\\r"); break;
                case '\t':  print("\\t"); break;
                case '\v':  print("\\v"); break;
                case '\0':  print("\\0"); break;
                case '\\':  print("\\\\"); break;
                default: print("%c", (*s));
            }
		}
		print("\"\n");
	}

}
/************************************************************
  export
  Emits code to export a variable.
  Such a directive is not supported by LC-3
 *************************************************************/
static void export(Symbol p) {
	print(".global %s\n", p->x.name);
}
/************************************************************
  import
  Emits code to import a variable.
  The postprocessor lc3pp replaces .externs with the code for
  that function.
 *************************************************************/
static void import(Symbol p) {
	//if (isfunc(p->type))
		//print(".extern %s %d\n", p->name, p->type->size);
		print(".extern %s\n", p->name);
}
/************************************************************
  defsymbol
  Sets up the symbol name to be 'L'+symbol name in case of labels.
 *************************************************************/
static void defsymbol(Symbol p) {
	if (p->scope >= LOCAL && p->sclass == STATIC)
		p->x.name = stringf("L%d_%s", genlabel(1), filename);
	else if (p->generated)
		p->x.name = stringf("L%s_%s", p->name, filename);
	else
		assert(p->scope != CONSTANTS || isint(p->type) || isptr(p->type)),
	p->x.name = p->name;
}
/************************************************************
  address
  Sets up address names to be address+offset
  LC3 assembler doesn't support this format.
 *************************************************************/
static void address(Symbol q, Symbol p, long n) {
	if (p->scope == GLOBAL || p->sclass == STATIC || p->sclass == EXTERN)
	{
		/*q->x.name = stringf("%s%s%D", p->x.name,
		  n >= 0 ? "+" : "", n/4);*/
		q->x.name = stringf("%s%s%D", p->x.name,
				n >= 0 ? "+" : "", n);
	}
	else {
		assert(n <= INT_MAX && n >= INT_MIN);
		q->x.offset = p->x.offset + n;
		q->x.name = stringd(q->x.offset);
	}
}
/************************************************************
  global
  Emits code for a variable declaration.
  Also inserts variable name into local global variable array.
 *************************************************************/
static void global(Symbol p) {
	print("LC3_GFLAG %s ", p->x.name);
}
/************************************************************
  segment
  Emits code to change segments
  There are no segments in LC-3
 *************************************************************/
static void segment(int n) {
}
/************************************************************
  space
  Emits code to allocate n bytes of space for an uninitialized
  global variable.
 *************************************************************/
static void space(int n) {
	if (cseg != BSS)
		print("LC3_GFLAG .BLKW %d\n", n);
}
/************************************************************
  blkloop
  Emits a loop to copy size bytes in memory.  
  Source address:	register sreg + soff
  Destination address:	register dreg + doff
  tmps[], 3 registers that can be used as temps
  Keep in account alignment issues
 *************************************************************/
static void blkloop(int dreg, int doff, int sreg, int soff, int size, int tmps[]) {
	int lab = genlabel(1);
	int i=0;

	print(";blkloop!!!!\n");

	//make sure both offsets are in [-16,15]
	for(; soff>15; soff-=15)
		lc3_addimm(sreg,sreg,15);
	for(; soff<-16; soff+=16)
		lc3_addimm(sreg,sreg,-16);

	for(; doff>15; doff-=15)
		lc3_addimm(dreg,dreg,15);
	for(; doff<-16; doff+=16)
		lc3_addimm(dreg,dreg,-16);

	//use tmps[1] as size counter (ecx)
	lc3_andimm(tmps[1],tmps[1],0);
	for(i=size; i>15; i-=15)
		lc3_addimm(tmps[1],tmps[1],15);
	lc3_addimm(tmps[1],tmps[1],i);

	//assumes size > 1
	//loop to perform the memory copy
	print("L%d\n", lab);

	lc3_load(tmps[0],sreg,soff);
	lc3_store(tmps[0],dreg,doff);
	lc3_addimm(sreg,sreg,1);
	lc3_addimm(dreg,dreg,1);

	lc3_addimm(tmps[1],tmps[1],-1);
	print("BRnp L%d\n", lab);
}
/************************************************************
  blkfetch
  Emits code to load register tmp with size bytes from address
  in register reg + offset off
 *************************************************************/
static void blkfetch(int size, int off, int reg, int tmp) {
	print("blkfetch!!!!\n");
	//print("ldr R%d, R%d, #%d\n", tmp, reg, off);
	for(; off>15; off-=15)
		lc3_addimm(reg,reg,15);
	for(; off<-16; off+=16)
		lc3_addimm(reg,reg,-16);

	lc3_load(tmp,reg,off);
}
/************************************************************
  blkstore
  Emits code to store in address register reg + offset off
  size bytes from register tmp
 *************************************************************/
static void blkstore(int size, int off, int reg, int tmp) {
	print(";blkstore!!!!!!!\n");
	//print("str R%d, R%d, #%d\n", tmp, reg, off);
	for(; off>15; off-=15)
		lc3_addimm(reg,reg,15);
	for(; off<-16; off+=16)
		lc3_addimm(reg,reg,-16);
	lc3_store(tmp,reg,off);
}

static void stabinit(char *, int, char *[]);
static void stabline(Coordinate *);
static void stabsym(Symbol);

static char *currentfile;


/* stabinit - initialize stab output */
static void stabinit(char *file, int argc, char *argv[]) {
	if (file) {
		print(".file 2,\"%s\"\n", file);
		currentfile = file;
	}
}

/* stabline - emit stab entry for source coordinate *cp */
static void stabline(Coordinate *cp) {
	if (cp->file && cp->file != currentfile) {
		print(".file 2,\"%s\"\n", cp->file);
		currentfile = cp->file;
	}
	print(".loc 2,%d\n", cp->y);
}

/* stabsym - output a stab entry for symbol p */
static void stabsym(Symbol p) {
    if (p == cfunc && IR->stabline)
        (*IR->stabline)(&p->src);
}

Interface lc3IR = {
	1, 1, 1,  /* char */
	1, 1, 1,  /* short */
	1, 1, 1,  /* int */
	1, 1, 1,  /* long */
	1, 1, 1,  /* long long */
	4, 4, 1,  /* float */
	8, 8, 1,  /* double */
	8, 8, 1,  /* long double */
	1, 1, 0,  /* T * */
	0, 1, 0,  /* struct */
	0,      /* big_endian 0=big*/
	0,  /* mulops_calls 1=hardware does not implement mul,div,rem */
	0,  /* 0 = no wants_callb */
	1,  /* 1 = wants_argb */
	0,  /* 0 = no left_to_right */
	0,  /* wants_dag  */
	0,  /* 0 = signed_char */
	address,
	blockbeg,
	blockend,
	defaddress,
	defconst,
	defstring,
	defsymbol,
	emit,
	export,
	function,
	gen,
	global,
	import,
	local,
	progbeg,
	progend,
	segment,
	space,
	0, 0, 0, stabinit, stabline, stabsym, 0,
	{
		4,      /*max_unaligned_load */
		rmap,
		blkfetch, blkstore, blkloop,
		_label,
		_rule,
		_nts,
		_kids,
		_string,
		_templates,
		_isinstruction,
		_ntname,
		emit2,
		doarg,
		target,
		clobber,

	}
};
/*static char rcsid[] = "$Id: lc3.md,v 1.2 2004/03/26 20:06:05 ladsaria Exp $";*/
static char rcsid[] = "$Id: lc3.md,v 1.2 2004/03/26 20:06:05 ladsaria Exp $";
