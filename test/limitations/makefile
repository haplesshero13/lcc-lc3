# Author: Ajay Ladsaria
# Example Makefile to compile a C source file into LC-3 object code

# Name of C source file
CODE = fig18.1
# Directory with rcc, gcc/cpp, gcc/include, other gcc files
# Should be changed to an absolute path
LCCDIR = ../../src
# Directory with LC-3 library routines
# Should be changed to an absolute path
LC3LIB = ../../lc3lib

all: $(CODE).obj

remake: clean all

# Assemble LC-3 .asm to LC-3 .obj
# lc3as is distributed in a different package by Steven Lumetta
$(CODE).obj: $(CODE).asm
	lc3as $(CODE).asm

# Postprocessor (lc3pp) makes a couple passes on the LC-3 .asm outputted
# 	by LCC
$(CODE).asm: $(CODE).lcc
	../../lc3pp/lc3pp $(LC3LIB) $(CODE).lcc $(CODE).asm
#lc3pp $(LC3LIB) $(CODE).lcc $(CODE).asm

# C compiler (lcc) 
# Options:	-S stops after making the .asm
# 			-Wo-lccdir=		Tells it where to look for rcc, gcc stuff
# 			-Wf-target=lc3	Tells it to make output for lc3 backend, the 
# 								other targets will not work correctly because
# 								some target-independent code has been modified.
# 			-I.				Tells it to look for header files first in the
# 								current directory
$(CODE).lcc: $(CODE).c 
	../../etc/lcc -S -Wo-lccdir=$(LCCDIR) -Wf-target=lc3 -I. $(CODE).c -o $(CODE).lcc
#lcc -S -Wo-lccdir=$(LCCDIR) -Wf-target=lc3 -I. $(CODE).c -o $(CODE).lcc

$(CODE).c:

clean:
	rm $(CODE).lcc $(CODE).asm $(CODE).obj $(CODE).sym

