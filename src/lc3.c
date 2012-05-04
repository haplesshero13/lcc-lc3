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

/*
generated at Thu Apr  1 20:24:56 2004
by $Id: lburg.c,v 1.1.1.1 2004/03/24 04:37:35 sjp Exp $
*/
static void _kids(NODEPTR_TYPE, int, NODEPTR_TYPE[]);
static void _label(NODEPTR_TYPE);
static int _rule(void*, int);

#define _stmt_NT 1
#define _reg_NT 2
#define _c6_NT 3
#define _addr_NT 4
#define _spill_NT 5
#define _c5_NT 6
#define _subc5_NT 7
#define _jaddr_NT 8

static char *_ntname[] = {
	0,
	"stmt",
	"reg",
	"c6",
	"addr",
	"spill",
	"c5",
	"subc5",
	"jaddr",
	0
};

struct _state {
	short cost[9];
	struct {
		unsigned int _stmt:6;
		unsigned int _reg:6;
		unsigned int _c6:2;
		unsigned int _addr:3;
		unsigned int _spill:1;
		unsigned int _c5:2;
		unsigned int _subc5:2;
		unsigned int _jaddr:1;
	} rule;
};

static short _nts_0[] = { 0 };
static short _nts_1[] = { _reg_NT, 0 };
static short _nts_2[] = { _reg_NT, _c6_NT, 0 };
static short _nts_3[] = { _spill_NT, _reg_NT, 0 };
static short _nts_4[] = { _addr_NT, _reg_NT, 0 };
static short _nts_5[] = { _addr_NT, 0 };
static short _nts_6[] = { _reg_NT, _reg_NT, 0 };
static short _nts_7[] = { _reg_NT, _c5_NT, 0 };
static short _nts_8[] = { _reg_NT, _subc5_NT, 0 };
static short _nts_9[] = { _jaddr_NT, 0 };

static short *_nts[] = {
	0,	/* 0 */
	_nts_0,	/* 1 */
	_nts_0,	/* 2 */
	_nts_0,	/* 3 */
	_nts_1,	/* 4 */
	_nts_1,	/* 5 */
	_nts_1,	/* 6 */
	_nts_1,	/* 7 */
	_nts_0,	/* 8 */
	_nts_0,	/* 9 */
	_nts_0,	/* 10 */
	_nts_2,	/* 11 */
	_nts_2,	/* 12 */
	_nts_2,	/* 13 */
	_nts_1,	/* 14 */
	_nts_0,	/* 15 */
	_nts_0,	/* 16 */
	_nts_0,	/* 17 */
	_nts_0,	/* 18 */
	_nts_0,	/* 19 */
	_nts_0,	/* 20 */
	_nts_0,	/* 21 */
	_nts_0,	/* 22 */
	_nts_0,	/* 23 */
	_nts_3,	/* 24 */
	_nts_3,	/* 25 */
	_nts_3,	/* 26 */
	_nts_4,	/* 27 */
	_nts_4,	/* 28 */
	_nts_4,	/* 29 */
	_nts_5,	/* 30 */
	_nts_5,	/* 31 */
	_nts_5,	/* 32 */
	_nts_5,	/* 33 */
	_nts_5,	/* 34 */
	_nts_5,	/* 35 */
	_nts_6,	/* 36 */
	_nts_6,	/* 37 */
	_nts_6,	/* 38 */
	_nts_6,	/* 39 */
	_nts_6,	/* 40 */
	_nts_6,	/* 41 */
	_nts_0,	/* 42 */
	_nts_0,	/* 43 */
	_nts_0,	/* 44 */
	_nts_0,	/* 45 */
	_nts_0,	/* 46 */
	_nts_0,	/* 47 */
	_nts_7,	/* 48 */
	_nts_7,	/* 49 */
	_nts_7,	/* 50 */
	_nts_7,	/* 51 */
	_nts_7,	/* 52 */
	_nts_6,	/* 53 */
	_nts_6,	/* 54 */
	_nts_6,	/* 55 */
	_nts_6,	/* 56 */
	_nts_6,	/* 57 */
	_nts_6,	/* 58 */
	_nts_6,	/* 59 */
	_nts_6,	/* 60 */
	_nts_6,	/* 61 */
	_nts_8,	/* 62 */
	_nts_8,	/* 63 */
	_nts_8,	/* 64 */
	_nts_6,	/* 65 */
	_nts_6,	/* 66 */
	_nts_6,	/* 67 */
	_nts_6,	/* 68 */
	_nts_6,	/* 69 */
	_nts_6,	/* 70 */
	_nts_6,	/* 71 */
	_nts_1,	/* 72 */
	_nts_1,	/* 73 */
	_nts_1,	/* 74 */
	_nts_1,	/* 75 */
	_nts_1,	/* 76 */
	_nts_1,	/* 77 */
	_nts_1,	/* 78 */
	_nts_1,	/* 79 */
	_nts_1,	/* 80 */
	_nts_1,	/* 81 */
	_nts_1,	/* 82 */
	_nts_1,	/* 83 */
	_nts_0,	/* 84 */
	_nts_0,	/* 85 */
	_nts_9,	/* 86 */
	_nts_1,	/* 87 */
	_nts_6,	/* 88 */
	_nts_6,	/* 89 */
	_nts_6,	/* 90 */
	_nts_6,	/* 91 */
	_nts_6,	/* 92 */
	_nts_6,	/* 93 */
	_nts_6,	/* 94 */
	_nts_6,	/* 95 */
	_nts_6,	/* 96 */
	_nts_6,	/* 97 */
	_nts_6,	/* 98 */
	_nts_6,	/* 99 */
	_nts_9,	/* 100 */
	_nts_9,	/* 101 */
	_nts_9,	/* 102 */
	_nts_9,	/* 103 */
	_nts_1,	/* 104 */
	_nts_1,	/* 105 */
	_nts_1,	/* 106 */
	_nts_1,	/* 107 */
	_nts_1,	/* 108 */
	_nts_1,	/* 109 */
	_nts_1,	/* 110 */
	_nts_1,	/* 111 */
	_nts_1,	/* 112 */
	_nts_1,	/* 113 */
	_nts_1,	/* 114 */
	_nts_1,	/* 115 */
	_nts_6,	/* 116 */
};

static char *_templates[] = {
/* 0 */	0,
/* 1 */	"# read register\n",	/* reg: INDIRI1(VREGP) */
/* 2 */	"# read register\n",	/* reg: INDIRU1(VREGP) */
/* 3 */	"# read register\n",	/* reg: INDIRP1(VREGP) */
/* 4 */	"# write register\n",	/* stmt: ASGNI1(VREGP,reg) */
/* 5 */	"# write register\n",	/* stmt: ASGNU1(VREGP,reg) */
/* 6 */	"# write register\n",	/* stmt: ASGNP1(VREGP,reg) */
/* 7 */	"",	/* stmt: reg */
/* 8 */	"%a",	/* c6: CNSTP1 */
/* 9 */	"%a",	/* c6: CNSTI1 */
/* 10 */	"%a",	/* c6: CNSTU1 */
/* 11 */	"%0, #%1",	/* addr: ADDI1(reg,c6) */
/* 12 */	"%0, #%1",	/* addr: ADDU1(reg,c6) */
/* 13 */	"%0, #%1",	/* addr: ADDP1(reg,c6) */
/* 14 */	"%0, #0",	/* addr: reg */
/* 15 */	"R5, #%a",	/* addr: ADDRFP1 */
/* 16 */	"R5, #%a",	/* addr: ADDRLP1 */
/* 17 */	"# formal stack\n",	/* reg: ADDRFP1 */
/* 18 */	"# local stack\n",	/* reg: ADDRLP1 */
/* 19 */	"# global var\n",	/* reg: ADDRGP1 */
/* 20 */	"# reg\n",	/* reg: CNSTI1 */
/* 21 */	"# reg\n",	/* reg: CNSTU1 */
/* 22 */	"# reg\n",	/* reg: CNSTP1 */
/* 23 */	"%a",	/* spill: ADDRLP1 */
/* 24 */	"#str %1, %0\n",	/* stmt: ASGNI1(spill,reg) */
/* 25 */	"#str %1, %0\n",	/* stmt: ASGNU1(spill,reg) */
/* 26 */	"#str %1, %0\n",	/* stmt: ASGNP1(spill,reg) */
/* 27 */	"str %1, %0\n",	/* stmt: ASGNI1(addr,reg) */
/* 28 */	"str %1, %0\n",	/* stmt: ASGNU1(addr,reg) */
/* 29 */	"str %1, %0\n",	/* stmt: ASGNP1(addr,reg) */
/* 30 */	"ldr %c, %0\n",	/* reg: INDIRI1(addr) */
/* 31 */	"ldr %c, %0\n",	/* reg: INDIRU1(addr) */
/* 32 */	"ldr %c, %0\n",	/* reg: INDIRP1(addr) */
/* 33 */	"ldr %c, %0\n",	/* reg: CVPU1(INDIRI1(addr)) */
/* 34 */	"ldr %c, %0\n",	/* reg: CVIU1(INDIRU1(addr)) */
/* 35 */	"ldr %c, %0\n",	/* reg: CVUI1(INDIRU1(addr)) */
/* 36 */	"#\n",	/* reg: DIVI1(reg,reg) */
/* 37 */	"#\n",	/* reg: DIVU1(reg,reg) */
/* 38 */	"#\n",	/* reg: MODI1(reg,reg) */
/* 39 */	"#\n",	/* reg: MODU1(reg,reg) */
/* 40 */	"#\n",	/* reg: MULI1(reg,reg) */
/* 41 */	"#\n",	/* reg: MULU1(reg,reg) */
/* 42 */	"%a",	/* c5: CNSTP1 */
/* 43 */	"%a",	/* c5: CNSTI1 */
/* 44 */	"%a",	/* c5: CNSTU1 */
/* 45 */	"%a",	/* subc5: CNSTP1 */
/* 46 */	"%a",	/* subc5: CNSTI1 */
/* 47 */	"%a",	/* subc5: CNSTU1 */
/* 48 */	"add %c, %0, #%1\n",	/* reg: ADDI1(reg,c5) */
/* 49 */	"add %c, %0, #%1\n",	/* reg: ADDP1(reg,c5) */
/* 50 */	"add %c, %0, #%1\n",	/* reg: ADDU1(reg,c5) */
/* 51 */	"and %c, %0, #%1\n",	/* reg: BANDI1(reg,c5) */
/* 52 */	"and %c, %0, #%1\n",	/* reg: BANDU1(reg,c5) */
/* 53 */	"add %c, %0, %1\n",	/* reg: ADDI1(reg,reg) */
/* 54 */	"add %c, %0, %1\n",	/* reg: ADDP1(reg,reg) */
/* 55 */	"add %c, %0, %1\n",	/* reg: ADDU1(reg,reg) */
/* 56 */	"and %c, %0, %1\n",	/* reg: BANDI1(reg,reg) */
/* 57 */	"and %c, %0, %1\n",	/* reg: BANDU1(reg,reg) */
/* 58 */	"# \n",	/* reg: BORI1(reg,reg) */
/* 59 */	"# \n",	/* reg: BORU1(reg,reg) */
/* 60 */	"# \n",	/* reg: BXORI1(reg,reg) */
/* 61 */	"# \n",	/* reg: BXORU1(reg,reg) */
/* 62 */	"add %c, %0, #-%1\n",	/* reg: SUBI1(reg,subc5) */
/* 63 */	"add %c, %0, #-%1\n",	/* reg: SUBP1(reg,subc5) */
/* 64 */	"add %c, %0, #-%1\n",	/* reg: SUBU1(reg,subc5) */
/* 65 */	"#\n",	/* reg: SUBI1(reg,reg) */
/* 66 */	"#\n",	/* reg: SUBP1(reg,reg) */
/* 67 */	"#\n",	/* reg: SUBU1(reg,reg) */
/* 68 */	"#\n",	/* reg: LSHI1(reg,reg) */
/* 69 */	"#\n",	/* reg: LSHU1(reg,reg) */
/* 70 */	"#\n",	/* reg: RSHI1(reg,reg) */
/* 71 */	"#\n",	/* reg: RSHU1(reg,reg) */
/* 72 */	"not %c,%0\n",	/* reg: BCOMI1(reg) */
/* 73 */	"not %c,%0\n",	/* reg: BCOMU1(reg) */
/* 74 */	"not %c,%0\nadd %c,%c,#1\n",	/* reg: NEGI1(reg) */
/* 75 */	"add %c, %0, #0\n",	/* reg: LOADI1(reg) */
/* 76 */	"add %c, %0, #0\n",	/* reg: LOADU1(reg) */
/* 77 */	"add %c, %0, #0\n",	/* reg: LOADP1(reg) */
/* 78 */	"add %c, %0, #0\n",	/* reg: CVPU1(reg) */
/* 79 */	"add %c, %0, #0\n",	/* reg: CVUP1(reg) */
/* 80 */	"add %c, %0, #0\n",	/* reg: CVII1(reg) */
/* 81 */	"add %c, %0, #0\n",	/* reg: CVIU1(reg) */
/* 82 */	"add %c, %0, #0\n",	/* reg: CVUI1(reg) */
/* 83 */	"add %c, %0, #0\n",	/* reg: CVUU1(reg) */
/* 84 */	"LC3_GFLAG %a LC3_GFLAG .FILL lc3_%a\nlc3_%a\n",	/* stmt: LABELV */
/* 85 */	"%a",	/* jaddr: ADDRGP1 */
/* 86 */	".LC3GLOBAL %0 0\nLDR R0, R0, #0\nJMP R0\n",	/* stmt: JUMPV(jaddr) */
/* 87 */	"LDR %0, %0, #0\nJMP %0 ;unconditional\n",	/* stmt: JUMPV(reg) */
/* 88 */	"#conditional branch\n",	/* stmt: EQI1(reg,reg) */
/* 89 */	"#conditional branch\n",	/* stmt: EQU1(reg,reg) */
/* 90 */	"#conditional branch\n",	/* stmt: GEI1(reg,reg) */
/* 91 */	"#conditional branch\n",	/* stmt: GEU1(reg,reg) */
/* 92 */	"#conditional branch\n",	/* stmt: GTI1(reg,reg) */
/* 93 */	"#conditional branch\n",	/* stmt: GTU1(reg,reg) */
/* 94 */	"#conditional branch\n",	/* stmt: LEI1(reg,reg) */
/* 95 */	"#conditional branch\n",	/* stmt: LEU1(reg,reg) */
/* 96 */	"#conditional branch\n",	/* stmt: LTI1(reg,reg) */
/* 97 */	"#conditional branch\n",	/* stmt: LTU1(reg,reg) */
/* 98 */	"#conditional branch\n",	/* stmt: NEI1(reg,reg) */
/* 99 */	"#conditional branch\n",	/* stmt: NEU1(reg,reg) */
/* 100 */	".LC3GLOBAL %0 0\nLDR R0, R0, #0\njsrr R0\nLDR R7, R6, #0\nADD R6, R6, #1\n",	/* reg: CALLI1(jaddr) */
/* 101 */	".LC3GLOBAL %0 0\nLDR R0, R0, #0\njsrr R0\nLDR R7, R6, #0\nADD R6, R6, #1\n",	/* reg: CALLP1(jaddr) */
/* 102 */	".LC3GLOBAL %0 0\nLDR R0, R0, #0\njsrr R0\nLDR R7, R6, #0\nADD R6, R6, #1\n",	/* reg: CALLU1(jaddr) */
/* 103 */	".LC3GLOBAL %0 0\nLDR R0, R0, #0\njsrr R0\n",	/* stmt: CALLV(jaddr) */
/* 104 */	"jsrr %0\nLDR R7, R6, #0\nADD R6, R6, #1\n",	/* reg: CALLI1(reg) */
/* 105 */	"jsrr %0\nLDR R7, R6, #0\nADD R6, R6, #1\n",	/* reg: CALLP1(reg) */
/* 106 */	"jsrr %0\nLDR R7, R6, #0\nADD R6, R6, #1\n",	/* reg: CALLU1(reg) */
/* 107 */	"jsrr %0\n",	/* stmt: CALLV(reg) */
/* 108 */	"# ret\n",	/* stmt: RETI1(reg) */
/* 109 */	"# ret\n",	/* stmt: RETU1(reg) */
/* 110 */	"# ret\n",	/* stmt: RETP1(reg) */
/* 111 */	"# ret\n",	/* stmt: RETV(reg) */
/* 112 */	"ADD R6, R6, #-1\nSTR %0, R6, #0\n",	/* stmt: ARGI1(reg) */
/* 113 */	"ADD R6, R6, #-1\nSTR %0, R6, #0\n",	/* stmt: ARGP1(reg) */
/* 114 */	"ADD R6, R6, #-1\nSTR %0, R6, #0\n",	/* stmt: ARGU1(reg) */
/* 115 */	"#argb \n",	/* stmt: ARGB(INDIRB(reg)) */
/* 116 */	"#asgnb\n",	/* stmt: ASGNB(reg,INDIRB(reg)) */
};

static char _isinstruction[] = {
/* 0 */	0,
/* 1 */	1,	/* # read register\n */
/* 2 */	1,	/* # read register\n */
/* 3 */	1,	/* # read register\n */
/* 4 */	1,	/* # write register\n */
/* 5 */	1,	/* # write register\n */
/* 6 */	1,	/* # write register\n */
/* 7 */	0,	/*  */
/* 8 */	0,	/* %a */
/* 9 */	0,	/* %a */
/* 10 */	0,	/* %a */
/* 11 */	0,	/* %0, #%1 */
/* 12 */	0,	/* %0, #%1 */
/* 13 */	0,	/* %0, #%1 */
/* 14 */	0,	/* %0, #0 */
/* 15 */	0,	/* R5, #%a */
/* 16 */	0,	/* R5, #%a */
/* 17 */	1,	/* # formal stack\n */
/* 18 */	1,	/* # local stack\n */
/* 19 */	1,	/* # global var\n */
/* 20 */	1,	/* # reg\n */
/* 21 */	1,	/* # reg\n */
/* 22 */	1,	/* # reg\n */
/* 23 */	0,	/* %a */
/* 24 */	1,	/* #str %1, %0\n */
/* 25 */	1,	/* #str %1, %0\n */
/* 26 */	1,	/* #str %1, %0\n */
/* 27 */	1,	/* str %1, %0\n */
/* 28 */	1,	/* str %1, %0\n */
/* 29 */	1,	/* str %1, %0\n */
/* 30 */	1,	/* ldr %c, %0\n */
/* 31 */	1,	/* ldr %c, %0\n */
/* 32 */	1,	/* ldr %c, %0\n */
/* 33 */	1,	/* ldr %c, %0\n */
/* 34 */	1,	/* ldr %c, %0\n */
/* 35 */	1,	/* ldr %c, %0\n */
/* 36 */	1,	/* #\n */
/* 37 */	1,	/* #\n */
/* 38 */	1,	/* #\n */
/* 39 */	1,	/* #\n */
/* 40 */	1,	/* #\n */
/* 41 */	1,	/* #\n */
/* 42 */	0,	/* %a */
/* 43 */	0,	/* %a */
/* 44 */	0,	/* %a */
/* 45 */	0,	/* %a */
/* 46 */	0,	/* %a */
/* 47 */	0,	/* %a */
/* 48 */	1,	/* add %c, %0, #%1\n */
/* 49 */	1,	/* add %c, %0, #%1\n */
/* 50 */	1,	/* add %c, %0, #%1\n */
/* 51 */	1,	/* and %c, %0, #%1\n */
/* 52 */	1,	/* and %c, %0, #%1\n */
/* 53 */	1,	/* add %c, %0, %1\n */
/* 54 */	1,	/* add %c, %0, %1\n */
/* 55 */	1,	/* add %c, %0, %1\n */
/* 56 */	1,	/* and %c, %0, %1\n */
/* 57 */	1,	/* and %c, %0, %1\n */
/* 58 */	1,	/* # \n */
/* 59 */	1,	/* # \n */
/* 60 */	1,	/* # \n */
/* 61 */	1,	/* # \n */
/* 62 */	1,	/* add %c, %0, #-%1\n */
/* 63 */	1,	/* add %c, %0, #-%1\n */
/* 64 */	1,	/* add %c, %0, #-%1\n */
/* 65 */	1,	/* #\n */
/* 66 */	1,	/* #\n */
/* 67 */	1,	/* #\n */
/* 68 */	1,	/* #\n */
/* 69 */	1,	/* #\n */
/* 70 */	1,	/* #\n */
/* 71 */	1,	/* #\n */
/* 72 */	1,	/* not %c,%0\n */
/* 73 */	1,	/* not %c,%0\n */
/* 74 */	1,	/* not %c,%0\nadd %c,%c,#1\n */
/* 75 */	1,	/* add %c, %0, #0\n */
/* 76 */	1,	/* add %c, %0, #0\n */
/* 77 */	1,	/* add %c, %0, #0\n */
/* 78 */	1,	/* add %c, %0, #0\n */
/* 79 */	1,	/* add %c, %0, #0\n */
/* 80 */	1,	/* add %c, %0, #0\n */
/* 81 */	1,	/* add %c, %0, #0\n */
/* 82 */	1,	/* add %c, %0, #0\n */
/* 83 */	1,	/* add %c, %0, #0\n */
/* 84 */	1,	/* LC3_GFLAG %a LC3_GFLAG .FILL lc3_%a\nlc3_%a\n */
/* 85 */	0,	/* %a */
/* 86 */	1,	/* .LC3GLOBAL %0 0\nLDR R0, R0, #0\nJMP R0\n */
/* 87 */	1,	/* LDR %0, %0, #0\nJMP %0 ;unconditional\n */
/* 88 */	1,	/* #conditional branch\n */
/* 89 */	1,	/* #conditional branch\n */
/* 90 */	1,	/* #conditional branch\n */
/* 91 */	1,	/* #conditional branch\n */
/* 92 */	1,	/* #conditional branch\n */
/* 93 */	1,	/* #conditional branch\n */
/* 94 */	1,	/* #conditional branch\n */
/* 95 */	1,	/* #conditional branch\n */
/* 96 */	1,	/* #conditional branch\n */
/* 97 */	1,	/* #conditional branch\n */
/* 98 */	1,	/* #conditional branch\n */
/* 99 */	1,	/* #conditional branch\n */
/* 100 */	1,	/* .LC3GLOBAL %0 0\nLDR R0, R0, #0\njsrr R0\nLDR R7, R6, #0\nADD R6, R6, #1\n */
/* 101 */	1,	/* .LC3GLOBAL %0 0\nLDR R0, R0, #0\njsrr R0\nLDR R7, R6, #0\nADD R6, R6, #1\n */
/* 102 */	1,	/* .LC3GLOBAL %0 0\nLDR R0, R0, #0\njsrr R0\nLDR R7, R6, #0\nADD R6, R6, #1\n */
/* 103 */	1,	/* .LC3GLOBAL %0 0\nLDR R0, R0, #0\njsrr R0\n */
/* 104 */	1,	/* jsrr %0\nLDR R7, R6, #0\nADD R6, R6, #1\n */
/* 105 */	1,	/* jsrr %0\nLDR R7, R6, #0\nADD R6, R6, #1\n */
/* 106 */	1,	/* jsrr %0\nLDR R7, R6, #0\nADD R6, R6, #1\n */
/* 107 */	1,	/* jsrr %0\n */
/* 108 */	1,	/* # ret\n */
/* 109 */	1,	/* # ret\n */
/* 110 */	1,	/* # ret\n */
/* 111 */	1,	/* # ret\n */
/* 112 */	1,	/* ADD R6, R6, #-1\nSTR %0, R6, #0\n */
/* 113 */	1,	/* ADD R6, R6, #-1\nSTR %0, R6, #0\n */
/* 114 */	1,	/* ADD R6, R6, #-1\nSTR %0, R6, #0\n */
/* 115 */	1,	/* #argb \n */
/* 116 */	1,	/* #asgnb\n */
};

static char *_string[] = {
/* 0 */	0,
/* 1 */	"reg: INDIRI1(VREGP)",
/* 2 */	"reg: INDIRU1(VREGP)",
/* 3 */	"reg: INDIRP1(VREGP)",
/* 4 */	"stmt: ASGNI1(VREGP,reg)",
/* 5 */	"stmt: ASGNU1(VREGP,reg)",
/* 6 */	"stmt: ASGNP1(VREGP,reg)",
/* 7 */	"stmt: reg",
/* 8 */	"c6: CNSTP1",
/* 9 */	"c6: CNSTI1",
/* 10 */	"c6: CNSTU1",
/* 11 */	"addr: ADDI1(reg,c6)",
/* 12 */	"addr: ADDU1(reg,c6)",
/* 13 */	"addr: ADDP1(reg,c6)",
/* 14 */	"addr: reg",
/* 15 */	"addr: ADDRFP1",
/* 16 */	"addr: ADDRLP1",
/* 17 */	"reg: ADDRFP1",
/* 18 */	"reg: ADDRLP1",
/* 19 */	"reg: ADDRGP1",
/* 20 */	"reg: CNSTI1",
/* 21 */	"reg: CNSTU1",
/* 22 */	"reg: CNSTP1",
/* 23 */	"spill: ADDRLP1",
/* 24 */	"stmt: ASGNI1(spill,reg)",
/* 25 */	"stmt: ASGNU1(spill,reg)",
/* 26 */	"stmt: ASGNP1(spill,reg)",
/* 27 */	"stmt: ASGNI1(addr,reg)",
/* 28 */	"stmt: ASGNU1(addr,reg)",
/* 29 */	"stmt: ASGNP1(addr,reg)",
/* 30 */	"reg: INDIRI1(addr)",
/* 31 */	"reg: INDIRU1(addr)",
/* 32 */	"reg: INDIRP1(addr)",
/* 33 */	"reg: CVPU1(INDIRI1(addr))",
/* 34 */	"reg: CVIU1(INDIRU1(addr))",
/* 35 */	"reg: CVUI1(INDIRU1(addr))",
/* 36 */	"reg: DIVI1(reg,reg)",
/* 37 */	"reg: DIVU1(reg,reg)",
/* 38 */	"reg: MODI1(reg,reg)",
/* 39 */	"reg: MODU1(reg,reg)",
/* 40 */	"reg: MULI1(reg,reg)",
/* 41 */	"reg: MULU1(reg,reg)",
/* 42 */	"c5: CNSTP1",
/* 43 */	"c5: CNSTI1",
/* 44 */	"c5: CNSTU1",
/* 45 */	"subc5: CNSTP1",
/* 46 */	"subc5: CNSTI1",
/* 47 */	"subc5: CNSTU1",
/* 48 */	"reg: ADDI1(reg,c5)",
/* 49 */	"reg: ADDP1(reg,c5)",
/* 50 */	"reg: ADDU1(reg,c5)",
/* 51 */	"reg: BANDI1(reg,c5)",
/* 52 */	"reg: BANDU1(reg,c5)",
/* 53 */	"reg: ADDI1(reg,reg)",
/* 54 */	"reg: ADDP1(reg,reg)",
/* 55 */	"reg: ADDU1(reg,reg)",
/* 56 */	"reg: BANDI1(reg,reg)",
/* 57 */	"reg: BANDU1(reg,reg)",
/* 58 */	"reg: BORI1(reg,reg)",
/* 59 */	"reg: BORU1(reg,reg)",
/* 60 */	"reg: BXORI1(reg,reg)",
/* 61 */	"reg: BXORU1(reg,reg)",
/* 62 */	"reg: SUBI1(reg,subc5)",
/* 63 */	"reg: SUBP1(reg,subc5)",
/* 64 */	"reg: SUBU1(reg,subc5)",
/* 65 */	"reg: SUBI1(reg,reg)",
/* 66 */	"reg: SUBP1(reg,reg)",
/* 67 */	"reg: SUBU1(reg,reg)",
/* 68 */	"reg: LSHI1(reg,reg)",
/* 69 */	"reg: LSHU1(reg,reg)",
/* 70 */	"reg: RSHI1(reg,reg)",
/* 71 */	"reg: RSHU1(reg,reg)",
/* 72 */	"reg: BCOMI1(reg)",
/* 73 */	"reg: BCOMU1(reg)",
/* 74 */	"reg: NEGI1(reg)",
/* 75 */	"reg: LOADI1(reg)",
/* 76 */	"reg: LOADU1(reg)",
/* 77 */	"reg: LOADP1(reg)",
/* 78 */	"reg: CVPU1(reg)",
/* 79 */	"reg: CVUP1(reg)",
/* 80 */	"reg: CVII1(reg)",
/* 81 */	"reg: CVIU1(reg)",
/* 82 */	"reg: CVUI1(reg)",
/* 83 */	"reg: CVUU1(reg)",
/* 84 */	"stmt: LABELV",
/* 85 */	"jaddr: ADDRGP1",
/* 86 */	"stmt: JUMPV(jaddr)",
/* 87 */	"stmt: JUMPV(reg)",
/* 88 */	"stmt: EQI1(reg,reg)",
/* 89 */	"stmt: EQU1(reg,reg)",
/* 90 */	"stmt: GEI1(reg,reg)",
/* 91 */	"stmt: GEU1(reg,reg)",
/* 92 */	"stmt: GTI1(reg,reg)",
/* 93 */	"stmt: GTU1(reg,reg)",
/* 94 */	"stmt: LEI1(reg,reg)",
/* 95 */	"stmt: LEU1(reg,reg)",
/* 96 */	"stmt: LTI1(reg,reg)",
/* 97 */	"stmt: LTU1(reg,reg)",
/* 98 */	"stmt: NEI1(reg,reg)",
/* 99 */	"stmt: NEU1(reg,reg)",
/* 100 */	"reg: CALLI1(jaddr)",
/* 101 */	"reg: CALLP1(jaddr)",
/* 102 */	"reg: CALLU1(jaddr)",
/* 103 */	"stmt: CALLV(jaddr)",
/* 104 */	"reg: CALLI1(reg)",
/* 105 */	"reg: CALLP1(reg)",
/* 106 */	"reg: CALLU1(reg)",
/* 107 */	"stmt: CALLV(reg)",
/* 108 */	"stmt: RETI1(reg)",
/* 109 */	"stmt: RETU1(reg)",
/* 110 */	"stmt: RETP1(reg)",
/* 111 */	"stmt: RETV(reg)",
/* 112 */	"stmt: ARGI1(reg)",
/* 113 */	"stmt: ARGP1(reg)",
/* 114 */	"stmt: ARGU1(reg)",
/* 115 */	"stmt: ARGB(INDIRB(reg))",
/* 116 */	"stmt: ASGNB(reg,INDIRB(reg))",
};

static short _decode_stmt[] = {
	0,
	4,
	5,
	6,
	7,
	24,
	25,
	26,
	27,
	28,
	29,
	84,
	86,
	87,
	88,
	89,
	90,
	91,
	92,
	93,
	94,
	95,
	96,
	97,
	98,
	99,
	103,
	107,
	108,
	109,
	110,
	111,
	112,
	113,
	114,
	115,
	116,
};

static short _decode_reg[] = {
	0,
	1,
	2,
	3,
	17,
	18,
	19,
	20,
	21,
	22,
	30,
	31,
	32,
	33,
	34,
	35,
	36,
	37,
	38,
	39,
	40,
	41,
	48,
	49,
	50,
	51,
	52,
	53,
	54,
	55,
	56,
	57,
	58,
	59,
	60,
	61,
	62,
	63,
	64,
	65,
	66,
	67,
	68,
	69,
	70,
	71,
	72,
	73,
	74,
	75,
	76,
	77,
	78,
	79,
	80,
	81,
	82,
	83,
	100,
	101,
	102,
	104,
	105,
	106,
};

static short _decode_c6[] = {
	0,
	8,
	9,
	10,
};

static short _decode_addr[] = {
	0,
	11,
	12,
	13,
	14,
	15,
	16,
};

static short _decode_spill[] = {
	0,
	23,
};

static short _decode_c5[] = {
	0,
	42,
	43,
	44,
};

static short _decode_subc5[] = {
	0,
	45,
	46,
	47,
};

static short _decode_jaddr[] = {
	0,
	85,
};

static int _rule(void *state, int goalnt) {
	if (goalnt < 1 || goalnt > 8)
		fatal("_rule", "Bad goal nonterminal %d\n", goalnt);
	if (!state)
		return 0;
	switch (goalnt) {
	case _stmt_NT:	return _decode_stmt[((struct _state *)state)->rule._stmt];
	case _reg_NT:	return _decode_reg[((struct _state *)state)->rule._reg];
	case _c6_NT:	return _decode_c6[((struct _state *)state)->rule._c6];
	case _addr_NT:	return _decode_addr[((struct _state *)state)->rule._addr];
	case _spill_NT:	return _decode_spill[((struct _state *)state)->rule._spill];
	case _c5_NT:	return _decode_c5[((struct _state *)state)->rule._c5];
	case _subc5_NT:	return _decode_subc5[((struct _state *)state)->rule._subc5];
	case _jaddr_NT:	return _decode_jaddr[((struct _state *)state)->rule._jaddr];
	default:
		fatal("_rule", "Bad goal nonterminal %d\n", goalnt);
		return 0;
	}
}

static void _closure_reg(NODEPTR_TYPE, int);

static void _closure_reg(NODEPTR_TYPE a, int c) {
	struct _state *p = STATE_LABEL(a);
	if (c + 0 < p->cost[_addr_NT]) {
		p->cost[_addr_NT] = c + 0;
		p->rule._addr = 4;
	}
	if (c + 0 < p->cost[_stmt_NT]) {
		p->cost[_stmt_NT] = c + 0;
		p->rule._stmt = 4;
	}
}

static void _label(NODEPTR_TYPE a) {
	int c;
	struct _state *p;

	if (!a)
		fatal("_label", "Null tree\n", 0);
	STATE_LABEL(a) = p = allocate(sizeof *p, FUNC);
	p->rule._stmt = 0;
	p->cost[1] =
	p->cost[2] =
	p->cost[3] =
	p->cost[4] =
	p->cost[5] =
	p->cost[6] =
	p->cost[7] =
	p->cost[8] =
		0x7fff;
	switch (OP_LABEL(a)) {
	case 41: /* ARGB */
		_label(LEFT_CHILD(a));
		if (	/* stmt: ARGB(INDIRB(reg)) */
			LEFT_CHILD(a)->op == 73 /* INDIRB */
		) {
			c = ((struct _state *)(LEFT_CHILD(LEFT_CHILD(a))->x.state))->cost[_reg_NT] + 10;
			if (c + 0 < p->cost[_stmt_NT]) {
				p->cost[_stmt_NT] = c + 0;
				p->rule._stmt = 35;
			}
		}
		break;
	case 57: /* ASGNB */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		if (	/* stmt: ASGNB(reg,INDIRB(reg)) */
			RIGHT_CHILD(a)->op == 73 /* INDIRB */
		) {
			c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(LEFT_CHILD(RIGHT_CHILD(a))->x.state))->cost[_reg_NT] + 10;
			if (c + 0 < p->cost[_stmt_NT]) {
				p->cost[_stmt_NT] = c + 0;
				p->rule._stmt = 36;
			}
		}
		break;
	case 73: /* INDIRB */
		_label(LEFT_CHILD(a));
		break;
	case 216: /* CALLV */
		_label(LEFT_CHILD(a));
		/* stmt: CALLV(jaddr) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_jaddr_NT] + 1;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 26;
		}
		/* stmt: CALLV(reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + 1;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 27;
		}
		break;
	case 217: /* CALLB */
		break;
	case 233: /* LOADB */
		break;
	case 248: /* RETV */
		_label(LEFT_CHILD(a));
		/* stmt: RETV(reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + 1;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 31;
		}
		break;
	case 584: /* JUMPV */
		_label(LEFT_CHILD(a));
		/* stmt: JUMPV(jaddr) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_jaddr_NT] + 0;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 12;
		}
		/* stmt: JUMPV(reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + 0;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 13;
		}
		break;
	case 600: /* LABELV */
		/* stmt: LABELV */
		if (0 + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = 0 + 0;
			p->rule._stmt = 11;
		}
		break;
	case 711: /* VREGP */
		break;
	case 1045: /* CNSTI1 */
		/* c6: CNSTI1 */
		c = (range(a,-32,31));
		if (c + 0 < p->cost[_c6_NT]) {
			p->cost[_c6_NT] = c + 0;
			p->rule._c6 = 2;
		}
		/* reg: CNSTI1 */
		if (2 + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = 2 + 0;
			p->rule._reg = 7;
			_closure_reg(a, 2 + 0);
		}
		/* c5: CNSTI1 */
		c = (range(a,-16,15));
		if (c + 0 < p->cost[_c5_NT]) {
			p->cost[_c5_NT] = c + 0;
			p->rule._c5 = 2;
		}
		/* subc5: CNSTI1 */
		c = (range(a,-15,16));
		if (c + 0 < p->cost[_subc5_NT]) {
			p->cost[_subc5_NT] = c + 0;
			p->rule._subc5 = 2;
		}
		break;
	case 1046: /* CNSTU1 */
		/* c6: CNSTU1 */
		c = (range(a,-32,31));
		if (c + 0 < p->cost[_c6_NT]) {
			p->cost[_c6_NT] = c + 0;
			p->rule._c6 = 3;
		}
		/* reg: CNSTU1 */
		if (2 + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = 2 + 0;
			p->rule._reg = 8;
			_closure_reg(a, 2 + 0);
		}
		/* c5: CNSTU1 */
		c = (range(a,-16,15));
		if (c + 0 < p->cost[_c5_NT]) {
			p->cost[_c5_NT] = c + 0;
			p->rule._c5 = 3;
		}
		/* subc5: CNSTU1 */
		c = (range(a,-15,16));
		if (c + 0 < p->cost[_subc5_NT]) {
			p->cost[_subc5_NT] = c + 0;
			p->rule._subc5 = 3;
		}
		break;
	case 1047: /* CNSTP1 */
		/* c6: CNSTP1 */
		c = (range(a,-32,31));
		if (c + 0 < p->cost[_c6_NT]) {
			p->cost[_c6_NT] = c + 0;
			p->rule._c6 = 1;
		}
		/* reg: CNSTP1 */
		if (2 + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = 2 + 0;
			p->rule._reg = 9;
			_closure_reg(a, 2 + 0);
		}
		/* c5: CNSTP1 */
		c = (range(a,-16,15));
		if (c + 0 < p->cost[_c5_NT]) {
			p->cost[_c5_NT] = c + 0;
			p->rule._c5 = 1;
		}
		/* subc5: CNSTP1 */
		c = (range(a,-15,16));
		if (c + 0 < p->cost[_subc5_NT]) {
			p->cost[_subc5_NT] = c + 0;
			p->rule._subc5 = 1;
		}
		break;
	case 1061: /* ARGI1 */
		_label(LEFT_CHILD(a));
		/* stmt: ARGI1(reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + 2;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 32;
		}
		break;
	case 1062: /* ARGU1 */
		_label(LEFT_CHILD(a));
		/* stmt: ARGU1(reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + 2;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 34;
		}
		break;
	case 1063: /* ARGP1 */
		_label(LEFT_CHILD(a));
		/* stmt: ARGP1(reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + 2;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 33;
		}
		break;
	case 1077: /* ASGNI1 */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		if (	/* stmt: ASGNI1(VREGP,reg) */
			LEFT_CHILD(a)->op == 711 /* VREGP */
		) {
			c = ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 0;
			if (c + 0 < p->cost[_stmt_NT]) {
				p->cost[_stmt_NT] = c + 0;
				p->rule._stmt = 1;
			}
		}
		/* stmt: ASGNI1(spill,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_spill_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 3;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 5;
		}
		/* stmt: ASGNI1(addr,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_addr_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 1;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 8;
		}
		break;
	case 1078: /* ASGNU1 */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		if (	/* stmt: ASGNU1(VREGP,reg) */
			LEFT_CHILD(a)->op == 711 /* VREGP */
		) {
			c = ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 0;
			if (c + 0 < p->cost[_stmt_NT]) {
				p->cost[_stmt_NT] = c + 0;
				p->rule._stmt = 2;
			}
		}
		/* stmt: ASGNU1(spill,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_spill_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 3;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 6;
		}
		/* stmt: ASGNU1(addr,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_addr_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 1;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 9;
		}
		break;
	case 1079: /* ASGNP1 */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		if (	/* stmt: ASGNP1(VREGP,reg) */
			LEFT_CHILD(a)->op == 711 /* VREGP */
		) {
			c = ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 0;
			if (c + 0 < p->cost[_stmt_NT]) {
				p->cost[_stmt_NT] = c + 0;
				p->rule._stmt = 3;
			}
		}
		/* stmt: ASGNP1(spill,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_spill_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 3;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 7;
		}
		/* stmt: ASGNP1(addr,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_addr_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 1;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 10;
		}
		break;
	case 1093: /* INDIRI1 */
		_label(LEFT_CHILD(a));
		if (	/* reg: INDIRI1(VREGP) */
			LEFT_CHILD(a)->op == 711 /* VREGP */
		) {
			if (mayrecalc(a)) {
				struct _state *q = a->syms[RX]->u.t.cse->x.state;
				if (q->cost[_stmt_NT] == 0) {
					p->cost[_stmt_NT] = 0;
					p->rule._stmt = q->rule._stmt;
				}
				if (q->cost[_reg_NT] == 0) {
					p->cost[_reg_NT] = 0;
					p->rule._reg = q->rule._reg;
				}
				if (q->cost[_c6_NT] == 0) {
					p->cost[_c6_NT] = 0;
					p->rule._c6 = q->rule._c6;
				}
				if (q->cost[_addr_NT] == 0) {
					p->cost[_addr_NT] = 0;
					p->rule._addr = q->rule._addr;
				}
				if (q->cost[_spill_NT] == 0) {
					p->cost[_spill_NT] = 0;
					p->rule._spill = q->rule._spill;
				}
				if (q->cost[_c5_NT] == 0) {
					p->cost[_c5_NT] = 0;
					p->rule._c5 = q->rule._c5;
				}
				if (q->cost[_subc5_NT] == 0) {
					p->cost[_subc5_NT] = 0;
					p->rule._subc5 = q->rule._subc5;
				}
				if (q->cost[_jaddr_NT] == 0) {
					p->cost[_jaddr_NT] = 0;
					p->rule._jaddr = q->rule._jaddr;
				}
			}
			c = 0;
			if (c + 0 < p->cost[_reg_NT]) {
				p->cost[_reg_NT] = c + 0;
				p->rule._reg = 1;
				_closure_reg(a, c + 0);
			}
		}
		/* reg: INDIRI1(addr) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_addr_NT] + 1;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 10;
			_closure_reg(a, c + 0);
		}
		break;
	case 1094: /* INDIRU1 */
		_label(LEFT_CHILD(a));
		if (	/* reg: INDIRU1(VREGP) */
			LEFT_CHILD(a)->op == 711 /* VREGP */
		) {
			if (mayrecalc(a)) {
				struct _state *q = a->syms[RX]->u.t.cse->x.state;
				if (q->cost[_stmt_NT] == 0) {
					p->cost[_stmt_NT] = 0;
					p->rule._stmt = q->rule._stmt;
				}
				if (q->cost[_reg_NT] == 0) {
					p->cost[_reg_NT] = 0;
					p->rule._reg = q->rule._reg;
				}
				if (q->cost[_c6_NT] == 0) {
					p->cost[_c6_NT] = 0;
					p->rule._c6 = q->rule._c6;
				}
				if (q->cost[_addr_NT] == 0) {
					p->cost[_addr_NT] = 0;
					p->rule._addr = q->rule._addr;
				}
				if (q->cost[_spill_NT] == 0) {
					p->cost[_spill_NT] = 0;
					p->rule._spill = q->rule._spill;
				}
				if (q->cost[_c5_NT] == 0) {
					p->cost[_c5_NT] = 0;
					p->rule._c5 = q->rule._c5;
				}
				if (q->cost[_subc5_NT] == 0) {
					p->cost[_subc5_NT] = 0;
					p->rule._subc5 = q->rule._subc5;
				}
				if (q->cost[_jaddr_NT] == 0) {
					p->cost[_jaddr_NT] = 0;
					p->rule._jaddr = q->rule._jaddr;
				}
			}
			c = 0;
			if (c + 0 < p->cost[_reg_NT]) {
				p->cost[_reg_NT] = c + 0;
				p->rule._reg = 2;
				_closure_reg(a, c + 0);
			}
		}
		/* reg: INDIRU1(addr) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_addr_NT] + 1;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 11;
			_closure_reg(a, c + 0);
		}
		break;
	case 1095: /* INDIRP1 */
		_label(LEFT_CHILD(a));
		if (	/* reg: INDIRP1(VREGP) */
			LEFT_CHILD(a)->op == 711 /* VREGP */
		) {
			if (mayrecalc(a)) {
				struct _state *q = a->syms[RX]->u.t.cse->x.state;
				if (q->cost[_stmt_NT] == 0) {
					p->cost[_stmt_NT] = 0;
					p->rule._stmt = q->rule._stmt;
				}
				if (q->cost[_reg_NT] == 0) {
					p->cost[_reg_NT] = 0;
					p->rule._reg = q->rule._reg;
				}
				if (q->cost[_c6_NT] == 0) {
					p->cost[_c6_NT] = 0;
					p->rule._c6 = q->rule._c6;
				}
				if (q->cost[_addr_NT] == 0) {
					p->cost[_addr_NT] = 0;
					p->rule._addr = q->rule._addr;
				}
				if (q->cost[_spill_NT] == 0) {
					p->cost[_spill_NT] = 0;
					p->rule._spill = q->rule._spill;
				}
				if (q->cost[_c5_NT] == 0) {
					p->cost[_c5_NT] = 0;
					p->rule._c5 = q->rule._c5;
				}
				if (q->cost[_subc5_NT] == 0) {
					p->cost[_subc5_NT] = 0;
					p->rule._subc5 = q->rule._subc5;
				}
				if (q->cost[_jaddr_NT] == 0) {
					p->cost[_jaddr_NT] = 0;
					p->rule._jaddr = q->rule._jaddr;
				}
			}
			c = 0;
			if (c + 0 < p->cost[_reg_NT]) {
				p->cost[_reg_NT] = c + 0;
				p->rule._reg = 3;
				_closure_reg(a, c + 0);
			}
		}
		/* reg: INDIRP1(addr) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_addr_NT] + 1;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 12;
			_closure_reg(a, c + 0);
		}
		break;
	case 1157: /* CVII1 */
		_label(LEFT_CHILD(a));
		/* reg: CVII1(reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + (move(a));
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 54;
			_closure_reg(a, c + 0);
		}
		break;
	case 1158: /* CVIU1 */
		_label(LEFT_CHILD(a));
		if (	/* reg: CVIU1(INDIRU1(addr)) */
			LEFT_CHILD(a)->op == 1094 /* INDIRU1 */
		) {
			c = ((struct _state *)(LEFT_CHILD(LEFT_CHILD(a))->x.state))->cost[_addr_NT] + 1;
			if (c + 0 < p->cost[_reg_NT]) {
				p->cost[_reg_NT] = c + 0;
				p->rule._reg = 14;
				_closure_reg(a, c + 0);
			}
		}
		/* reg: CVIU1(reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + (move(a));
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 55;
			_closure_reg(a, c + 0);
		}
		break;
	case 1174: /* CVPU1 */
		_label(LEFT_CHILD(a));
		if (	/* reg: CVPU1(INDIRI1(addr)) */
			LEFT_CHILD(a)->op == 1093 /* INDIRI1 */
		) {
			c = ((struct _state *)(LEFT_CHILD(LEFT_CHILD(a))->x.state))->cost[_addr_NT] + 1;
			if (c + 0 < p->cost[_reg_NT]) {
				p->cost[_reg_NT] = c + 0;
				p->rule._reg = 13;
				_closure_reg(a, c + 0);
			}
		}
		/* reg: CVPU1(reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + (move(a));
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 52;
			_closure_reg(a, c + 0);
		}
		break;
	case 1205: /* CVUI1 */
		_label(LEFT_CHILD(a));
		if (	/* reg: CVUI1(INDIRU1(addr)) */
			LEFT_CHILD(a)->op == 1094 /* INDIRU1 */
		) {
			c = ((struct _state *)(LEFT_CHILD(LEFT_CHILD(a))->x.state))->cost[_addr_NT] + 1;
			if (c + 0 < p->cost[_reg_NT]) {
				p->cost[_reg_NT] = c + 0;
				p->rule._reg = 15;
				_closure_reg(a, c + 0);
			}
		}
		/* reg: CVUI1(reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + (move(a));
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 56;
			_closure_reg(a, c + 0);
		}
		break;
	case 1206: /* CVUU1 */
		_label(LEFT_CHILD(a));
		/* reg: CVUU1(reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + (move(a));
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 57;
			_closure_reg(a, c + 0);
		}
		break;
	case 1207: /* CVUP1 */
		_label(LEFT_CHILD(a));
		/* reg: CVUP1(reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + (move(a));
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 53;
			_closure_reg(a, c + 0);
		}
		break;
	case 1221: /* NEGI1 */
		_label(LEFT_CHILD(a));
		/* reg: NEGI1(reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + 2;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 48;
			_closure_reg(a, c + 0);
		}
		break;
	case 1237: /* CALLI1 */
		_label(LEFT_CHILD(a));
		/* reg: CALLI1(jaddr) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_jaddr_NT] + 1;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 58;
			_closure_reg(a, c + 0);
		}
		/* reg: CALLI1(reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + 1;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 61;
			_closure_reg(a, c + 0);
		}
		break;
	case 1238: /* CALLU1 */
		_label(LEFT_CHILD(a));
		/* reg: CALLU1(jaddr) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_jaddr_NT] + 1;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 60;
			_closure_reg(a, c + 0);
		}
		/* reg: CALLU1(reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + 1;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 63;
			_closure_reg(a, c + 0);
		}
		break;
	case 1239: /* CALLP1 */
		_label(LEFT_CHILD(a));
		/* reg: CALLP1(jaddr) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_jaddr_NT] + 1;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 59;
			_closure_reg(a, c + 0);
		}
		/* reg: CALLP1(reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + 1;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 62;
			_closure_reg(a, c + 0);
		}
		break;
	case 1253: /* LOADI1 */
		_label(LEFT_CHILD(a));
		/* reg: LOADI1(reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + (move(a));
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 49;
			_closure_reg(a, c + 0);
		}
		break;
	case 1254: /* LOADU1 */
		_label(LEFT_CHILD(a));
		/* reg: LOADU1(reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + (move(a));
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 50;
			_closure_reg(a, c + 0);
		}
		break;
	case 1255: /* LOADP1 */
		_label(LEFT_CHILD(a));
		/* reg: LOADP1(reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + (move(a));
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 51;
			_closure_reg(a, c + 0);
		}
		break;
	case 1269: /* RETI1 */
		_label(LEFT_CHILD(a));
		/* stmt: RETI1(reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + 1;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 28;
		}
		break;
	case 1270: /* RETU1 */
		_label(LEFT_CHILD(a));
		/* stmt: RETU1(reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + 1;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 29;
		}
		break;
	case 1271: /* RETP1 */
		_label(LEFT_CHILD(a));
		/* stmt: RETP1(reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + 1;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 30;
		}
		break;
	case 1287: /* ADDRGP1 */
		/* reg: ADDRGP1 */
		if (2 + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = 2 + 0;
			p->rule._reg = 6;
			_closure_reg(a, 2 + 0);
		}
		/* jaddr: ADDRGP1 */
		if (0 + 0 < p->cost[_jaddr_NT]) {
			p->cost[_jaddr_NT] = 0 + 0;
			p->rule._jaddr = 1;
		}
		break;
	case 1303: /* ADDRFP1 */
		/* addr: ADDRFP1 */
		c = (range(a, -32, 31));
		if (c + 0 < p->cost[_addr_NT]) {
			p->cost[_addr_NT] = c + 0;
			p->rule._addr = 5;
		}
		/* reg: ADDRFP1 */
		if (2 + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = 2 + 0;
			p->rule._reg = 4;
			_closure_reg(a, 2 + 0);
		}
		break;
	case 1319: /* ADDRLP1 */
		/* addr: ADDRLP1 */
		c = (range(a, -32, 31));
		if (c + 0 < p->cost[_addr_NT]) {
			p->cost[_addr_NT] = c + 0;
			p->rule._addr = 6;
		}
		/* reg: ADDRLP1 */
		if (2 + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = 2 + 0;
			p->rule._reg = 5;
			_closure_reg(a, 2 + 0);
		}
		/* spill: ADDRLP1 */
		c = (!range(a,-32,31));
		if (c + 0 < p->cost[_spill_NT]) {
			p->cost[_spill_NT] = c + 0;
			p->rule._spill = 1;
		}
		break;
	case 1333: /* ADDI1 */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* addr: ADDI1(reg,c6) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_c6_NT] + 0;
		if (c + 0 < p->cost[_addr_NT]) {
			p->cost[_addr_NT] = c + 0;
			p->rule._addr = 1;
		}
		/* reg: ADDI1(reg,c5) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_c5_NT] + 1;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 22;
			_closure_reg(a, c + 0);
		}
		/* reg: ADDI1(reg,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 1;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 27;
			_closure_reg(a, c + 0);
		}
		break;
	case 1334: /* ADDU1 */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* addr: ADDU1(reg,c6) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_c6_NT] + 0;
		if (c + 0 < p->cost[_addr_NT]) {
			p->cost[_addr_NT] = c + 0;
			p->rule._addr = 2;
		}
		/* reg: ADDU1(reg,c5) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_c5_NT] + 1;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 24;
			_closure_reg(a, c + 0);
		}
		/* reg: ADDU1(reg,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 1;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 29;
			_closure_reg(a, c + 0);
		}
		break;
	case 1335: /* ADDP1 */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* addr: ADDP1(reg,c6) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_c6_NT] + 0;
		if (c + 0 < p->cost[_addr_NT]) {
			p->cost[_addr_NT] = c + 0;
			p->rule._addr = 3;
		}
		/* reg: ADDP1(reg,c5) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_c5_NT] + 1;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 23;
			_closure_reg(a, c + 0);
		}
		/* reg: ADDP1(reg,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 1;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 28;
			_closure_reg(a, c + 0);
		}
		break;
	case 1349: /* SUBI1 */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* reg: SUBI1(reg,subc5) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_subc5_NT] + 1;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 36;
			_closure_reg(a, c + 0);
		}
		/* reg: SUBI1(reg,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 3;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 39;
			_closure_reg(a, c + 0);
		}
		break;
	case 1350: /* SUBU1 */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* reg: SUBU1(reg,subc5) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_subc5_NT] + 1;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 38;
			_closure_reg(a, c + 0);
		}
		/* reg: SUBU1(reg,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 3;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 41;
			_closure_reg(a, c + 0);
		}
		break;
	case 1351: /* SUBP1 */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* reg: SUBP1(reg,subc5) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_subc5_NT] + 1;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 37;
			_closure_reg(a, c + 0);
		}
		/* reg: SUBP1(reg,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 3;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 40;
			_closure_reg(a, c + 0);
		}
		break;
	case 1365: /* LSHI1 */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* reg: LSHI1(reg,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 6;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 42;
			_closure_reg(a, c + 0);
		}
		break;
	case 1366: /* LSHU1 */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* reg: LSHU1(reg,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 6;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 43;
			_closure_reg(a, c + 0);
		}
		break;
	case 1381: /* MODI1 */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* reg: MODI1(reg,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 15;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 18;
			_closure_reg(a, c + 0);
		}
		break;
	case 1382: /* MODU1 */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* reg: MODU1(reg,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 15;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 19;
			_closure_reg(a, c + 0);
		}
		break;
	case 1397: /* RSHI1 */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* reg: RSHI1(reg,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 15;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 44;
			_closure_reg(a, c + 0);
		}
		break;
	case 1398: /* RSHU1 */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* reg: RSHU1(reg,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 15;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 45;
			_closure_reg(a, c + 0);
		}
		break;
	case 1413: /* BANDI1 */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* reg: BANDI1(reg,c5) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_c5_NT] + 1;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 25;
			_closure_reg(a, c + 0);
		}
		/* reg: BANDI1(reg,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 1;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 30;
			_closure_reg(a, c + 0);
		}
		break;
	case 1414: /* BANDU1 */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* reg: BANDU1(reg,c5) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_c5_NT] + 1;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 26;
			_closure_reg(a, c + 0);
		}
		/* reg: BANDU1(reg,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 1;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 31;
			_closure_reg(a, c + 0);
		}
		break;
	case 1429: /* BCOMI1 */
		_label(LEFT_CHILD(a));
		/* reg: BCOMI1(reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + 1;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 46;
			_closure_reg(a, c + 0);
		}
		break;
	case 1430: /* BCOMU1 */
		_label(LEFT_CHILD(a));
		/* reg: BCOMU1(reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + 1;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 47;
			_closure_reg(a, c + 0);
		}
		break;
	case 1445: /* BORI1 */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* reg: BORI1(reg,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 5;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 32;
			_closure_reg(a, c + 0);
		}
		break;
	case 1446: /* BORU1 */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* reg: BORU1(reg,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 5;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 33;
			_closure_reg(a, c + 0);
		}
		break;
	case 1461: /* BXORI1 */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* reg: BXORI1(reg,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 10;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 34;
			_closure_reg(a, c + 0);
		}
		break;
	case 1462: /* BXORU1 */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* reg: BXORU1(reg,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 10;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 35;
			_closure_reg(a, c + 0);
		}
		break;
	case 1477: /* DIVI1 */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* reg: DIVI1(reg,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 15;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 16;
			_closure_reg(a, c + 0);
		}
		break;
	case 1478: /* DIVU1 */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* reg: DIVU1(reg,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 15;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 17;
			_closure_reg(a, c + 0);
		}
		break;
	case 1493: /* MULI1 */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* reg: MULI1(reg,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 10;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 20;
			_closure_reg(a, c + 0);
		}
		break;
	case 1494: /* MULU1 */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* reg: MULU1(reg,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 10;
		if (c + 0 < p->cost[_reg_NT]) {
			p->cost[_reg_NT] = c + 0;
			p->rule._reg = 21;
			_closure_reg(a, c + 0);
		}
		break;
	case 1509: /* EQI1 */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* stmt: EQI1(reg,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 1;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 14;
		}
		break;
	case 1510: /* EQU1 */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* stmt: EQU1(reg,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 1;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 15;
		}
		break;
	case 1525: /* GEI1 */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* stmt: GEI1(reg,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 1;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 16;
		}
		break;
	case 1526: /* GEU1 */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* stmt: GEU1(reg,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 1;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 17;
		}
		break;
	case 1541: /* GTI1 */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* stmt: GTI1(reg,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 1;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 18;
		}
		break;
	case 1542: /* GTU1 */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* stmt: GTU1(reg,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 1;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 19;
		}
		break;
	case 1557: /* LEI1 */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* stmt: LEI1(reg,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 1;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 20;
		}
		break;
	case 1558: /* LEU1 */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* stmt: LEU1(reg,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 1;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 21;
		}
		break;
	case 1573: /* LTI1 */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* stmt: LTI1(reg,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 1;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 22;
		}
		break;
	case 1574: /* LTU1 */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* stmt: LTU1(reg,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 1;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 23;
		}
		break;
	case 1589: /* NEI1 */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* stmt: NEI1(reg,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 1;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 24;
		}
		break;
	case 1590: /* NEU1 */
		_label(LEFT_CHILD(a));
		_label(RIGHT_CHILD(a));
		/* stmt: NEU1(reg,reg) */
		c = ((struct _state *)(LEFT_CHILD(a)->x.state))->cost[_reg_NT] + ((struct _state *)(RIGHT_CHILD(a)->x.state))->cost[_reg_NT] + 1;
		if (c + 0 < p->cost[_stmt_NT]) {
			p->cost[_stmt_NT] = c + 0;
			p->rule._stmt = 25;
		}
		break;
	default:
		fatal("_label", "Bad terminal %d\n", OP_LABEL(a));
	}
}

static void _kids(NODEPTR_TYPE p, int eruleno, NODEPTR_TYPE kids[]) {
	if (!p)
		fatal("_kids", "Null tree\n", 0);
	if (!kids)
		fatal("_kids", "Null kids\n", 0);
	switch (eruleno) {
	case 85: /* jaddr: ADDRGP1 */
	case 84: /* stmt: LABELV */
	case 47: /* subc5: CNSTU1 */
	case 46: /* subc5: CNSTI1 */
	case 45: /* subc5: CNSTP1 */
	case 44: /* c5: CNSTU1 */
	case 43: /* c5: CNSTI1 */
	case 42: /* c5: CNSTP1 */
	case 23: /* spill: ADDRLP1 */
	case 22: /* reg: CNSTP1 */
	case 21: /* reg: CNSTU1 */
	case 20: /* reg: CNSTI1 */
	case 19: /* reg: ADDRGP1 */
	case 18: /* reg: ADDRLP1 */
	case 17: /* reg: ADDRFP1 */
	case 16: /* addr: ADDRLP1 */
	case 15: /* addr: ADDRFP1 */
	case 10: /* c6: CNSTU1 */
	case 9: /* c6: CNSTI1 */
	case 8: /* c6: CNSTP1 */
	case 3: /* reg: INDIRP1(VREGP) */
	case 2: /* reg: INDIRU1(VREGP) */
	case 1: /* reg: INDIRI1(VREGP) */
		break;
	case 6: /* stmt: ASGNP1(VREGP,reg) */
	case 5: /* stmt: ASGNU1(VREGP,reg) */
	case 4: /* stmt: ASGNI1(VREGP,reg) */
		kids[0] = RIGHT_CHILD(p);
		break;
	case 14: /* addr: reg */
	case 7: /* stmt: reg */
		kids[0] = p;
		break;
	case 99: /* stmt: NEU1(reg,reg) */
	case 98: /* stmt: NEI1(reg,reg) */
	case 97: /* stmt: LTU1(reg,reg) */
	case 96: /* stmt: LTI1(reg,reg) */
	case 95: /* stmt: LEU1(reg,reg) */
	case 94: /* stmt: LEI1(reg,reg) */
	case 93: /* stmt: GTU1(reg,reg) */
	case 92: /* stmt: GTI1(reg,reg) */
	case 91: /* stmt: GEU1(reg,reg) */
	case 90: /* stmt: GEI1(reg,reg) */
	case 89: /* stmt: EQU1(reg,reg) */
	case 88: /* stmt: EQI1(reg,reg) */
	case 71: /* reg: RSHU1(reg,reg) */
	case 70: /* reg: RSHI1(reg,reg) */
	case 69: /* reg: LSHU1(reg,reg) */
	case 68: /* reg: LSHI1(reg,reg) */
	case 67: /* reg: SUBU1(reg,reg) */
	case 66: /* reg: SUBP1(reg,reg) */
	case 65: /* reg: SUBI1(reg,reg) */
	case 64: /* reg: SUBU1(reg,subc5) */
	case 63: /* reg: SUBP1(reg,subc5) */
	case 62: /* reg: SUBI1(reg,subc5) */
	case 61: /* reg: BXORU1(reg,reg) */
	case 60: /* reg: BXORI1(reg,reg) */
	case 59: /* reg: BORU1(reg,reg) */
	case 58: /* reg: BORI1(reg,reg) */
	case 57: /* reg: BANDU1(reg,reg) */
	case 56: /* reg: BANDI1(reg,reg) */
	case 55: /* reg: ADDU1(reg,reg) */
	case 54: /* reg: ADDP1(reg,reg) */
	case 53: /* reg: ADDI1(reg,reg) */
	case 52: /* reg: BANDU1(reg,c5) */
	case 51: /* reg: BANDI1(reg,c5) */
	case 50: /* reg: ADDU1(reg,c5) */
	case 49: /* reg: ADDP1(reg,c5) */
	case 48: /* reg: ADDI1(reg,c5) */
	case 41: /* reg: MULU1(reg,reg) */
	case 40: /* reg: MULI1(reg,reg) */
	case 39: /* reg: MODU1(reg,reg) */
	case 38: /* reg: MODI1(reg,reg) */
	case 37: /* reg: DIVU1(reg,reg) */
	case 36: /* reg: DIVI1(reg,reg) */
	case 29: /* stmt: ASGNP1(addr,reg) */
	case 28: /* stmt: ASGNU1(addr,reg) */
	case 27: /* stmt: ASGNI1(addr,reg) */
	case 26: /* stmt: ASGNP1(spill,reg) */
	case 25: /* stmt: ASGNU1(spill,reg) */
	case 24: /* stmt: ASGNI1(spill,reg) */
	case 13: /* addr: ADDP1(reg,c6) */
	case 12: /* addr: ADDU1(reg,c6) */
	case 11: /* addr: ADDI1(reg,c6) */
		kids[0] = LEFT_CHILD(p);
		kids[1] = RIGHT_CHILD(p);
		break;
	case 114: /* stmt: ARGU1(reg) */
	case 113: /* stmt: ARGP1(reg) */
	case 112: /* stmt: ARGI1(reg) */
	case 111: /* stmt: RETV(reg) */
	case 110: /* stmt: RETP1(reg) */
	case 109: /* stmt: RETU1(reg) */
	case 108: /* stmt: RETI1(reg) */
	case 107: /* stmt: CALLV(reg) */
	case 106: /* reg: CALLU1(reg) */
	case 105: /* reg: CALLP1(reg) */
	case 104: /* reg: CALLI1(reg) */
	case 103: /* stmt: CALLV(jaddr) */
	case 102: /* reg: CALLU1(jaddr) */
	case 101: /* reg: CALLP1(jaddr) */
	case 100: /* reg: CALLI1(jaddr) */
	case 87: /* stmt: JUMPV(reg) */
	case 86: /* stmt: JUMPV(jaddr) */
	case 83: /* reg: CVUU1(reg) */
	case 82: /* reg: CVUI1(reg) */
	case 81: /* reg: CVIU1(reg) */
	case 80: /* reg: CVII1(reg) */
	case 79: /* reg: CVUP1(reg) */
	case 78: /* reg: CVPU1(reg) */
	case 77: /* reg: LOADP1(reg) */
	case 76: /* reg: LOADU1(reg) */
	case 75: /* reg: LOADI1(reg) */
	case 74: /* reg: NEGI1(reg) */
	case 73: /* reg: BCOMU1(reg) */
	case 72: /* reg: BCOMI1(reg) */
	case 32: /* reg: INDIRP1(addr) */
	case 31: /* reg: INDIRU1(addr) */
	case 30: /* reg: INDIRI1(addr) */
		kids[0] = LEFT_CHILD(p);
		break;
	case 115: /* stmt: ARGB(INDIRB(reg)) */
	case 35: /* reg: CVUI1(INDIRU1(addr)) */
	case 34: /* reg: CVIU1(INDIRU1(addr)) */
	case 33: /* reg: CVPU1(INDIRI1(addr)) */
		kids[0] = LEFT_CHILD(LEFT_CHILD(p));
		break;
	case 116: /* stmt: ASGNB(reg,INDIRB(reg)) */
		kids[0] = LEFT_CHILD(p);
		kids[1] = LEFT_CHILD(RIGHT_CHILD(p));
		break;
	default:
		fatal("_kids", "Bad rule number %d\n", eruleno);
	}
}


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
	print("LD R0, GLOBAL_MAIN_POINTER\n");		//puts x3000 in r6
	print("jsrr R0\n");
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
#if 0
	if (file) {
		print(".file 2,\"%s\"\n", file);
		currentfile = file;
	}
#endif
}

/* stabline - emit stab entry for source coordinate *cp */
static void stabline(Coordinate *cp) {
#if 0
	if (cp->file && cp->file != currentfile) {
		print(".file 2,\"%s\"\n", cp->file);
		currentfile = cp->file;
	}
	print(".loc 2,%d\n", cp->y);
#endif
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
