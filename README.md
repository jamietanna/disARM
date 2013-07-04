disARM.c
========
 
Note that the code has not been refactored since submission, January 2013. I will work on a refactored version soon.

To be compiled with `gcc disarm.c -o disarm -std=c99 -Wall -pedantic`.

General Background 
------------------

Author: Jamie Tanna
Email:  jamie@jamietanna.co.uk

DisARM takes in an Executable-and-Linked (ELF) file, disassembles it, and then outputs the corresponding ARM code.
Project from the G51PRG course at the University of Nottingham. Specification can be found at [http://g51prg.cs.nott.ac.uk/Distribution/Coursework/cswk5.pdf](http://g51prg.cs.nott.ac.uk/Distribution/Coursework/cswk5.pdf) or at `UoN-G51PRG-disARM.pdf`.

When working out how to perform all the disassembly, I referenced the documents below, as well as the output of `readelf -a`, `objdump -D` and `hexdump -C`.

The program can deal with the following types of instructions:
 - SWI
 - Branch
 - Multiple Load/Store
 - Normal Load/Store
 - Multiply
 - ADR and ADRL
 - Data Processing
 - A couple of other pseudo-instructions
 - Shifts and rotates

The program's extra features were influenced by the programs readelf and objdump. Output of the section headers and symbol table was done mainly for debugging, but then it remained so that users who cared would be able to see these tables.

The program is best run using a screen width of at least 160 columns, in order to display all the text. However, 200+ columns is also recommended.

Extra features
--------------

 - #### ASCII output
   By using the -a flag, an ASCII dump of each byte, decoded with accordance to the endian-encoding of the ELF file, will be added to each line. This helps work out where sections of data are. This was influenced by `hexdump -C`.

 - #### Colour coded instructions
   Output of colour coded instructions is done through ANSI escape codes. The following colours should be displayed:
     - Purple        - Pseudocode
     - Bright Purple - ASCII decoded
     - Bright Red    - Errors
     - Yellow        - Condition Code
     - Bright Blue   - Labels
     - Cyan          - Status Bit
     - Bright Yellow - Table Headers, Filename
     - Light Grey    - Comment, Banners
     - Bright Green  - Top Banners

 - #### Labels
   Something I was extremely proud of is being able to decode labels and other strings from the program. Steve originally said this was impossible, and so I took a lot of time to work it out. It gives the program disassembly a better feel, especially as it now looks almost exactly as the pre-assembled source code.
   Something I was extremely proud of is being able to decode labels and other strings from the program. Steve originally said this was impossible, and so I took a lot of time to work it out. It gives the program disassembly a better feel, especially as it now looks almost exactly as the pre-assembled source code.

 - #### ADRL Decoding
   By checking ahead instructions, as well as the current one, we can determine whether an ADRL is present. NOTE: This code often fails when two consecutive functions use the same register. i.e. `ADRL R0, cval`  followed by `ADD R0, R1, R5, LSL #3`. Although very unlikely, this is a problem the code cannot understand well. If code does this, please use a NOP after the ADRL. The program will display an error if this happens, and request the user to remove the ADRL flag.

 - #### Comments
   Comments are provided to show what numbers in Operand 2 are converted between decimal and hexadecimal, which is useful if specified in hexadecimal when coding, but it is outputted in decimal by default. A full breakdown of the ADRL/ADR is displayed to users, so that if they have it enabled, they are able to see which instructions make it up. Branches and ADR(L)s also show the memory address of any labels that are pointed to.

 - #### Section Headers and Symbol Table
   These were first seen in `readelf -a` and `objdump -D`, which also put me on the track of decoding labels. I decided that if I was going to use them in my program, I may as well output them as well. It indicates whether data is a compiler constant or an actual block of memory.

### Additional features to implement:
 - #### File output

   The ability to specify a file to write to, which will then include an ORIGIN 0xMEM at each new section header, to give the user very likely source. The only problem is not having data decoded, due to Von Neumann architecture. Guessing from any ASCII text found in the memory location may work, but would not necessarily be accurate. This is something that would still require some work and thought, and most probably user input.
 - (There were others but I generally covered them all as I was going)


References
----------
 - The SCO Developer Network Application Binary Interface
    - [http://sco.com/developers/gabi/](http://sco.com/developers/gabi/)
    - [http://sco.com/developers/gabi/1998-04-29/contents.html](http://sco.com/developers/gabi/1998-04-29/contents.html)

 - G51CSA ARM Instruction Set Summary
    - [http://www.cs.nott.ac.uk/~dfb/G51CSA/resources_files/ARMInst-3.pdf](http://www.cs.nott.ac.uk/~dfb/G51CSA/resources_files/ARMInst-3.pdf)

 - ARM Instruction Layouts. Conditions and Op-codes
    - [http://www.cs.nott.ac.uk/~dfb/G51CSA/resources_files/layouts-3.pdf](http://www.cs.nott.ac.uk/~dfb/G51CSA/resources_files/layouts-3.pdf)

 - Comprehensive data sheets for ARM chip
    - [http://www.cs.nott.ac.uk/~dfb/G51CSA/resources_files/ARM_doc.pdf](http://www.cs.nott.ac.uk/~dfb/G51CSA/resources_files/ARM_doc.pdf)

 - The `man` entries of:
    - readelf
    - objdump
    - hexdump


Program Execution and Source
----------------------------

 The order that functions are listed is the order in which the program calls the functions, give or take a little inaccuracy.

 The program was initially indented manually, but in order to ensure a consistent style, the following command was used `indent disarm.c elf.h -kr -bad -bap -bbb -nut -di2 -brs -lp -ce -saf -cdw -cli4`.

 Program return values, and their meanings:
 
  0 :    Success

  1 :    Invalid arguments

  2 :    Cannot open file

  3 :    Invalid file format i.e. not ELF

  4 :    Run out of memory

Code Feedback
-------------
 Main feedback was to remove preprocessor macros. However, in order to ensure this program is more readable for those who have not got full knowledge of ARM, I have left them in.

Creating ELF Files
==================

In order to create ELF Files, other than those in the ElfFiles, you need to run `/path/to/kmd/aasm -lk filename.kmd -e filename.elf filename.s` where `kmd` is the Komodo ARM Debugger. 