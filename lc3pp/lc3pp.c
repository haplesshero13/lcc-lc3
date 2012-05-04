/**************************************************************
  		lc3pp.c
Author: Ajay Ladsaria	11/22/2003
	This linker modifies the output of LCC to make it friendly for LC3's
	assembler.  It replaces the global variable flags with the actual address
	based on the variable's offset from the beginning of the global variable
	memory region (R4).  It reorders the code so that the code comes first and
	then the variable declarations.  It attempts to attach asm files with the
	same name as the externed items.

Modified: 1/05/2004
   Now supports arbitrary sizes for global variable names and source code line
   length and an arbitrary number of global variables.  Also, the linker now
   requires an argument specifing the path to the libcode directory.
	
Modified: 3/31/2004 
   Functions as a proper linker at the .asm level, and thus takes multiple
   source files as command line arguments.  It is necessary to specify an
   output file as the last command line argument and the path to the lc3lib
   code as the first command line argument.
***************************************************************************/

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>

#define LINE_MAXLEN 100

typedef struct {
   char* name;
   int size;
} variable;

/*this struct represents one .global or .extern variable/function name
 * argn = file# based on the order filenames were given in command line
 */
typedef struct {
   char* name;
   int argn;
} link_variable;

/*this struct has a list of all the .global vars and
  a list of all the .extern vars
  */
typedef struct {
   link_variable* g;   /*list of .globals and file ids*/
   int gi;             /*num of .globals*/
   int gm;            /*total num allocated for .globals*/
   link_variable* e;
   int ei;
   int em;
} link_list;

/*this struct represents a variable/label that is used by a function
  since the variable holding the label address is declared within
  the function being imported, the label flag is set
  */
typedef struct {
   char* name;
   int label;  /* 1 == yes its a local label/variable*/
} used_variable;

void Readline(FILE* file, char** buf, int* len);
int Getword(char* line, char* buf);

/*passing link by value
  replaces all .externed symbols in the input file with
  the correct imported code from the other source files provided
  in the command line
  */
void 
resolve_externs(FILE** files, link_list link, int argc)
{
   char* buf, *word;
   int buf_max = 200, word_max =200; 
   int present_flag = 0, end_flag = 0;
   int len=0, num=0, i=0, j=0, k=0;
   int mfile;

   used_variable* used_var;
   int used_num=0, used_max=100;
   used_var = (used_variable*) malloc(sizeof(used_variable) * used_max);

   buf = (char*) malloc(buf_max);
   word = (char*) malloc(word_max);

   for(k=0; k<link.gi; k++) 
      if(strcmp(link.g[k].name,"main") == 0)
         break;
   mfile = link.g[k].argn;
   rewind(files[mfile]);
   /*copy all non-externs and non-globals into output file
     stop copying when extern is seen, so the appropriate
     code can be imported into the output file
     */
   while(1) {
      Readline(files[mfile], &buf, &buf_max);
      if(strncmp(buf, ".extern", 7)==0) 
         break;
      if(strncmp(buf, ".END", 4)==0) {
         fprintf(files[argc-2],"%s\n",buf);
         free(used_var); free(buf); free(word);
         return;	/*done*/
      }
      if(strncmp(buf, ".global", 7)!=0) 
         fprintf(files[argc-2],"%s\n",buf);
   }

   for(k=0; k<link.ei; k++) {
      if(link.e[k].argn != mfile)
         continue;

      while(buf_max < strlen(link.e[k].name)) {
         buf_max = buf_max << 1;
         buf = (char*) realloc(buf, buf_max);
      }

      strcpy(buf,link.e[k].name);   
      strcpy(buf+strlen(link.e[k].name),".asm");   

      /*locates the file that has the current externed symbol*/
      for(i=0; i<link.gi; i++) {
         if(strcmp(link.g[i].name, link.e[k].name) == 0) 
            break;
      }

      if(i != link.gi) {
         if(link.g[i].argn == mfile)
            continue;
         num = link.g[i].argn;
         rewind(files[num]);

         while(word_max < strlen(link.g[i].name)+8) {
            word_max = word_max << 1;
            word = (char*) realloc(word, word_max);
         }
         strcpy(word,".global ");
         strcat(word+8, link.g[i].name); 

         while(1) {
            Readline(files[num], &buf, &buf_max);
            if(strcmp(word, buf) == 0) {
               Readline(files[num], &buf, &buf_max);
               fprintf(files[argc-2],"%s\n",buf);
               if(strncmp(buf, "LC3_GFLAG",9) == 0) {
                  /*import variable decl*/
                  while(1) {
                     Readline(files[num], &buf, &buf_max);
                     if(strncmp(buf, "LC3_GFLAG .FILL",15) == 0) {
                        fprintf(files[argc-2],"%s\n",buf);
                     } else
                        break;
                  }
               } else {
                  /*import function def and
                    any variable decl that go along

                    insert func name in used_var so it does not
                    get imported recursively
                    */
                  used_var[used_num].label = 1;
                  used_var[used_num].name =
                     (char*) malloc(strlen(link.g[i].name));
                  strcpy(used_var[used_num++].name, link.g[i].name);

                  while(1) {
                     Readline(files[num], &buf, &buf_max);
                     fprintf(files[argc-2],"%s\n",buf);
                     if(strncmp(buf, ".LC3GLOBAL",10) == 0) {
                        for(j=11; buf[j]!=' ' && buf[j]!='+';j++);
                        while(j-11 > word_max) {
                           word_max = word_max <<1;
                           word = (char*) realloc(word, word_max);
                        }
                        strncpy(word, buf+11, j-11);
                        word[j-11] = '\0';

                        if(used_num == used_max) {
                           used_max = used_max <<1;
                           used_var = (used_variable*) 
                              realloc(used_var,used_max);
                        }

                        /*dont insert in used_var unless new*/
                        present_flag = 0;
                        for(i=0; i<used_num; i++) 
                           if(used_var[i].name != NULL)
                              if(strcmp(word, used_var[i].name)==0) {
                                 present_flag = 1;
                                 break;
                              }

                        if(!present_flag) {
                           used_var[used_num].label = 0;
                           used_var[used_num].name =
                              (char*) malloc(strlen(word));
                           strcpy(used_var[used_num++].name, word);
                        }
                     } else if(strncmp(buf, "LC3_GFLAG",9) == 0) {
                        if(used_num == used_max) {
                           used_max = used_max <<1;
                           used_var =(used_variable*)realloc(used_var,used_max);
                        }

                        /*variable is actually a label*/
                        present_flag = 0;
                        strcpy(word,"LC3_GFLAG ");
                        for(i=0; i<used_num; i++) {
                           strcpy(word+10, used_var[i].name);
                           len = 10 + strlen(used_var[i].name);
                           if(strncmp(buf, word, len) == 0) {
                              used_var[i].label = 1;
                              present_flag = 1;
                              break;
                           }
                        }

                        for(j=10; buf[j]!=' ' && buf[j]!='+';j++);
                        while(j-10 > word_max) {
                           word_max = word_max <<1;
                           word = (char*) realloc(word, word_max);
                        }
                        strncpy(word, buf+10, j-10);
                        word[j-10] = '\0';
                        if(!present_flag) {
                           used_var[used_num].label = 1;
                           used_var[used_num].name =
                              (char*) malloc(strlen(word));
                           strcpy(used_var[used_num++].name, word);
                        }
                     } else if(strncmp(buf, "RET",3) == 0) {
                        break;
                     } 
                  }

                  /*if any used_vars are externed then set label and
                    put used_var in the externed var list
                    */
                  for(i=0; i<used_num; i++) {
                     for(j=0; j<link.ei; j++) {
                        if(link.e[j].argn == num && 
                              strcmp(link.e[j].name, used_var[i].name)==0) {
                           used_var[i].label = 1;
                           for(j=0; j<link.ei; j++) 
                              if(link.e[j].argn == 0 && 
                                    strcmp(link.e[j].name, used_var[i].name)==0)
                                 break;

                           if(j == link.ei) {
                              if( link.ei == link.em) {
                                 link.em = link.em << 1;
                                 link.e = realloc(link.e, 
                                       sizeof(link_variable)*link.em);
                              }
                              link.e[link.ei].name =(char*) 
                                 malloc(strlen(used_var[i].name));
                              strcpy(link.e[link.ei].name, used_var[i].name);
                              link.e[link.ei++].argn = mfile;
                           }
                           break;
                        }
                     }
                  }

                  /*import var decl used by function*/
                  strcpy(word,"LC3_GFLAG ");
                  Readline(files[num], &buf, &buf_max);

                  for(i=0; i<used_num; i++) {
                     if(used_var[i].label == 1)
                        continue;

                     strcpy(word+10, used_var[i].name);
                     len = 10 + strlen(used_var[i].name);
                     end_flag = 0;

                     while(1) {
                        if(strncmp(word, buf, len) == 0) {
                           fprintf(files[argc-2],"%s\n",buf);
                           while(1) {
                              Readline(files[num], &buf, &buf_max);
                              if(strncmp(buf,"LC3_GFLAG .FILL",15) == 0) {
                                 fprintf(files[argc-2],"%s\n",buf);
                              } else
                                 break;
                           }
                           break;
                        } else if(strncmp(buf, ".END", 4) == 0) {
                           rewind(files[num]);
                           Readline(files[num], &buf, &buf_max);
                           /*this is to prevent infinitely looking
                             for a var thats not there
                             */
                           if(end_flag) {
                              printf("symbol %s not found\n",word);
                              break;
                           } else
                              end_flag = 1;
                        } else
                           Readline(files[num], &buf, &buf_max);
                     }
                  }
                  /*free memory for used_var for next externed func*/
                  for(i=0; i<used_num; i++) 
                     free(used_var[i].name);
                  used_num = 0;
               }
            }
            if(strncmp(buf, ".END", 4)==0)
               break;	/*done*/
         }
      } 
      else if( (files[argc-1] = fopen(buf, "r")) == NULL)
         printf("error reading file %s\n",buf);
      else {
         while(1) {
            buf[0] = fgetc(files[argc-1]);
            if(feof(files[argc-1]))
               break;
            fputc(buf[0],files[argc-2]);
         }
         fclose(files[argc-1]);
      }
   } 

   /*copy rest of the input file into output file
     ignoring all .externs and .globals
     */
   while(1) {
      Readline(files[mfile], &buf, &buf_max);

      if(strncmp(buf, ".END", 4)==0) {
         fprintf(files[argc-2],"%s\n",buf);
         break;	/*done*/
      }

      if(strncmp(buf, ".global", 7)!=0 && strncmp(buf, ".extern", 7)!=0) 
         fprintf(files[argc-2],"%s\n",buf);
   }

   free(used_var); free(word); free(buf);
   return;
}

int 
main (int argc, char *argv[]) 
{
   FILE *output_file, *code_file, *var_file, *full_file;
   int i=0;
   int var_name_maxlen = LINE_MAXLEN;
   int cwd_size=200;
   int length=0;

   /*used when replacing .LC3Global lines*/
   int reg_num=0, offset=0;

   /*This array will hold FILE* to input source files*/
   FILE** files;

   /*This will hold all the .global and .extern symbols*/
   link_list link;

   /*initialize buffers to read lines into*/
   int curr_line_maxlen = LINE_MAXLEN;
   int tmp_line_maxlen = LINE_MAXLEN;
   char *curr_line, *tmp_line, *var_name;
   char* cwd = malloc(cwd_size);

   /*set up mini-symbol table*/
   int g_num = -1;
   variable* g_vars; int g_total = 100;

   curr_line = malloc(LINE_MAXLEN);
   tmp_line = malloc(LINE_MAXLEN);
   var_name = malloc(LINE_MAXLEN); 

   g_vars = malloc(sizeof(variable)*g_total);

   files = (FILE**) malloc(sizeof(FILE*) * (argc+1));

   link.gi = link.ei = 0;
   link.gm = link.em = 100;
   link.g =(link_variable*) malloc(sizeof(link_variable) * link.gm);
   link.e =(link_variable*) malloc(sizeof(link_variable) * link.em);

   if( (output_file = fopen(argv[argc-1], "w")) == NULL)
      printf("error opening file %s\n",argv[argc-1]);

   /*opening all input files read only
     assuming first 2 args are lc3pp and libpath*/
   argc--;
   for(i=0; i<argc-2; i++) {
      files[i] = fopen(argv[i+2], "r");
      if( files[i] == NULL) { 
         printf("Error opening file %s\n", argv[i+2]);
         return -1;
      }
   }

   /*get current working dir (cwd)*/
   while(!getcwd(cwd,cwd_size)) {
      cwd_size = cwd_size<<1;
      cwd = realloc(cwd, cwd_size);
   }

   /*argv[1] == libpath*/
   chdir(argv[1]);
   files[argc-2] = fopen("stdio.asm", "r");
   if(files[argc-2] == NULL)
      printf("Error opening file %sstdio.asm\n", argv[1]);
   else
      argc++;
   chdir(cwd);

   /*scan each file for all .global declarations and record them*/
   /*scan each file for all .extern declarations and record them*/
   for(i=0; i<argc-2; i++) {
      while(1) {
         Readline(files[i], &tmp_line, &tmp_line_maxlen);
         if(strncmp(tmp_line, ".global", 7)==0) {
            if(link.gi == link.gm) {
               link.gm = link.gm << 1;
               link.g = realloc(link.g, sizeof(link_variable)*link.gm);
            }
            link.g[link.gi].name =(char*) malloc(strlen(tmp_line+8)+1);
            strcpy(link.g[link.gi].name, tmp_line+8);
            link.g[link.gi++].argn = i;
         }
         else if(strncmp(tmp_line, ".extern", 7)==0) {
            if(link.ei == link.em) {
               link.em = link.em << 1;
               link.e = realloc(link.e, sizeof(link_variable)*link.em);
            }
            link.e[link.ei].name =(char*) malloc(strlen(tmp_line+8)+1);
            strcpy(link.e[link.ei].name, tmp_line+8);
            link.e[link.ei++].argn = i;
         } else if(strncmp(tmp_line, ".END", 4)==0)
            break;	/*file is done*/
      }
   }

   if(argc < 3) {
      printf("Missing Arguments: lc3pp libpath source [sources]\n");
      return -1;
   }

   /*open three tmp files with unique file names to use as tmp buffers*/
   var_file = tmpfile();
   code_file = tmpfile();
   files[argc-2] = tmpfile();
   full_file = files[argc-2];

   /*argv[1] == libpath*/
   chdir(argv[1]);

   /* Imports externed functions and variables 
      Eliminates .global declarations
      returns FILE* to complete file in files[argc-2], also in full_file
      */
   resolve_externs(files, link, argc);

   chdir(cwd);

   /*closing all input files read only*/
   /*assuming first 2 args are lc3pp and libpath*/
   for(i=0; i<argc-2; i++) {
      fclose(files[i]);
   }
   /*clearing memory for file array*/
   free(files);

   rewind(full_file);

   /* Input file's variables and code are separated into
    * into separate files for processing and then later merging 
    * All LC3_GFLAGS are stripped and a global variable table is made
    */ 
   while(1) {
      /*Read one line*/
      Readline(full_file, &curr_line, &curr_line_maxlen);

      if(strncmp(curr_line, "LC3_GFLAG", 9)==0) {
         /*global var declaration on this line*/
         if(strncmp(curr_line+10, ".FILL", 5)==0) {
            fprintf(var_file,"%s\n",curr_line+10);
            g_vars[g_num].size++;
         } else if(strncmp(curr_line+10, ".BLKW", 5)==0) {
            fprintf(var_file,"%s\n",curr_line+10);
            g_vars[g_num].size+=atoi(curr_line+16);
         } else if(strncmp(curr_line+10, ".STRINGZ", 8)==0) {
            length = Getword(tmp_line, curr_line+19);
            fprintf(var_file,".STRINGZ %s\n",curr_line+19+length);
            g_vars[g_num].size+=atoi(curr_line+19);
         } else {
            length = Getword(tmp_line, curr_line+10);
            if(g_num > g_total - 5) {
               g_vars = realloc(g_vars, sizeof(variable)*2*g_total);
               g_total = g_total << 1;
            }
            g_num++;

            g_vars[g_num].name = (char *) malloc(strlen(tmp_line)+1);

            strcpy(g_vars[g_num].name, tmp_line);
            if(strncmp(curr_line+length+21, ".FILL", 5)==0) {
               g_vars[g_num].size = 1;
               fprintf(var_file,"%s %s\n",tmp_line, curr_line+length+21);
            } else if(strncmp(curr_line+length+21, ".BLKW", 5)==0) {
               g_vars[g_num].size = atoi(curr_line+length+26);
               fprintf(var_file,"%s %s\n",tmp_line, curr_line+length+21);
            } else {
               fprintf(var_file,"%s ",tmp_line);
               length += Getword(tmp_line, curr_line+length+30);
               g_vars[g_num].size = atoi(tmp_line);
               fprintf(var_file,".STRINGZ %s\n", curr_line+length+31);
            }
         }
      } else {
         /*No LC-3 generated global var declaration on this line
           just write to codefile
           .end will end up in codefile
           */
         if(strncmp(curr_line, ".extern", 7)!=0) {
            fprintf(code_file,"%s\n",curr_line);
         }
         if(strncmp(curr_line, ".END", 4)==0) {
            fprintf(var_file,"%s\n",curr_line);
            break;
         }
      }
   }
   rewind(var_file);
   rewind(code_file);

   /*name the output file the same as the first input source
     except with .asm suffix
     */
   /*
      for(i=0; i<link.gi; i++) 
      if(strcmp(link.g[i].name,"main") == 0)
      break;
      strcpy(tmp_line, argv[2+link.g[i].argn]);
      strcpy(tmp_line + strlen(argv[2+link.g[i].argn]) - 2, ".asm");
      if( (output_file = fopen(tmp_line, "w")) == NULL)
      printf("error opening file \n");
      */

   /* Replace each .LC3GLOBAL varname regnum line with the LC-3
    * assembly to load the value of the variable into the register
    */
   while(1) {
      Readline(code_file, &curr_line, &curr_line_maxlen);
      if(strncmp(curr_line, ".END", 4)==0)
         break;	

      if(strncmp(curr_line, ".LC3GLOBAL", 10)!=0)
         fprintf(output_file, "%s\n", curr_line);
      else
      {
         /*Need to replace the global var flag*/
         i=0;
         if(var_name_maxlen < curr_line_maxlen)
         {
            var_name_maxlen = curr_line_maxlen;
            var_name = realloc( var_name, curr_line_maxlen);
         }
         while( (var_name[i++] = curr_line[i+11]) != ' ');

         var_name[i-1] = '\0';
         reg_num = atoi(curr_line+i+11);

         strcpy(var_name, curr_line+11);
         i = 0;
         offset=0;
         while (var_name[++i] != '+' && var_name[i] != '-' 
               && var_name[i] != ' ');
         if(var_name[i] == '+' || var_name[i] == '-') {
            offset += atoi(var_name+i);
         }
         var_name[i] = '\0';

         for(i=0; i<g_num; i++) {
            if(strcmp(g_vars[i].name,var_name)==0)
               break;
            offset+=g_vars[i].size;
         }

         i=offset;
         if(i<0) {
            if(i > -16)
               fprintf(output_file, "ADD R%d, R4, #%d\n",reg_num,i);
            else {
               fprintf(output_file, "ADD R%d, R4, #-16\n",reg_num);
               for(i+=16; i<-16; i+=16)
                  fprintf(output_file, "ADD R%d, R%d, #-16\n",reg_num,reg_num);
               fprintf(output_file, "ADD R%d, R%d, #%d\n",reg_num,reg_num,i);
            }
         }
         else {
            if(i < 15)
               fprintf(output_file, "ADD R%d, R4, #%d\n",reg_num,i);
            else {
               fprintf(output_file, "ADD R%d, R4, #15\n",reg_num);
               for(i-=15;i>15; i-=15)
                  fprintf(output_file, "ADD R%d, R%d, #15\n",reg_num,reg_num);
               fprintf(output_file, "ADD R%d, R%d, #%d\n",reg_num,reg_num,i);
            }
         }
      } 
   }

   fprintf(output_file,"\nGLOBAL_DATA_START\n");

   /* Copy the variable declarations into the end of the output file */
   while(1) {
      Readline(var_file, &tmp_line, &tmp_line_maxlen);
      fprintf(output_file,"%s\n",tmp_line);
      if(strncmp(tmp_line, ".END", 4)==0)
         break;	/*done*/
   }

   /*close file descriptors and free dynamic memory*/
   fclose(full_file); fclose(code_file);
   fclose(var_file); fclose(output_file);

   free(curr_line); free(tmp_line); free(var_name);
   free(cwd);
   for(i=0; i<=g_num; i++) 
      free(g_vars[i].name);
   free(g_vars);
   for(i=0; i<link.gi; i++)
      free(link.g[i].name);
   for(i=0; i<link.ei; i++)
      free(link.e[i].name);
   free(link.g); free(link.e);
   return 0;
}


/*********************************************************************
 * Readline
 *  inputs: file descriptor, ptr to char ptr, length of this buf
 *  Reads one line from the current place in the File, if the buffer is 
 *  not large enough, then realloc is used to double its size
 *  *****************************************************************/

void 
Readline(FILE* file, char** buf, int* len) 
{
   int i=0;
   for(i=0; !feof(file); i++) {
      if( i > *len - 2) {
         *buf = (char*) realloc((void*) *buf, (*len) * 2);
         *len = (*len) * 2;
      }

      if( ((*buf)[i] = fgetc(file)) == '\n')
         break;
   }
   (*buf)[i] = '\0';
}

/*********************************************************************
 * Getword
 *  inputs: destination buffer, and source buffer
 *  Reads one word from 'line' and copies it into 'buf'
 *  *****************************************************************/
int 
Getword(char* buf, char* line) 
{
   int i=0;
   while(line[i++] != '\0' && line[i] != ' ');
   strncpy(buf, line, i);
   buf[i] = '\0';
   return i;
}

