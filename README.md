-------------------------------
   DESCRIPTION OF CONTENTS
-------------------------------

This is the preliminary distribution of LCC that supports the LC-3.  This
is my first time the auto configuration process, so if it doesn't work for
you, please email me and I will try to see what I can do.  The copyright
information is in the file CPYRIGHT.  There is absolutely no warranty for
this software.  The installation information is in INSTALL.  TODO contains
a to-do list.

-------------------------------
   INSTALLATION INSTRUCTIONS
-------------------------------
Untar the package with a command like 'tar -xzf lcc-1.1.tar.gz.'

From the top-level directory type 'configure'

Now install the lcc/lc3 binaries by typing 'make install'

If everything goes well, the make process will create a directory off the
top level called "install".  It should contain the various binaries needed
by the compiler, along with some informational files, such as this README
file, a sample C file to compile, and a makefile for compiling that file.

-------------------------------
   HOW TO USE
-------------------------------

In the package there is a regressions and limitations test directiory in
topdir/test/limitations and topdir/test/regression.  The regression
directory has a small set of files that have been tested to compile
properly and have been assembled and simulated using Steve Lumetta's LC-3
simulator.  Many of these examples are taken from the book by Patt and Patel.

Compiling programs using the compiler is similar to using a standard C
compiler.  Behind the scenes, the compiler will (1) compile the .c files
into a set of pseudo-assembly .lcc files, (2) use the lc3pp (lc3 post
processor) to link and massage the .lcc files and library files into a
single .asm file, and (3) use Steve Lumetta's LC-3 assembler to assemble
the .asm into a .obj file.  This object file can then be loaded The file
topdir/test/regressions/Makefile provides an example of how the
compilation process works.

Note: not all C programs will compile to the LC-3.  Programs with floating
point types, for example will not currently compile.  Also, certain
complex integer expressions will not generate properly because of the
limited LC-3 register set.  See topdir/test/limitations for some examples
that do not compile properly.  I hope to reduce this set of exceptional
cases over time.

Ajay Ladsaria
Sanjay J. Patel (sjp@crhc.uiuc.edu)
