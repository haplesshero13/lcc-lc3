.Orig x3000
INIT_CODE
LEA R6, #-1
ADD R5, R6, #0
ADD R6, R6, R6
ADD R6, R6, R6
ADD R6, R6, R5
ADD R6, R6, #-1
ADD R5, R5, R5
ADD R5, R6, #0
LD R4, GLOBAL_DATA_POINTER
LD R7, GLOBAL_MAIN_POINTER
jsrr R7
HALT

GLOBAL_DATA_POINTER .FILL GLOBAL_DATA_START
GLOBAL_MAIN_POINTER .FILL main
;;;;;;;;;;;;;;;;;;;;;;;;;;;;main;;;;;;;;;;;;;;;;;;;;;;;;;;;;
main
ADD R6, R6, #-2
STR R7, R6, #0
ADD R6, R6, #-1
STR R5, R6, #0
ADD R5, R6, #-1

ADD R6, R6, #-2
ADD R7, R4, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #12
ldr R7, R7, #0
str R7, R5, #-1
ADD R0, R4, #15
ADD R0, R0, #15
ADD R0, R0, #4
LDR R0, R0, #0
JMP R0 ;addrgp unconditional
lc3_L5
ADD R7, R4, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #12
ldr R7, R7, #0
str R7, R5, #0
ADD R0, R4, #15
ADD R0, R0, #15
ADD R0, R0, #2
LDR R0, R0, #0
JMP R0 ;addrgp unconditional
lc3_L10
ldr R7, R5, #0
ldr R3, R5, #-1
ADD R2, R4, #15
ADD R2, R2, #10
add R3, R3, R2
ldr R3, R3, #0
add R7, R7, R3
ldr R7, R7, #0
;check
ADD R6, R6, #-1
STR R7, R6, #0
ADD R7, R4, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #7
;check
ADD R6, R6, #-1
STR R7, R6, #0
ADD R0, R4, #15
ADD R0, R0, #15
ADD R0, R0, #15
ADD R0, R0, #14
LDR R0, R0, #0
jsrr R0
LDR R7, R6, #0
ADD R6, R6, #1
lc3_L11
ldr R7, R5, #0
ADD R3, R4, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #11
ldr R3, R3, #0
add R7, R7, R3
str R7, R5, #0
lc3_L13
ldr R7, R5, #0
ldr R3, R5, #-1
ADD R2, R4, #15
ADD R2, R2, #10
add R3, R3, R2
ldr R3, R3, #0
add R7, R7, R3
ldr R7, R7, #0
ADD R3, R4, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #12
ldr R3, R3, #0
NOT R7, R7
ADD R7, R7, #1
ADD R7, R7, R3
BRz L18
ADD R7, R4, #15
ADD R7, R7, #15
LDR R7, R7, #0
jmp R7
L18
ADD R7, R4, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #5
;check
ADD R6, R6, #-1
STR R7, R6, #0
ADD R0, R4, #15
ADD R0, R0, #15
ADD R0, R0, #15
ADD R0, R0, #14
LDR R0, R0, #0
jsrr R0
LDR R7, R6, #0
ADD R6, R6, #1
lc3_L6
ldr R7, R5, #-1
ADD R3, R4, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #11
ldr R3, R3, #0
add R7, R7, R3
str R7, R5, #-1
lc3_L8
ldr R7, R5, #-1
ADD R3, R4, #15
ADD R3, R3, #10
add R7, R7, R3
ldr R7, R7, #0
ADD R3, R4, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #4
ldr R3, R3, #0
NOT R7, R7
ADD R7, R7, #1
ADD R7, R7, R3
BRz L19
ADD R7, R4, #15
ADD R7, R7, #14
LDR R7, R7, #0
jmp R7
L19
ADD R0, R4, #15
ADD R0, R0, #15
ADD R0, R0, #11
LDR R0, R0, #0
jsrr R0
LDR R7, R6, #0
ADD R6, R6, #1
ADD R7, R4, #9
ldr R7, R7, #0
;check
ADD R6, R6, #-1
STR R7, R6, #0
ADD R0, R4, #15
ADD R0, R0, #15
ADD R0, R0, #15
ADD R0, R0, #1
LDR R0, R0, #0
jsrr R0
LDR R7, R6, #0
ADD R6, R6, #1
ADD R7, R4, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #12
ldr R7, R7, #0
lc3_L4
STR R7, R5, #3
ADD R6, R5, #1
LDR R5, R6, #0
ADD R6, R6, #1
LDR R7, R6, #0
ADD R6, R6, #1
RET

;;;;;;;;;;;;;;;;;;;;;;;;;;;;f;;;;;;;;;;;;;;;;;;;;;;;;;;;;
lc3_f
ADD R6, R6, #-2
STR R7, R6, #0
ADD R6, R6, #-1
STR R5, R6, #0
ADD R5, R6, #-1

ADD R6, R6, #-1
ADD R7, R4, #15
ADD R7, R7, #15
ADD R7, R7, #6
str R7, R5, #0
ADD R0, R4, #15
ADD R0, R0, #15
ADD R0, R0, #14
LDR R0, R0, #0
JMP R0 ;addrgp unconditional
lc3_L26
ldr R7, R5, #0
ldr R7, R7, #0
;check
ADD R6, R6, #-1
STR R7, R6, #0
ADD R7, R4, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #12
;check
ADD R6, R6, #-1
STR R7, R6, #0
ADD R0, R4, #15
ADD R0, R0, #15
ADD R0, R0, #15
ADD R0, R0, #14
LDR R0, R0, #0
jsrr R0
LDR R7, R6, #0
ADD R6, R6, #1
lc3_L27
ldr R7, R5, #0
ADD R3, R4, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #11
ldr R3, R3, #0
add R7, R7, R3
str R7, R5, #0
lc3_L29
ldr R7, R5, #0
ldr R7, R7, #0
ADD R3, R4, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #4
ldr R3, R3, #0
NOT R7, R7
ADD R7, R7, #1
ADD R7, R7, R3
BRz L32
ADD R7, R4, #15
ADD R7, R7, #15
ADD R7, R7, #12
LDR R7, R7, #0
jmp R7
L32
ADD R7, R4, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #12
ldr R7, R7, #0
lc3_L20
STR R7, R5, #3
ADD R6, R5, #1
LDR R5, R6, #0
ADD R6, R6, #1
LDR R7, R6, #0
ADD R6, R6, #1
RET

;;;;;;;;;;;;;;;;;;;;;;;;;;;;g;;;;;;;;;;;;;;;;;;;;;;;;;;;;
lc3_g
ADD R6, R6, #-2
STR R7, R6, #0
ADD R6, R6, #-1
STR R5, R6, #0
ADD R5, R6, #-1

ADD R6, R6, #-1
ADD R0, R4, #15
ADD R0, R0, #15
ADD R0, R0, #15
ADD R0, R0, #7
LDR R0, R0, #0
JMP R0 ;addrgp unconditional
lc3_L34
ADD R7, R4, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #12
ldr R7, R7, #0
str R7, R5, #0
ADD R0, R4, #15
ADD R0, R0, #15
ADD R0, R0, #15
ADD R0, R0, #5
LDR R0, R0, #0
JMP R0 ;addrgp unconditional
lc3_L38
ldr R7, R5, #0
ldr R3, R5, #4
add R7, R7, R3
ldr R7, R7, #0
;check
ADD R6, R6, #-1
STR R7, R6, #0
ADD R7, R4, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #7
;check
ADD R6, R6, #-1
STR R7, R6, #0
ADD R0, R4, #15
ADD R0, R0, #15
ADD R0, R0, #15
ADD R0, R0, #14
LDR R0, R0, #0
jsrr R0
LDR R7, R6, #0
ADD R6, R6, #1
lc3_L39
ldr R7, R5, #0
ADD R3, R4, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #11
ldr R3, R3, #0
add R7, R7, R3
str R7, R5, #0
lc3_L41
ldr R7, R5, #0
ADD R3, R4, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #6
ldr R3, R3, #0
NOT R7, R7
ADD R7, R7, #1
ADD R7, R7, R3
BRnz L47
ADD R7, R4, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #3
LDR R7, R7, #0
jmp R7
L47
ldr R7, R5, #4
ADD R3, R4, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #1
ldr R3, R3, #0
add R7, R7, R3
;check
ADD R6, R6, #-1
STR R7, R6, #0
ADD R7, R4, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #2
;check
ADD R6, R6, #-1
STR R7, R6, #0
ADD R0, R4, #15
ADD R0, R0, #15
ADD R0, R0, #15
ADD R0, R0, #14
LDR R0, R0, #0
jsrr R0
LDR R7, R6, #0
ADD R6, R6, #1
lc3_L35
ldr R7, R5, #4
ADD R3, R4, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ldr R3, R3, #0
add R7, R7, R3
str R7, R5, #4
lc3_L37
ldr R7, R5, #4
ldr R7, R7, #0
ADD R3, R4, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #12
ldr R3, R3, #0
NOT R7, R7
ADD R7, R7, #1
ADD R7, R7, R3
BRz L48
ADD R7, R4, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #2
LDR R7, R7, #0
jmp R7
L48
ADD R0, R4, #15
ADD R0, R0, #15
ADD R0, R0, #15
ADD R0, R0, #9
LDR R0, R0, #0
jsrr R0
LDR R7, R6, #0
ADD R6, R6, #1
ADD R7, R4, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #12
ldr R7, R7, #0
lc3_L33
STR R7, R5, #3
ADD R6, R5, #1
LDR R5, R6, #0
ADD R6, R6, #1
LDR R7, R6, #0
ADD R6, R6, #1
RET

;;;;;;;;;;;;;;;;;;;;;;;;;;;;h;;;;;;;;;;;;;;;;;;;;;;;;;;;;
lc3_h
ADD R6, R6, #-2
STR R7, R6, #0
ADD R6, R6, #-1
STR R5, R6, #0
ADD R5, R6, #-1

ADD R6, R6, #-1
ADD R7, R4, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #12
ldr R7, R7, #0
str R7, R5, #0
ADD R0, R4, #15
ADD R0, R0, #15
ADD R0, R0, #15
ADD R0, R0, #12
LDR R0, R0, #0
JMP R0 ;addrgp unconditional
lc3_L50
ADD R7, R4, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #1
ldr R7, R7, #0
ldr R3, R5, #0
;bef prologue R7, R7, R3
ADD R6, R6, #-1
STR R0, R6, #0
;aft prologue x=0 y=7 z=3
;mul R7, R7, R3
ADD R6, R6, #-1
STR R3, R6, #0
AND R0, R0, #0
ADD R3, R3, #0
BRz L61
BRp L60
NOT R3, R3
ADD R3, R3, #1
L60
ADD R0, R0, R7
ADD R3, R3, #-1
BRnp L60
L61
LDR R3, R6, #0
ADD R6, R6, #1
ADD R3, R3, #0
BRzp L62
NOT R0, R0
ADD R0, R0, #1
L62
;bef epilogue x=0 y=7 z=3
ADD R7, R0, #0
LDR R0, R6, #0
ADD R6, R6, #1
;aft epilogue x=0 y=7 z=3
ADD R3, R4, #3
add R3, R7, R3
;check
ADD R6, R6, #-1
STR R3, R6, #0
ADD R3, R4, #2
add R3, R7, R3
ldr R3, R3, #0
;check
ADD R6, R6, #-1
STR R3, R6, #0
ADD R3, R4, #1
add R3, R7, R3
ldr R3, R3, #0
;check
ADD R6, R6, #-1
STR R3, R6, #0
ADD R3, R4, #0
add R7, R7, R3
ldr R7, R7, #0
;check
ADD R6, R6, #-1
STR R7, R6, #0
ADD R7, R4, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #2
;check
ADD R6, R6, #-1
STR R7, R6, #0
ADD R0, R4, #15
ADD R0, R0, #15
ADD R0, R0, #15
ADD R0, R0, #14
LDR R0, R0, #0
jsrr R0
LDR R7, R6, #0
ADD R6, R6, #1
lc3_L51
ldr R7, R5, #0
ADD R3, R4, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #11
ldr R3, R3, #0
add R7, R7, R3
str R7, R5, #0
lc3_L53
ldr R7, R5, #0
ADD R3, R4, #15
ADD R3, R3, #15
ADD R3, R3, #15
ADD R3, R3, #15
ldr R3, R3, #0
NOT R7, R7
ADD R7, R7, #1
ADD R7, R7, R3
BRnz L63
ADD R7, R4, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #10
LDR R7, R7, #0
jmp R7
L63
ADD R7, R4, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #15
ADD R7, R7, #12
ldr R7, R7, #0
lc3_L49
STR R7, R5, #3
ADD R6, R5, #1
LDR R5, R6, #0
ADD R6, R6, #1
LDR R7, R6, #0
ADD R6, R6, #1
RET



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;	void printf(const char *format, ...)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

PRINTF_PERCENT .FILL -37
PRINTF_C .FILL -99
PRINTF_D .FILL -100
PRINTF_S .FILL -115
PRINTF_0 .FILL 48 		;postive ascii value of '0'
PRINTF_MINUS .FILL 45  
PRINTF_BUF .BLKW 7
 

lc3_printf
ADD R6, R6, #-2
STR R7, R6, #0		;return address
ADD R6, R6, #-1
STR R5, R6, #0
ADD R5, R6, #-1

ADD R6, R6, #-1
STR R4, R6, #0

ADD R5, R5, #4		;cheating with the bp (no longer bp)
LDR R4, R5, #0		;got addr of format string


PRINTF_LOOP	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

LDR R0, R4, #0

ADD R0, R0, #0		;End of string? (0x0000)
BRz PRINTF_DONE

ADD R2, R0, #0
LD R1, PRINTF_PERCENT
ADD R2, R2, R1
BRnp PRINTF_CHAR		

ADD R4, R4, #1
LDR R0, R4, #0
;is it %c?
ADD R2, R0, #0
LD R3, PRINTF_C
ADD R2, R2, R3
BRnp PRINTF_CHECKSTR
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;%c
ADD R5, R5, #1
LDR R0, R5, #0

PRINTF_CHAR
OUT

ADD R4, R4, #1
BRnzp PRINTF_LOOP

PRINTF_CHECKSTR
;is it %s?
ADD R2, R0, #0
LD R7, PRINTF_S
ADD R2, R2, R7
BRnp PRINTF_CHECKDEC		

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;%s

ADD R5, R5, #1
LDR R0, R5, #0
PUTS

ADD R4, R4, #1
BRnzp PRINTF_LOOP

PRINTF_CHECKDEC
;is it %s?
ADD R2, R0, #0
LD R7, PRINTF_D
ADD R2, R2, R7
BRnp PRINTF_ERROR


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;%d

LEA R7, PRINTF_BUF
ADD R7, R7, #5 

AND R2, R2, #0
ADD R2, R2, #-10		;going to divide by 10 by using sub loop

ADD R5, R5, #1			;acquire the binary number
LDR R0, R5, #0

ADD R0, R0, #0
BRzp PRINTF_DECPOS 

NOT R0, R0				;make num positive for sub loop
ADD R0, R0, #1

PRINTF_DECPOS

AND R3, R3, #0
ADD R3, R3, #-1

PRINTF_DIVLOOP
ADD R3, R3, #1			;num/10 
ADD R0, R0, R2			;R0 = num % 10 - 10
BRzp PRINTF_DIVLOOP

ADD R3, R3, #0
BRz PRINTF_LASTDIGIT

LD R1, PRINTF_0
ADD R1, R1, R0
ADD R1, R1, #10
STR R1, R7, #0
ADD R7, R7, #-1			;stored ascii value of one digit

ADD R0, R3, #0			;num/10

BRnzp PRINTF_DECPOS

PRINTF_LASTDIGIT

LD R1, PRINTF_0
ADD R1, R1, R0
ADD R1, R1, #10
STR R1, R7, #0			;stored ascii value of highest order digit

LDR R0, R5, #0
ADD R0, R0, #0
BRzp PRINTF_DECSTRING

LD R0, PRINTF_MINUS		;num was negative
ADD R7, R7, #-1
STR R0, R7, #0			;stored ascii value negative sign

PRINTF_DECSTRING		;print the calculated string
ADD R0, R7, #0
PUTS

ADD R4, R4, #1
BRnzp PRINTF_LOOP

PRINTF_ERROR
PRINTF_DONE

LDR R4, R6, #0		;restore R4
ADD R6, R6, #1

LDR R5, R6, #0		;restore bp
ADD R6, R6, #1

LDR R7, R6, #0		;restore ret addr
ADD R6, R6, #1

RET
GLOBAL_DATA_START
words .FILL #4
.FILL #5
.BLKW 1
.FILL #102
.FILL #111
.FILL #114
.BLKW 3
wordlist .FILL words
x .FILL #1
.FILL #2
.FILL #3
.FILL #4
.FILL #0
.FILL #5
.FILL #6
.BLKW 3
.FILL #7
.BLKW 4
y .FILL x
.FILL x+5
.FILL x+10
.FILL x0
L5 .FILL lc3_L5
L10 .FILL lc3_L10
L11 .FILL lc3_L11
L13 .FILL lc3_L13
L6 .FILL lc3_L6
L8 .FILL lc3_L8
L4 .FILL lc3_L4
L21 .FILL L22
.FILL L23
.FILL L24
.FILL L25
.FILL x0
f .FILL lc3_f
L26 .FILL lc3_L26
L27 .FILL lc3_L27
L29 .FILL lc3_L29
L20 .FILL lc3_L20
g .FILL lc3_g
L34 .FILL lc3_L34
L38 .FILL lc3_L38
L39 .FILL lc3_L39
L41 .FILL lc3_L41
L35 .FILL lc3_L35
L37 .FILL lc3_L37
L33 .FILL lc3_L33
h .FILL lc3_h
L50 .FILL lc3_L50
L51 .FILL lc3_L51
L53 .FILL lc3_L53
L49 .FILL lc3_L49
printf .FILL lc3_printf
L59 .FILL #1
L58 .FILL #9
L54 .STRINGZ "%d %d %d %s\n"
L46 .FILL #9
L45 .FILL #3
L44 .STRINGZ "%s\n"
L43 .FILL #3
L42 .STRINGZ "%d "
L31 .FILL #1
L30 .STRINGZ "%s\n"
L25 .STRINGZ "while"
L24 .STRINGZ "else"
L23 .STRINGZ "for"
L22 .STRINGZ "if"
L17 .FILL #0
L16 .STRINGZ "\n"
L14 .STRINGZ " %d"
L15 .FILL #1
L9 .FILL #0
.END
