LCC-LC3 C Compiler
===================

##Recent Changes
####May 2012

*Avery Yen*

Modified Makefile.def and configure to automatically install to `~/.lc3`. Also, `make install` now only installs executables. This is because having documents like a Makefile and a README in your executable path is a Bad Idea. The Makefile, README, and other documents previously installed by `make install` already live in the source directory. There doesn't seem to be any reason to make them live in two places.

*Sean Smith (Dartmouth College), Stephen Canon*

Modified configure to compile on Mac OS X, especially Lion, but should be compatible with Leopard/Snow Leopard.

On Mac OS X Lion using Xcode 4.3, compiling with `/usr/bin/gcc` produces a binary lcc that crashes every time it is called. The fix is to use the `-mno-sse` flag when compiling `rcc`. Sean Smith reports that the issue appears to be with the MOVAPS instruction that gets compiled by `/usr/bin/gcc`, using the latest llvm-gcc compiler that comes with Xcode 4.3 on OS X Lion, which expects to copy aligned memory, but for some reason doesn't.

From Stephen Canon:

> `/usr/bin/gcc` is a bit of a frankencompiler on Lion; it's actually an alias for `llvm-gcc-4.2`, which uses the front end of GCC-4.2 and the LLVM back end. I suspect that the GCC frontend is making an alignment assumption in its internal representation that would later get unwound by the GCC backend, but that never has a chance to happen...


##Description Of Contents

This is the preliminary distribution of LCC that supports the LC-3. This
is the slightly modified version of Ladsaria and Patel's LCC for use at Dartmouth College. The copyright information is in the file CPYRIGHT. There is absolutely no warranty for this software. Complete installation information is in INSTALL. TODO contains a to-do list.

##Installation Instructions

First, you must install the [LC3 Tools](https://github.com/haplesshero13/lc3tools) to have `lc3as`.

Download and unpack this source, either with `git clone git://github.com/haplesshero13/lc3tools.git` or by clicking on the ZIP download button at the top of this github page.

Change to the lcc directory with `cd lcc-1.3`

From the top-level directory type `configure`

Now install the lcc/lc3 binaries by typing `make install`

If everything goes well, the make process will create a directory off the
top level called "install". It should contain the various binaries needed
by the compiler, along with some informational files, such as this README
file, a sample C file to compile, and a makefile for compiling that file.


##How To Use

In the package there is a regressions and limitations test directiory in
topdir/test/limitations and topdir/test/regression. The regression
directory has a small set of files that have been tested to compile
properly and have been assembled and simulated using Steve Lumetta's LC-3
simulator. Many of these examples are taken from the book by Patt and Patel.

Compiling programs using the compiler is similar to using a standard C
compiler. Behind the scenes, the compiler will (1) compile the .c files
into a set of pseudo-assembly .lcc files, (2) use the lc3pp (lc3 post
processor) to link and massage the .lcc files and library files into a
single .asm file, and (3) use Steve Lumetta's LC-3 assembler to assemble
the .asm into a .obj file. This object file can then be loaded The file
topdir/test/regressions/Makefile provides an example of how the
compilation process works.

Note: not all C programs will compile to the LC-3. Programs with floating
point types, for example will not currently compile. Also, certain
complex integer expressions will not generate properly because of the
limited LC-3 register set. See topdir/test/limitations for some examples
that do not compile properly. I hope to reduce this set of exceptional
cases over time.

##Maintainers and Contributors
* Sean Smith
* Stephen Canon
* Avery Yen

##Original Authors

* Ajay Ladsaria
* Sanjay J. Patel (sjp@crhc.uiuc.edu)
