/**
 * disARM.c
 * ========
 *
 * Note that the code has not been refactored since submission, January 2013. I will work on a refactored version soon.
 *
 * To be compiled with `gcc disarm.c -o disarm -std=c99 -Wall -pedantic`.
 *
 * General Background
 * ------------------
 *
 * Author: Jamie Tanna
 * Email:  jamie@jamietanna.co.uk
 *
 * DisARM takes in an Executable-and-Linked (ELF) file, disassembles it, and then outputs the corresponding ARM code.
 * Project from the G51PRG course at the University of Nottingham. Specification can be found at [http://g51prg.cs.nott.ac.uk/Distribution/Coursework/cswk5.pdf](http://g51prg.cs.nott.ac.uk/Distribution/Coursework/cswk5.pdf) or at `UoN-G51PRG-disARM.pdf`.
 *
 * When working out how to perform all the disassembly, I referenced the documents below, as well as the output of `readelf -a`, `objdump -D` and `hexdump -C`.
 *
 * The program can deal with the following types of instructions:
 *  - SWI
 *  - Branch
 *  - Multiple Load/Store
 *  - Normal Load/Store
 *  - Multiply
 *  - ADR and ADRL
 *  - Data Processing
 *  - A couple of other pseudo-instructions
 *  - Shifts and rotates
 *
 * The program's extra features were influenced by the programs readelf and objdump. Output of the section headers and symbol table was done mainly for debugging, but then it remained so that users who cared would be able to see these tables.
 *
 * The program is best run using a screen width of at least 160 columns, in order to display all the text. However, 200+ columns is also recommended.
 *
 * Extra features
 * --------------
 *
 *  - #### ASCII output
 *    By using the -a flag, an ASCII dump of each byte, decoded with accordance to the endian-encoding of the ELF file, will be added to each line. This helps work out where sections of data are. This was influenced by `hexdump -C`.
 *
 *  - #### Colour coded instructions
 *    Output of colour coded instructions is done through ANSI escape codes. The following colours should be displayed:
 *      - Purple        - Pseudocode
 *      - Bright Purple - ASCII decoded
 *      - Bright Red    - Errors
 *      - Yellow        - Condition Code
 *      - Bright Blue   - Labels
 *      - Cyan          - Status Bit
 *      - Bright Yellow - Table Headers, Filename
 *      - Light Grey    - Comment, Banners
 *      - Bright Green  - Top Banners
 *
 *  - #### Labels
 *    Something I was extremely proud of is being able to decode labels and other strings from the program. Steve originally said this was impossible, and so I took a lot of time to work it out. It gives the program disassembly a better feel, especially as it now looks almost exactly as the pre-assembled source code.
 *    Something I was extremely proud of is being able to decode labels and other strings from the program. Steve originally said this was impossible, and so I took a lot of time to work it out. It gives the program disassembly a better feel, especially as it now looks almost exactly as the pre-assembled source code.
 *
 *  - #### ADRL Decoding
 *    By checking ahead instructions, as well as the current one, we can determine whether an ADRL is present. NOTE: This code often fails when two consecutive functions use the same register. i.e. `ADRL R0, cval`  followed by `ADD R0, R1, R5, LSL #3`. Although very unlikely, this is a problem the code cannot understand well. If code does this, please use a NOP after the ADRL. The program will display an error if this happens, and request the user to remove the ADRL flag.
 *
 *  - #### Comments
 *    Comments are provided to show what numbers in Operand 2 are converted between decimal and hexadecimal, which is useful if specified in hexadecimal when coding, but it is outputted in decimal by default. A full breakdown of the ADRL/ADR is displayed to users, so that if they have it enabled, they are able to see which instructions make it up. Branches and ADR(L)s also show the memory address of any labels that are pointed to.
 *
 *  - #### Section Headers and Symbol Table
 *    These were first seen in `readelf -a` and `objdump -D`, which also put me on the track of decoding labels. I decided that if I was going to use them in my program, I may as well output them as well. It indicates whether data is a compiler constant or an actual block of memory.
 *
 * ### Additional features to implement:
 *  - #### File output
 *
 *    The ability to specify a file to write to, which will then include an ORIGIN 0xMEM at each new section header, to give the user very likely source. The only problem is not having data decoded, due to Von Neumann architecture. Guessing from any ASCII text found in the memory location may work, but would not necessarily be accurate. This is something that would still require some work and thought, and most probably user input.
 *  - (There were others but I generally covered them all as I was going)
 *
 *
 * References
 * ----------
 *  - The SCO Developer Network Application Binary Interface
 *     - [http://sco.com/developers/gabi/](http://sco.com/developers/gabi/)
 *     - [http://sco.com/developers/gabi/1998-04-29/contents.html](http://sco.com/developers/gabi/1998-04-29/contents.html)
 *
 *  - G51CSA ARM Instruction Set Summary
 *     - [http://www.cs.nott.ac.uk/~dfb/G51CSA/resources_files/ARMInst-3.pdf](http://www.cs.nott.ac.uk/~dfb/G51CSA/resources_files/ARMInst-3.pdf)
 *
 *  - ARM Instruction Layouts. Conditions and Op-codes
 *     - [http://www.cs.nott.ac.uk/~dfb/G51CSA/resources_files/layouts-3.pdf](http://www.cs.nott.ac.uk/~dfb/G51CSA/resources_files/layouts-3.pdf)
 *
 *  - Comprehensive data sheets for ARM chip
 *     - [http://www.cs.nott.ac.uk/~dfb/G51CSA/resources_files/ARM_doc.pdf](http://www.cs.nott.ac.uk/~dfb/G51CSA/resources_files/ARM_doc.pdf)
 *
 *  - The `man` entries of:
 *     - readelf
 *     - objdump
 *     - hexdump
 *
 *
 * Program Execution and Source
 * ----------------------------
 *
 *  The order that functions are listed is the order in which the program calls the functions, give or take a little inaccuracy.
 *
 *  The program was initially indented manually, but in order to ensure a consistent style, the following command was used `indent disarm.c elf.h -kr -bad -bap -bbb -nut -di2 -brs -lp -ce -saf -cdw -cli4`.
 *
 *  Program return values, and their meanings:
 *
 *   0 :    Success
 *
 *   1 :    Invalid arguments
 *
 *   2 :    Cannot open file
 *
 *   3 :    Invalid file format i.e. not ELF
 *
 *   4 :    Run out of memory
 *
 * Code Feedback
 * -------------
 *  Main feedback was to remove preprocessor macros. However, in order to ensure this program is more readable for those who have not got full knowledge of ARM, I have left them in.
 *
 */

#include <stdio.h>              // standard includes
#include <string.h>             // strcat, sprintf etc
#include <stdlib.h>             // malloc/realloc
#include <ctype.h>              // isprint etc
#include <unistd.h>             // arguments
#include <getopt.h>             // arguments

#include "elf.h"                // structures

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
//                       Constant and Preprocessor Macro Declarations
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */

    // By using preprocessor macros, we can easily get the result of a bitmask/bitshift by calling i.e. BITMASK_SWI(instruction), which helps with readability of code greatly, compared with having each piece of code below embedded in the source. This also will aid changing bitmasks/shifts, as many are used multiple times, and therefore storing a main copy is less data-redundant. 


    // Major bitmasks - instruction type
#define     IS_SWI(i)                   (((i & 0x0F000000) >> 24) == 15)        // 1111
#define     IS_MULT(i)                  (((i & 0x0FC00000) >> 22) ==  0)        // 000000
#define     IS_MULT_2(i)                (((i & 0x00000090) >>  4) ==  9)        // 1001
#define     IS_BR(i)                    (((i & 0x0E000000) >> 25) ==  5)        // 101
#define     IS_LDST(i)                  (((i & 0x0C000000) >> 26) ==  1)        // 01
#define     IS_DP(i)                    (((i & 0x0C000000) >> 26) ==  0)        // 00
#define     IS_MLDST(i)                 (((i & 0x0E000000) >> 25) ==  4)        // 100

        // Condition code bitmask
#define     BITMASK_CONDCODE(i)         ((i & 0xF0000000) >> 28)

        // SWI-specific bitmask
#define     IS_SWI_IMMED(i)             ( BITMASK_SWI_IMMED(i) <= 4 )
#define     BITMASK_SWI_IMMED(i)        ((i & 0x00FFFFFF) >>  0)

        // Load/Store-specific bitmask
#define     BITMASK_LDST_RN(i)          ((i & 0x000F0000) >> 16)
#define     BITMASK_LDST_RD(i)          ((i & 0x0000F000) >> 12)
#define     BITMASK_LDST_OP2(i)         ((i & 0x00000FFF) >>  0)

#define     BITMASK_LDST_B25(i)         ((i & 0x02000000) >> 25)
#define     BITMASK_LDST_PREPOST(i)     ((i & 0x01000000) >> 24)
#define     BITMASK_LDST_UPDOWN(i)      ((i & 0x00800000) >> 24)
#define     BITMASK_LDST_WORDBYTE(i)    ((i & 0x00400000) >> 22)
#define     BITMASK_LDST_WRITEBACK(i)   ((i & 0x00200000) >> 21)
#define     BITMASK_LDST_LOADSTORE(i)   ((i & 0x00100000) >> 20)

#define     BITMASK_LDST_B4(i)          ((i & 0x00000008) >>  4)
#define     BITMASK_LDST_IMMED(i)       ((i & 0x00000FFF) >>  0)
#define     BITMASK_LDST_SH_IMME(i)     ((i & 0x00000F80) >>  7)
#define     BITMASK_LDST_SH_TYPE(i)     ((i & 0x00000060) >>  5)
#define     BITMASK_LDST_SH_RM(i)       ((i & 0x0000000F) >>  0)

        // Multiple Load/Store-specific bitmask
        // NOTE: Some bitmasks used for registers are stored in the array multLoadStoreBitmasks, for easier indexing

#define     BITMASK_MLDST_PREPOST(i)    ((i & 0x01000000) >> 24)
#define     BITMASK_MLDST_UPDOWN(i)     ((i & 0x00800000) >> 23)
#define     BITMASK_MLDST_WRITEBACK(i)  ((i & 0x00200000) >> 21)
#define     BITMASK_MLDST_LOADSTORE(i)  ((i & 0x00100000) >> 20)

#define     BITMASK_MLDST_RN(i)         ((i & 0x000F0000) >> 16)

#define     BITMASK_MLDST_SET(i, a, x)  (i & a[x])   >>  x    == 1
#define     BITMASK_MLDST_SETN(i, a, x) (i & a[x+1]) >> (x+1) == 1


        // Multiply-specific bitmask

#define     BITMASK_MULT_ACCUM(i)       ((i & 0x00200000) >> 21)
#define     BITMASK_MULT_STATUS(i)      ((i & 0x00100000) >> 20)
#define     BITMASK_MULT_RD(i)          ((i & 0x000F0000) >> 16)
#define     BITMASK_MULT_RN(i)          ((i & 0x0000F000) >> 12)
#define     BITMASK_MULT_RS(i)          ((i & 0x00000F00) >>  8)
#define     BITMASK_MULT_RM(i)          ((i & 0x0000000F) >>  0)

        // Data Processing-specific bitmask

#define     BITMASK_DP_OPCODE(i)        ((i & 0x01E00000) >> 21)

#define     BITMASK_DP_RN(i)            ((i & 0x000F0000) >> 16)
#define     BITMASK_DP_RD(i)            ((i & 0x0000F000) >> 12)
#define     BITMASK_DP_OP2(i)           ((i & 0x00000FFF) >>  0)
#define     BITMASK_DP_B20(i)           ((i & 0x00100000) >> 20)
#define     BITMASK_DP_B25(i)           ((i & 0x02000000) >> 25)
#define     BITMASK_DP_ROT(i)           ((i & 0x00000F00) >>  8)
#define     BITMASK_DP_ROT_IM(i)        ((i & 0x000000FF) >>  0)
#define     BITMASK_DP_B4(i)            ((i & 0x00000010) >>  4)
#define     BITMASK_DP_RS(i)            ((i & 0x00000F00) >>  8)

#define     BITMASK_DP_SH_TYPE(i)       ((i & 0x00000060) >>  5)
#define     BITMASK_DP_SH_IMME(i)       ((i & 0x00000F80) >>  7)
#define     BITMASK_DP_SH_RM(i)         ((i & 0x0000000F) >>  0)

#define     BITMASK_BR_OFFSET(i)        ((i & 0x00FFFFFF) >>  0)
#define     BITMASK_BR_B24(i)           ((i & 0x01000000) >> 24)

#define     BRANCH_OFFSET_SH            23

#define     SHNDX_EQU                   0xFFF1

            // declared after the DP bitmasks as we need them
#define     IS_ADR(i)                   ((BITMASK_DP_OPCODE(i) == 2) || (BITMASK_DP_OPCODE(i) == 4))  && ( BITMASK_DP_RN(i) == 15  )
                                            // ADR = ADD/SUB Rd, R15, #nn

            // the next bit's very long code, but I've macro'd it because having it inside the actual code is horrible on the eyes
            // we check whether the next instruction is an ADD/SUB, which goes between the same register, and has the same Rd as the current instruction. 

#define     IS_ADRL(arr, i)             (((BITMASK_DP_OPCODE(armI[i]) == 2)   || (BITMASK_DP_OPCODE(armI[i]) == 4)) &&(BITMASK_DP_RN(armI[i]) == 15) && (IS_ADRL_SAME_REG(armI, i)))
#define     IS_ADRL_SAME_REG(arr, i)    (((BITMASK_DP_OPCODE(arr[i+1]) == 2) || (BITMASK_DP_OPCODE(arr[i+1]) == 4)) && (BITMASK_DP_RN(arr[i+1]) == BITMASK_DP_RD(arr[i+1]) && (BITMASK_DP_RD(arr[i+1]) == BITMASK_DP_RD(arr[i]))))

        // Function parameter constants
#define     TABLE_STR                   0
#define     TABLE_SHSTR                 1
#define     SHOW_COMMENTS               1
#define     NO_COMMENTS                 0

        // ELF Functions
#define     ELF32_ST_BIND(i)            ((i) >> 4)
#define     ELF32_ST_TYPE(i)            ((i) & 0xF)
#define     ELF32_ST_INFO(i)            (((b) << 4) + ((t) & 0xF))

        // Width of columns, used for outputting with proper spacings
#define     WIDTH_MEMORY                16
#define     WIDTH_BINARY                16
#define     WIDTH_BINARY_MIN_ASCII       4
#define     WIDTH_ASCII                 10
#define     WIDTH_LABEL                 24
#define     WIDTH_INSTRUCTION           64      // tested with [ LDMPLFD SP!,    {R0-R1, R3-R4, R6-R7, R9-R10, R12-SP, PC} ], the longest instruction I could determine
#define     WIDTH_COMMENTS              32
#define     WIDTH_BANNER_MINIMUM        40

        // Used to append to an sprintf() call
#define     EOS(s)                      ((s)+strlen(s))

        // Used when malloc-/realloc-ing space
#define     CHARS(c)                    (sizeof(char) * (c + 1))

        // Colours for outputting
#define     COLOUR_PSEUDO               35
#define     COLOUR_ASCII                95
#define     COLOUR_ERROR                91
#define     COLOUR_CONDCODE             33
#define     COLOUR_LABEL                94
#define     COLOUR_STATUSBIT            36
#define     COLOUR_HEADER               93
#define     COLOUR_COMMENT              90
#define     COLOUR_HEADER_TOP           92

#define     ERROR_ARGUMENTS             1
#define     ERROR_FILE_OPEN             2
#define     ERROR_INVALID_FILE_FORMAT   3
#define     ERROR_MEMORY                4


        // Easier way of outputting a uniform style of error message, adapted from http://c.learncodethehardway.org/book/ex20.html
#define     ERR(M, ...)                 fprintf(stderr, "\033[%dmError: " M "\n", COLOUR_ERROR, ## __VA_ARGS__);
                                            // we get compiler errors under C99 because some calls don't use the  (M, ...)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
//                       Type Declarations
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef unsigned char BOOL;     // we need this a couple of times, and it's easier to store this than a simple int, this also lets us know what's being stored more easily

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
//                       Global Variable Declarations
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

char gConditionCodes[15][6] =   // doing this as an array is much easier than having a switch/case, and because so many procedures/functions use it, we need it globally!
{
    "EQ", "NE", "CS-HS", "CC-LO", "MI", "PL", "VS",
    "VC", "HI", "LS", "GE", "LT", "GT", "LE",
    ""                          // should be "AL", but as we don't want it printed, leave it blank 
        // could have done it using ternary output, but nicer to do it this way, as less code to maintain/change
};

    // initialise as -1 so that we will know if they've been uninitialised - sometimes they *could* be a 0!

int gStringTableOffset = -1;

int gStringTableSize = -1;

int gStringTableCount = -1;

int gShStringTableOffset = -1;

int gShStringTableSize = -1;

int gShStringTableCount = -1;

int gSymbolTableOffset = -1;

int gSymbolTableSize = -1;

char *gFilename;                // dynamically malloc

BOOL gOutputAscii = 0;          // output ASCII codes next to the instruction, to help determine whether data or instruction

BOOL gOutputADRL = 0;           // allow users to select whether to run these, as they can be innacurate

BOOL gEndianism = 0;            // 0 = LittleEnd, 1 = BigEnd

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
//                       Function Declarations                              
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    // Put in order in which called

    /**
     *  Used in main()
     */
int main(int argc, const char *argv[]); // (obviously)

void processArguments(int argc, const char **argv);

void usageError();

void memoryError(FILE * fp);

void outputSectionHeaders(unsigned int sectHdrOffset,
                          unsigned int sectHdrCount);
void outputSymbolTable();

void niceBanner(const char *title);

    /**
     * Used in outputSymbolTable(), outputSectionHeaders(), and generateStringTableArray()
     */

const char *getStringNameFromSymName(int index, int tableType);

int getStringTableEntryCount(int tableType);


    /* 
       Disassembly and decoding of binary instructions 
     */
    // First called
void Disassemble(unsigned int *armI, int count, unsigned int startAddress);

    // Then called next
void DecodeInstruction(unsigned int instr, unsigned int currentAddress,
                       char *returnString, char **returnCommentsString,
                       int *returnCommentsStringCount);

    // and then one of these is called    
void DecodeSWI(unsigned int instr, char *returnString);

void DecodeBranch(unsigned int instr, unsigned int currentAddress,
                  char *returnString, char **returnCommentsString,
                  int *returnCommentsStringCount);

void DecodeMultipleLoadStore(unsigned int instr, char *returnString);

void DecodeLoadStore(unsigned int instr, unsigned int currentAddress,
                     char *returnString, char **returnCommentsString,
                     int *returnCommentsStringCount);

void DecodeMultiply(unsigned int instr, char *returnString);

void DecodeADR(unsigned int instr, unsigned int currentAddress,
               char *returnString, char **returnCommentsString,
               int *returnCommentsStringCount);

int DecodeADRL(unsigned int *armI, int i, unsigned int startAddress,
               char *returnString, char **returnCommentsString,
               int *returnCommentsStringCount);

void DecodeDataProcessing(unsigned int instr, char *returnString,
                          char **returnCommentsString,
                          int *returnCommentsStringCount);

const char *decodeOperand2(unsigned int instr, char **returnCommentsString,
                           int *returnCommentsStringCount, int doComments);

    /*
       Used for Disassemble() 
     */
void decodeASCII(unsigned int instr, char *returnString);

    /*
       Used in the decoder functions
     */
int SignExtend(unsigned int x, int bits);

int Rotate(unsigned int rotatee, int amount);

char *getLabelFromAddress(int address); // Also used in Disassemble()

int strlene(const char *string);

const char *returnRegisterString(unsigned int registerValue);


/***************************************************************************************************************************************************
 *  Main program implementation.
 * 
 *  Note: order of code listing is the order in which it is defined above. 
 ***************************************************************************************************************************************************/

/**
 *  Used in main()
 */

/**
 * main() controls the program's execution. 
 *
 *  As it's the main body of code, we do the main organising here. We start off by ensuring everything's okay with input, output some details about the file, section headers, symbol table, then get down to the disassembly itself. 
 *
**/
int main(int argc, const char *argv[])
{

    /* Check we have a valid filename/arguments */

    FILE *fp = NULL;

    processArguments(argc, argv);

    fp = fopen(gFilename, "rb");

    if (!fp) {                  // throw an error if we can't access it
        ERR("Cannot open file %s. Program will now halt.\n", gFilename);
        return 1;
    }

    /* Check we have a valid ELF file */

    ELFHEADER elfhead;

    fread(&elfhead, 1, sizeof(ELFHEADER), fp);
    if (!(elfhead.magic[0] == 0177 &&
          elfhead.magic[1] == 'E' &&
          elfhead.magic[2] == 'L' && elfhead.magic[3] == 'F')) {
        ERR("\"%s\" is not an ELF file\n", argv[1]);
        return 2;
    }

    /* Header and TODOs */

    char *title =               // only allocate enough space for the full string, as we don't need to append to it
        (char *) malloc(snprintf(NULL, 0,
                                 "File:  \033[%dm%s\033[%dm",
                                 COLOUR_HEADER,
                                 argv[1],
                                 COLOUR_HEADER_TOP) + 1);

    printf("\033[%dm", COLOUR_HEADER_TOP);
    sprintf(title, "File:  \033[%dm%s\033[%dm", COLOUR_HEADER, argv[1],
            COLOUR_HEADER_TOP);
    niceBanner(title);          // don't free as we do use it again later
    printf("\033[0m");


    // todoList();

    /* File Type and other ELF info */

    // Put these in an array as it's all sequential, and it makes it nicer formatting
    char fileTypes[5][20] = {
        "No file type", "Relocatable file",
        "Executable file", "Shared object file",
        "Core file"
    };

    printf("\033[%dm", COLOUR_COMMENT);
    niceBanner("ELF Header");
    printf("\033[0m");

    printf("\n");

    printf("File-type:\t\t%s (Type %d)\n",
           fileTypes[elfhead.filetype], elfhead.filetype);

    if (elfhead.class != 0)
        printf("File class:\t\t%d-bit ELF File\n",
               (elfhead.class == 1) ? 32 : 64);

    if (elfhead.archtype == 40)
        printf("Arch-type:\t\tAdvanced RISC Machine (ARM)\n");
    else
        printf("Arch-type:\t\t%d\n", elfhead.archtype);

    printf("Byte encoding:\t\tTwo's complement, %s endian\n",
           (elfhead.byteorder == 1) ? "little" : "big");

    printf("Entry point:\t\t\033[%dmMemory Address \033[0m0x%x\n",
           COLOUR_COMMENT, elfhead.entry);
    printf("Prog-Header:\t\t%d bytes into file\n", elfhead.phdrpos);
    printf("Prog-Header-count:\t%d\n", elfhead.phdrcnt);
    printf("Section-Header:\t\t%d bytes into file\n", elfhead.shdrpos);
    printf("Section-Header-count:\t%d\n", elfhead.shdrcnt);

    /* Read through the section headers */

    outputSectionHeaders(elfhead.shdrpos, elfhead.shdrcnt);

    /* Read through the symbol table */

    outputSymbolTable();

    /* Read through the program headers */

    ELFPROGHDR *prgHdr = NULL;

    unsigned int *armInstructions = NULL;

    fseek(fp, elfhead.phdrpos, SEEK_SET);

    prgHdr = (ELFPROGHDR *) malloc(sizeof(ELFPROGHDR));
    if (!prgHdr)
        memoryError(fp);


    title = (char *) malloc(CHARS(32));

    for (int x = 1; x <= elfhead.phdrcnt; x++) {

        fread(prgHdr, 1, sizeof(ELFPROGHDR), fp);

        if (prgHdr[0].filesize == 0)
            break;              // when a program header doesn't exist, then we're done

        // then output it all
        printf("\033[%dm", COLOUR_COMMENT);
        sprintf(title, "Program Header %d", x);
        niceBanner(title);
        printf("\033[0m");

        printf("\n");
        printf("Segment-Offset:\t\t%d bytes\n", prgHdr[0].offset);

        if (prgHdr[0].filesize == prgHdr[0].memsize) {
            printf("Size of segment:\t%d bytes\n", prgHdr[0].filesize);
        } else {
            printf("File-size:\t\t%d\n", prgHdr[0].filesize);
            printf("Memory Size:\t\t%d\n", prgHdr[0].memsize);
        }

        printf("Align:\t\t\t%d\n", prgHdr[0].align);

        if (prgHdr[0].virtaddr == prgHdr[0].physaddr) { // no point showing both if they're the same
            printf("Memory Address:\t\t0x%x\n", prgHdr[0].physaddr);
        } else {
            printf("Virtual Address:\t\t0x%x\n", prgHdr[0].virtaddr);
            printf("Physical Address:\t\t0x%x\n", prgHdr[0].physaddr);
        }

        printf("\n");
        printf("Type:\t\t\t%d\n", prgHdr[0].type);
        printf("Flags:\t\t\t%d\n", prgHdr[0].flags);


        /* allocate memory and read in ARM instructions */
        armInstructions =
            (unsigned int *) malloc((prgHdr->filesize + 3) & ~3);
        if (!armInstructions) {
            free(prgHdr);
            memoryError(fp);
        }
        fseek(fp, prgHdr->offset, SEEK_SET);
        fread(armInstructions, 1, prgHdr->filesize, fp);

        /* Disassemble */
        printf("\nInstructions\n============\n\n");

        Disassemble(armInstructions, ((prgHdr->filesize + 3) & ~3) / 4, // ensure we're on a 4-bit boundary
                    prgHdr->virtaddr);
        printf("\n");

        free(armInstructions);  // deallocate the memory

        fseek(fp, elfhead.phdrpos + (x * sizeof(ELFPROGHDR)), SEEK_SET);        // go to the next program header, luckily all stored sequentially in memory

    }                           // end while(1)

    /* Free any dynamically allocated data, and close the file */

    free(title);
    free(prgHdr);

    fclose(fp);

    return EXIT_SUCCESS;

}                               // end main

/**
 * processArguments() is a procedure to work out what has been passed into the program, check whether it is valid, and then act accordingly with the input. 
 *
 *  We need to allow our users to have some control over output, so let them specify what to enable. 
 *
**/
void processArguments(int argc, const char **argv)
{
    int c;

    if (argc == 1)              // if only called as ./disARM
        usageError();

    gFilename = (char *) malloc(CHARS(strlen(argv[1])));        // Only allocate enough space for the string
    strcpy(gFilename, argv[1]);

    while ((c = getopt(argc, argv, "alAL")) != -1) {    // allow caps, too
        switch (c) {
            case 'a':
            case 'A':
                gOutputAscii = 1;
                break;
            case 'l':
            case 'L':
                gOutputADRL = 1;
                break;
            case '?':
            default:
                usageError();
        }
    }
}

/**
 * usageError() is a procedure to tell users that they've called the program incorrectly. 
 *
 *  All this does is print out an error message, then exit. We do this instead here instead of code that could potentially be repeated. 
 *
**/
void usageError()
{
    ERR("\nCorrect usage: \t\033[%dm DisARM <filename[.elf]>\t(-a -l)\n\t\t\t\t\t-a=Ascii, -l=ADRL\033[0m", COLOUR_ERROR);
    exit(ERROR_ARGUMENTS);
}

/**
 * memoryError() is a procedure to tell users that memory has been depleted. Highly unlikely, but still possible. 
 *
 *  We close the file, and output the error. If anything else needs to be freed/closed, it is done before this is called. 
 *
**/
void memoryError(FILE * fp)
{
    ERR("Out of Memory");
    fclose(fp);
    exit(ERROR_MEMORY);
}

/**
 * outputSectionHeaders() is a procedure used at the top of the program, that first initialises globals for the program's use, and then outputs the section headers. 
 *
 *  This is the best place to set globals, unfortunately.
 *
**/
void outputSectionHeaders(unsigned int sectHdrOffset,
                          unsigned int sectHdrCount)
{
    FILE *fp;

    ELFSECTHEADER *sectHdr;

    int x, i, top;

    fp = fopen(gFilename, "rb");

    sectHdr = (ELFSECTHEADER *) malloc(sizeof(ELFSECTHEADER));

    if (!sectHdr)
        memoryError(fp);

    char *title = (char *) malloc(CHARS(32));

    sprintf(title, "Section Headers (%d value%s)", sectHdrCount, sectHdrCount > 1 ? "s" : "");  // output plurals nicely

    printf("\n\n");
    printf("\033[90m");
    niceBanner(title);
    free(title);
    printf("\033[0m");

    printf("\033[%dm\n\n", COLOUR_HEADER);
    printf("%s", "Index");
    top = strlene("Index");
    for (i = 8; i > top; i--)
        printf(" ");

    printf("%s", "Name");
    top = strlene("Name");
    for (i = 24; i > top; i--)
        printf(" ");

    printf("%s", "Type");
    top = strlene("Type");
    for (i = 16; i > top; i--)
        printf(" ");

    printf("%s", "Memory Address*");
    top = strlene("Memory Address*");
    for (i = 18; i > top; i--)
        printf(" ");

    printf("%s", "Offset (Bytes)");
    top = strlene("Offset (Bytes)");
    for (i = 16; i > top; i--)
        printf(" ");

    printf("%s", "Size (Bytes)");
    top = strlene("Size (Bytes)");
    for (i = 12; i > top; i--)
        printf(" ");

    printf("\033[0m\n");



    // first we do some checks to initialise variables, this is the easiest place to do this, so we just need to skip over it if we've already done it

    if (gStringTableSize == -1 && gStringTableSize == -1) {
        fseek(fp, sectHdrOffset, SEEK_SET);

        for (x = 0; x < sectHdrCount; x++) {
            fread(sectHdr, 1, sizeof(ELFSECTHEADER), fp);
            if (x == 0)
                continue;       // there is always one null 

            if (sectHdr[0].type == 2) { // Symbol Table
                gSymbolTableOffset = sectHdr[0].offset;
                gSymbolTableSize = sectHdr[0].size;
            } else if (sectHdr[0].type == 3) {  // String Table

                // next we have a hack due to the fact that the String table and the Section Header string tables have the same type. Therefore, as the standard one is first, we assign to that, then if we know that we've assigned to it, we can assign to the Section Header one

                if (gStringTableOffset == -1 && gStringTableSize == -1) {       // never usually 0 (always 1 NUL)

                    gStringTableOffset = sectHdr[0].offset;
                    gStringTableSize = sectHdr[0].size;
                    gStringTableCount =
                        getStringTableEntryCount(TABLE_STR);
                } else {
                    gShStringTableOffset = sectHdr[0].offset;
                    gShStringTableSize = sectHdr[0].size;
                    gShStringTableCount =
                        getStringTableEntryCount(TABLE_SHSTR);
                }
            }                   // end elseif type==3

        }                       // end for (sectHdrCount)
    }                           // end if( ... == -1)





    char *currentColumn = (char *) malloc(CHARS(32));

    char *currentLabel = (char *) malloc(CHARS(32));

    fseek(fp, sectHdrOffset, SEEK_SET);
    for (x = 0; x < sectHdrCount; x++) {
        fread(sectHdr, 1, sizeof(ELFSECTHEADER), fp);

        sprintf(currentColumn, "%d", x);
        printf("%s", currentColumn);
        top = strlene(currentColumn);
        for (i = 8; i > top; i--)
            printf(" ");

        currentLabel =
            getStringNameFromSymName(sectHdr[0].name, TABLE_SHSTR);

        if (strlen(currentLabel) >= 24) {       // only print enough to fit in our column
            // sprintf(currentColumn, )
            sprintf(currentColumn, "");

            strncat(currentColumn, currentLabel, 8);
            strcat(currentColumn, "...");
            strncat(currentColumn, currentLabel + strlen(currentLabel) - 8,
                    8);

            strcat(currentColumn, "\0");
        } else {
            sprintf(currentColumn, "%s", currentLabel);
        }

        printf("%s", currentColumn);
        top = strlene(currentColumn);
        for (i = 24; i > top; i--)
            printf(" ");

        switch (ELF32_ST_TYPE(sectHdr[0].type)) {
            case 0:
                sprintf(currentColumn, "Undefined");
                break;

            case 1:
                sprintf(currentColumn, "Program Data");
                break;

            case 2:
                sprintf(currentColumn, "Symbol Table");
                break;

            case 3:
                sprintf(currentColumn, "String Table");
                break;

            default:
                sprintf(currentColumn, "Other");
                break;

        }

        // sprintf(currentColumn, "%d", );
        printf("%s", currentColumn);
        top = strlene(currentColumn);
        for (i = 16; i > top; i--)
            printf(" ");

        if (ELF32_ST_TYPE(sectHdr[0].type) == 1) {      // if only program bits, output address
            sprintf(currentColumn, "0x%x", sectHdr[0].address);
            printf("%s", currentColumn);
            top = strlene(currentColumn);
        } else {                // otherwise blank
            top = 0;
        }

        for (i = 18; i > top; i--)
            printf(" ");

        sprintf(currentColumn, "%d", sectHdr[0].offset);
        printf("%s", currentColumn);
        top = strlene(currentColumn);
        for (i = 18; i > top; i--)
            printf(" ");

        sprintf(currentColumn, "%d", sectHdr[0].size);
        printf("%s", currentColumn);
        top = strlene(currentColumn);
        for (i = 16; i > top; i--)
            printf(" ");


        printf("\n");
    }                           // end for(section)

    printf
        ("\n\033[%dm* \033[%dmOnly applicable to program headers \033[0m\n",
         COLOUR_HEADER, COLOUR_COMMENT);


}

/**
 * outputSymbolTable() is a procedure used at the top of the program, that outputs the symbol table, grabbing the actual values from the string table. 
 *
**/
void outputSymbolTable()
{
    FILE *fp;

    ELFSYMTABLE *symTabl;

    int x;

    int top, i;

    fp = fopen(gFilename, "rb");
    fseek(fp, gSymbolTableOffset, SEEK_SET);
    symTabl = (ELFSYMTABLE *) malloc(sizeof(ELFSYMTABLE));
    if (!symTabl)
        memoryError(fp);

    char *title = (char *) malloc(CHARS(32));

    printf("\n\n");
    printf("\033[%dm", COLOUR_COMMENT);

    x = getStringTableEntryCount(TABLE_STR) + 1;
    sprintf(title, "Symbol Table (%d value%s)", x, x > 1 ? "s" : "");
    niceBanner(title);
    free(title);
    printf("\033[0m");

    // USE ARRAY TO STORE HEADERS AND OFFSETS

    printf("\033[%dm\n\n", COLOUR_HEADER);
    printf("%s", "Index");
    top = strlene("Index");
    for (i = 8; i > top; i--)
        printf(" ");

    printf("%s", "Name");
    top = strlene("Name");
    for (i = 24; i > top; i--)
        printf(" ");

    printf("%s", "Address/Value");
    top = strlene("Address/Value");
    for (i = 32; i > top; i--)
        printf(" ");

    printf("%s", "Size");
    top = strlene("Size");
    for (i = 12; i > top; i--)
        printf(" ");

    printf("%s", "Type");
    top = strlene("Type");
    for (i = 12; i > top; i--)
        printf(" ");

    printf("%s", "Bind");
    top = strlene("Bind");
    for (i = 12; i > top; i--)
        printf(" ");

    printf("%s", "Section Header Index");

    printf("\033[0m\n");


    char *currentColumn = (char *) malloc(CHARS(32));

    for (x = 0; x <= getStringTableEntryCount(TABLE_STR); x++) {
        fread(symTabl, 1, sizeof(ELFSYMTABLE), fp);

        sprintf(currentColumn, "%d", x);
        printf("%s", currentColumn);
        top = strlene(currentColumn);
        for (i = 8; i > top; i--)
            printf(" ");

        printf("%s", getStringNameFromSymName(symTabl[0].name, TABLE_STR));

        top =
            strlene(getStringNameFromSymName(symTabl[0].name, TABLE_STR));
        for (i = 24; i > top; i--)
            printf(" ");


        if (symTabl[0].shndx != SHNDX_EQU)      // != SHN_ABS
            sprintf(currentColumn, "\033[%dmMemory\033[0m 0x%x",
                    COLOUR_COMMENT, symTabl[0].value);
        else                    // == SHN_ABS
            sprintf(currentColumn, "\033[%dmEQU   \033[0m #%u", // spaces to align with Memory outputs
                    COLOUR_COMMENT, symTabl[0].value);

        printf("%s", currentColumn);
        top = strlene(currentColumn);
        for (i = 32; i > top; i--)
            printf(" ");

        sprintf(currentColumn, "%d", symTabl[0].size);
        printf("%s", currentColumn);
        top = strlene(currentColumn);
        for (i = 12; i > top; i--)
            printf(" ");

        char elfTypes[5][10] = {
            "No Type", "Object", "Function",
            "Section", "File"
        };

        sprintf(currentColumn, "%s",
                elfTypes[ELF32_ST_TYPE(symTabl[0].info)]);

        printf("%s", currentColumn);
        top = strlene(currentColumn);
        for (i = 12; i > top; i--)
            printf(" ");

        switch (ELF32_ST_BIND(symTabl[0].info)) {
            case 0:
                sprintf(currentColumn, "Local");
                break;

            case 1:
                sprintf(currentColumn, "Global");
                break;

            case 2:
                sprintf(currentColumn, "Weak");
                break;
        }

        printf("%s", currentColumn);
        top = strlene(currentColumn);
        for (i = 12; i > top; i--)
            printf(" ");


        switch (symTabl[0].shndx) {

            case 0:
                sprintf(currentColumn, "Undefined");
                break;

            case SHNDX_EQU:
                sprintf(currentColumn, "Compiler Constant");
                break;

            default:
                if (symTabl[0].shndx <= (gStringTableCount + 1))        // gStringTableCount is the same as the would-be gSymbolTableCount - one entry in either for each string!
                    sprintf(currentColumn, "Program Header %d",
                            symTabl[0].shndx);
                else
                    // sprintf(currentColumn, "ERR");
                    printf("%d %d", symTabl[0].shndx,
                           gStringTableCount + 1);
                break;
        }

        printf("%s", currentColumn);
        top = strlene(currentColumn);
        for (i = 12; i > top; i--)
            printf(" ");

        printf("\033[0m\n");
    }
    free(symTabl);
    printf("\n\n");
}


/**
 * niceBanner() is a procedure used to output a equals sign banner around main titles. 
 *
 *  In terms of aesthetics, it does make the program nicer, and it generates a header wide enough to fit *title in, which means there's no guessing, or any longer strings looking weird. 
 *
**/
void niceBanner(const char *title)
{
    int whole = (strlene(title) > WIDTH_BANNER_MINIMUM) ? strlene(title) + 8    // and some extra padding
        : WIDTH_BANNER_MINIMUM;

    // we need everything at least a certain width, and if the string is more than that, adjust to that length, plus gutter padding

    int gutter = (whole - strlene(title)) / 2;

    int i = 0;

    printf("\t");
    for (i = 0; i <= whole; i++)
        printf("=");
    printf("\n");

    printf("\t");
    for (i = 0; i <= gutter; i++)
        printf(" ");
    printf("%s\n", title);

    printf("\t");
    for (i = 0; i <= whole; i++)
        printf("=");
    printf("\n");
}

/**
 * Used in outputSymbolTable(), outputSectionHeaders(), and generateStringTableArray()
 */


/**
 * getStringNameFromSymName() is a function to get labels and strings from the symbol table name. 
 *
 *  This function is invaluable when outputting the symbol table, and is adapted later for addresses, used in the main dissassembly. 
 *
**/
const char *getStringNameFromSymName(int index, int tableType)
{
    FILE *fp;

    int i = 0, currChar = 0, currCharPtr = 0;

    char *returnString = (char *) malloc(CHARS((((tableType ==
                                                  TABLE_STR) ?
                                                 gStringTableSize :
                                                 gShStringTableSize)
                                                - index) + 1)
        );                      // we won't need to go before the current index, therefore don't allocate that much space

    fp = fopen(gFilename, "rb");

    fseek(fp,
          ((tableType ==
            TABLE_STR) ? gStringTableOffset : gShStringTableOffset),
          SEEK_SET);

    for (i = 0;
         i <=
         ((tableType ==
           TABLE_STR) ? gStringTableSize : gShStringTableSize) + 1; i++) {
        if (i < index) {        // this will now only execute once
            while (i++ < index) // increment after the loop is executed
                fseek(fp, sizeof(char), SEEK_CUR);      // and keep going until we're at the right place!
            continue;           // then go to to the next execution, and we'll do the main code
        }

        while (fread(&currChar, 1, sizeof(char), fp) == 1 &&    // as long as we're reading a character in, keep going
               currChar != 0)   // if we reach a NUL, we're done with our string
            returnString[currCharPtr++] = currChar;     // otherwise add the character to the string

        break;                  // when we've finished reading our string 
    }

    returnString[currCharPtr] = 0;      // add an extra NUL, or we get some cray results..

    return (index > 0) ? returnString   // this flags the following:   "warning: function returns address of local variable [enabled by default]""
        : "(NULL)";             // this shouldn't be returned, but in case it is called anyway
}

/**
 * getStringTableEntryCount() is a function to find out how many non-null strings are in the table. 
 *
 *  This is used when we're working out the symbol table, as the header doesn't tell us anything about how many *strings* there are, just how many bytes the tables take up. 
 *
**/
int getStringTableEntryCount(int tableType)
{
    FILE *fp;

    int entryCount = 0, i = 0, currChar = 0, lastNull = 0;      // if we get 00 00, we know we're at the end

    fp = fopen(gFilename, "rb");

    fseek(fp,
          ((tableType ==
            TABLE_STR) ? gStringTableOffset : gShStringTableOffset),
          SEEK_SET);
    for (i = 0;
         i <=
         ((tableType ==
           TABLE_STR) ? gStringTableSize : gShStringTableSize) + 1; i++) {

        fread(&currChar, 1, sizeof(char), fp);  // read in a character

        if (i == 0)
            continue;           // leading NUL

        // Note: could put this in the for loop conditions, but it doesn't look very pretty, and doesn't really save much

        if (currChar == 0) {    // if we reach a NUL, we're done with our string
            if (lastNull == 1)  // if we have two consecutive, we're done with the table
                break;

            entryCount++;       // otherwise we have a word
            lastNull = 1;       // and the last one was a NUL
        } else
            lastNull = 0;       // if it's a normal character, don't do this
    }

    fclose(fp);

    // printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! %d", entryCount);

    return entryCount;
}




/** 
 *  Disassembly and decoding of binary instructions 
**/

/**
 * Disassemble() is the function that starts off the disassembly
 *
 *  This starts by outputting headers, then each individual line's code. If gOutputADRL is set, ADRLs are attemptedly decoded, provided they are legitimately an ADRL. 
 *
**/
void Disassemble(unsigned int *armI, int count, unsigned int startAddress)
{
    int i, x, y, top;

    printf("\033[%dm", COLOUR_HEADER);

    printf("Memory");
    // for( x = 16; x > strlen("Memory"); x-- )
    for (x = 16; x > strlen("Memory"); x--)
        printf(" ");

    printf("Binary");
    for (x = (gOutputAscii == 1)
         ? (WIDTH_BINARY - WIDTH_BINARY_MIN_ASCII)
         : WIDTH_BINARY; x > strlen("Binary"); x--)
        printf(" ");

    if (gOutputAscii == 1) {
        printf("ASCII");
        for (x = WIDTH_ASCII; x > strlen("ASCII"); x--)
            printf(" ");
    }

    printf("Label");
    for (x = WIDTH_LABEL; x > strlen("Label"); x--)
        printf(" ");

    printf("Instruction");
    for (x = WIDTH_INSTRUCTION; x > strlen("Instruction"); x--)
        printf(" ");

    printf("DisARM Comments");
    for (x = WIDTH_COMMENTS; x > strlen("DisARM Comments"); x--)
        printf(" ");



    printf("\n\033[0m");

    char *thisMemory = (char *) malloc(CHARS(WIDTH_MEMORY));

    char *thisBinary = (char *)
        malloc(CHARS
               ((gOutputAscii ==
                 1) ? WIDTH_BINARY -
                WIDTH_BINARY_MIN_ASCII : WIDTH_BINARY));
    char *thisLabel = (char *) malloc(CHARS(WIDTH_LABEL));

    char *thisInstruction = (char *) malloc(CHARS(WIDTH_INSTRUCTION));

    char *thisAscii = (char *) malloc(CHARS(WIDTH_ASCII));

    int lineHasLabel = 0;

    char *decodeString;

    char **decodeComments;

    int decodeCommentsCount = 0;


    int inADRL = 0;


    for (i = 0; i < count; i++) {
        // next = 0;
        // currentOffset = 0;
        sprintf(thisMemory, "%08x:", startAddress + i * 4);

        top = strlene(thisMemory);
        for (x = WIDTH_MEMORY; x > top; x--)
            sprintf(EOS(thisMemory), " ");

        sprintf(thisBinary, "%08x    ", armI[i]);
        top = strlene(thisBinary);

        for (x =
             (gOutputAscii ==
              1) ? (WIDTH_BINARY - WIDTH_BINARY_MIN_ASCII) : WIDTH_BINARY;
             x > top; x--)
            sprintf(EOS(thisBinary), " ");

        printf("%s%s", thisMemory, thisBinary);

        if (gOutputAscii == 1) {        /*6 total */
            decodeASCII(armI[i], thisAscii);

            top = strlene(thisAscii);
            for (x = WIDTH_ASCII; x > top; x--)
                sprintf(EOS(thisAscii), " ");

            printf("\033[%dm%s\033[0m", COLOUR_ASCII, thisAscii);
        }

        for (x = 0; x < 4; x++) {
            thisLabel = getLabelFromAddress(startAddress + i * 4 + x);  // we need to use it a couple of times, so cut down executions by storing it, and only calling it once then

            if (strlen(thisLabel) > 0) {        /*24 total */
                lineHasLabel = 1;

                if (x != 0)     // show line offset if not properly aligned
                    sprintf(thisLabel, "%s (0x%x)",
                            thisLabel, startAddress + i * 4 + x);

                printf("%s", thisLabel);

                for (y = 24; y > strlene(thisLabel); y--)
                    printf(" ");
            }
        }                       // end for(x)

        if (lineHasLabel == 0)
            for (y = 0; y < 24; y++)
                printf(" ");

        decodeCommentsCount = 0;


        // We then first determine that it is an ADRL, that is:
        //  ADD/SUB Rd, PC, #offset
        // followed by one or more of the following:
        //  ADD/SUB Rd, Rn, #offset         (where Rn and Rd are the same)

        //  We use the macro IS_ADRL and IS_ADRL_SAME_REG() as having the full code would be horrible. Although this means that a programmer will have to go to the declarations, it makes using the code a lot easier as about 10 lines of an if statement isn't very nice!
        if (gOutputADRL && IS_ADRL(armI, i)) {

            decodeString = (char *) malloc(CHARS(WIDTH_INSTRUCTION));
            inADRL = DecodeADRL(armI, i, startAddress, decodeString,
                                decodeComments, &decodeCommentsCount);
            i++;                // need to go one ahead, otherwise will show last instruction of the ADRL
        }

        if (inADRL == 0)
            DecodeInstruction(armI[i], startAddress + i * 4, decodeString,
                              decodeComments, &decodeCommentsCount);
        else {
            inADRL = 0;         // reset it after DI() otherwise will execute
        }

        printf("\n");
        lineHasLabel = 0;
    }                           // end for

    free(thisMemory);
    free(thisBinary);
    free(thisInstruction);
}                               // end Disassemble()

/** 
 *  Decoders
**/

/**
 * DecodeInstruction() is the function that determines which instructions are which, and how to decode them. 
 *
 *  We use preprocessor macros to make the code a lot nicer to read, and to make it easier to call these checks elsewhere. Comments are then printed at the end, with the instruction.
 *
**/
void
DecodeInstruction(unsigned int instr, unsigned int currentAddress,
                  char *returnString, char **returnCommentsString,
                  int *returnCommentsStringCount)
{

    returnCommentsString = (char **) malloc(CHARS(1));

    returnString = (char *) malloc(CHARS(WIDTH_INSTRUCTION));
    strcpy(returnString, "\033[91mUnknown instruction\033[0m"); // Some instructions haven't been coded, and when decoding data, there may be errors

    int skip = 0;

    /******************
        SWI decodes here 
     ******************/
    if (IS_SWI(instr)) {
        if (IS_SWI_IMMED(instr))        // ignore anything else, as they're not relevant
            DecodeSWI(instr, returnString);
    }

    /******************
        Branches decode here 
     ******************/
    else if (IS_BR(instr))
        DecodeBranch(instr, currentAddress, returnString,
                     returnCommentsString, returnCommentsStringCount);

    /******************
        Multiple Load/Store decodes here 
     ******************/
    else if (IS_MLDST(instr))
        DecodeMultipleLoadStore(instr, returnString);

    /******************
        Load/Store decodes here 
     ******************/
    else if (IS_LDST(instr))
        DecodeLoadStore(instr, currentAddress, returnString,
                        returnCommentsString, returnCommentsStringCount);

    /******************
        Multiply decodes here 
     ******************/
    else if (IS_MULT(instr) && IS_MULT_2(instr))
        // note that we need two checks here, otherwise AND/EOR etc match
        DecodeMultiply(instr, returnString);

    /******************
        Data Processing decodes here 
     ******************/
    else if (IS_DP(instr)) {

        if (IS_ADR(instr))      // ADR is an ADD/SUB, so shows up as a DP instruction
            DecodeADR(instr, currentAddress, returnString,
                      returnCommentsString, returnCommentsStringCount);
        else
            DecodeDataProcessing(instr, returnString, returnCommentsString,
                                 returnCommentsStringCount);
    }


    if (*returnCommentsStringCount == 0) {
        returnCommentsString[*returnCommentsStringCount] =
            (char *) malloc(CHARS(3));
        sprintf(returnCommentsString[0], " ");
        ++*returnCommentsStringCount;
    }

    if (skip == 0) {            // output DP inside function, as a couple of problems
        printf("%s", returnString);
        int top = strlene(returnString);

        for (int x = 64; x > top; x--)
            printf(" ");
    }



    if (*returnCommentsStringCount > 0) {
        printf("\033[%dm%s\033[0m", COLOUR_COMMENT, returnCommentsString[0]);   //(strlen(returnCommentsString[0]) > 0) ? returnCommentsString[0] : "");
        int x, y;

        if (*returnCommentsStringCount - 1 >= 1) {      // increments after each usage              TODO: MAKE IT INCREMENT WHEN NEEDED????
            for (y = 1; y < *returnCommentsStringCount; y++) {
                printf("\n");
                for (x = 0; x <
                     // 
                     //  WIDTH_MEMORY + 
                     // ((gOutputAscii == 1)    ? (WIDTH_BINARY)   // output spacing enough for when the ASCII is/isn't there
                     //                         : WIDTH_BINARY - WIDTH_BINARY_MIN_ASCII) 
                     // + WIDTH_LABEL + WIDTH_INSTRUCTION + WIDTH_BINARY_MIN_ASCII + 2;
                     16 + 12 + ((gOutputAscii == 1) ? 6 : 0) + 24 + 64 + 4;
                     x++)
                    printf(" ");
                printf("\033[90m%s\033[0m", returnCommentsString[y]);
            }
        }
    }

}                               // end DecodeInstruction()

/** 
 *  DecodeSWI(), nothing special here. 
 *
 */
void DecodeSWI(unsigned int instr, char *returnString)
{
    returnString = (char *) realloc(returnString, snprintf(NULL, 0,
                                                           "SWI\033[%dm%s\033[0m          %u\033[0m",
                                                           COLOUR_CONDCODE,
                                                           gConditionCodes
                                                           [BITMASK_CONDCODE
                                                            (instr)],
                                                           BITMASK_SWI_IMMED
                                                           (instr)) + 1);
    sprintf(returnString, "SWI\033[%dm%s\033[0m", COLOUR_CONDCODE,
            gConditionCodes[BITMASK_CONDCODE(instr)]);

    int top = strlene(returnString);

    for (int i = 8; i > top; i--)
        sprintf(EOS(returnString), " ");
    sprintf(EOS(returnString), "%u\033[0m", BITMASK_SWI_IMMED(instr));
}

/** 
 *  DecodeBranch(), nothing that special here, apart from using the SignExtend() function.
 *
 */
void
DecodeBranch(unsigned int instr, unsigned int currentAddress,
             char *returnString, char **returnCommentsString,
             int *returnCommentsStringCount)
{
    sprintf(returnString,
            "B%s\033[%dm%s\033[0m",
            (BITMASK_BR_B24(instr) == 1) ? "L"
            : "",
            COLOUR_CONDCODE, gConditionCodes[BITMASK_CONDCODE(instr)]);

    int top = strlene(returnString);    // need it declared here, otherwise it's executed multiple times, each time changing

    for (int i = 8; i > top; i--)
        sprintf(EOS(returnString), " ");

    sprintf(EOS(returnString), "%s",
            getLabelFromAddress(currentAddress +
                                SignExtend(BITMASK_BR_OFFSET(instr) << 2,
                                           BRANCH_OFFSET_SH) + 8));

    returnCommentsString[*returnCommentsStringCount] = (char *)
        malloc(snprintf
               (NULL, 0, "%s\033[%dm is stored at address 0x%x\033[0m",
                getLabelFromAddress(currentAddress +
                                    SignExtend(BITMASK_BR_OFFSET(instr) <<
                                               2, BRANCH_OFFSET_SH) + 8),
                COLOUR_COMMENT,
                currentAddress + SignExtend(BITMASK_BR_OFFSET(instr) << 2,
                                            BRANCH_OFFSET_SH) + 8) + 1);

    sprintf(returnCommentsString[(*returnCommentsStringCount)++],       // increment after it's used
            "%s\033[%dm is stored at address 0x%x\033[0m",
            getLabelFromAddress(currentAddress +
                                SignExtend(BITMASK_BR_OFFSET(instr) << 2,
                                           BRANCH_OFFSET_SH) + 8),
            COLOUR_COMMENT,
            currentAddress + SignExtend(BITMASK_BR_OFFSET(instr) << 2,
                                        BRANCH_OFFSET_SH) + 8);
}

/** 
 *  DecodeMultipleLoadStore() has lots of ternary to output our stack structure and writeback, as well as an array of bitmasks for use with a loop. We also output the registers in the format Rx-Ry if it saves space.
 *
 *  Working out how to do the Rx-Ry took a bit of thinking. One main problem was reaching the end of the registers in a list, but having a trailing comma. To fix this, a loop goes from the top to bottom, and finds out when we're at the uppermost. Then we loop through, and work out whether the current register bit is set, and if so, whether the bit ahead of it is set. If so, we then set it up to do a list, and output nicely. If we have the end of the registers, we output without comma, then we're done. If it hits a 0 bit for a register, and the last register was set, we output it and carry on. The crazy ternary is in order to ensure that we've got  
 */
void DecodeMultipleLoadStore(unsigned int instr, char *returnString)
{
    int i;

    int multLoadStoreBitmasks[16] = {   // use an array for easy manipulation with a for loop
        0x00000001, 0x00000002, 0x00000004, 0x00000008,
        0x00000010, 0x00000020, 0x00000040, 0x00000080,
        0x00000100, 0x00000200, 0x00000400, 0x00000800,
        0x00001000, 0x00002000, 0x00004000, 0x00008000
    };


    returnString =
        (char *) realloc(returnString, CHARS(WIDTH_INSTRUCTION));

    sprintf(returnString, "%s\033[%dm%s\033[0m",
            (BITMASK_MLDST_LOADSTORE(instr) == 0) ? "STM"
            : "LDM",
            COLOUR_CONDCODE, gConditionCodes[BITMASK_CONDCODE(instr)]);


    // braces needed, otherwise it won't work, DUHH
    if (BITMASK_MLDST_PREPOST(instr) == 1) {
        sprintf(EOS(returnString), "%s", (BITMASK_MLDST_LOADSTORE(instr) == 0) ? "F"    // pre-inc load
                : "E");         // pre-inc store
    } else if (BITMASK_MLDST_PREPOST(instr) == 0) {
        sprintf(EOS(returnString), "%s", (BITMASK_MLDST_LOADSTORE(instr) == 0) ? "E"    // post-inc load
                : "F");         // post-inc store
    }

    if (BITMASK_MLDST_UPDOWN(instr) == 1) {
        sprintf(EOS(returnString), "%s", (BITMASK_MLDST_LOADSTORE(instr) == 0) ? "A"    // pre-inc load
                : "D");         // pre-inc store
    } else if (BITMASK_MLDST_UPDOWN(instr) == 0) {
        sprintf(EOS(returnString), "%s", (BITMASK_MLDST_LOADSTORE(instr) == 0) ? "D"    // post-inc load
                : "A");         // post-inc store
    }

    int top = strlene(returnString);

    for (int i = 8; i > top; i--)
        sprintf(EOS(returnString), " ");

    sprintf(EOS(returnString), "%s%s,", returnRegisterString(BITMASK_MLDST_RN(instr)), (BITMASK_MLDST_WRITEBACK(instr) == 1) ? "!"      // == 1
            : ""                // == 0
        );

    top = strlene(returnString);
    for (int i = 16; i > top; i--)
        sprintf(EOS(returnString), " ");

    sprintf(EOS(returnString), "\033[0m{");

    int doList = 0, lastRegister = -1;

    for (i = 16; i > 0; i--) {
        if (BITMASK_MLDST_SET(instr, multLoadStoreBitmasks, i)) {
            lastRegister = i;
            break;
        }
    }

    for (i = 0; i < 16; i++) {
        if (BITMASK_MLDST_SET(instr, multLoadStoreBitmasks, i)) {
            if (doList != 1 && (BITMASK_MLDST_SETN(instr, multLoadStoreBitmasks, i))) { // next is set
                doList = 1;
                sprintf(EOS(returnString), "%s-", returnRegisterString(i));
            }
            if (i == lastRegister) {
                sprintf(EOS(returnString), "%s", returnRegisterString(i));
                break;
            }

        } else {                // the next occurance of a null bit
            doList = 0;
            if (BITMASK_MLDST_SET(instr, multLoadStoreBitmasks, ((i != 0) ? (i - 1) : i)))      // without the ternary, will cause an error if R0 is encountered
                sprintf(EOS(returnString), "%s, ",
                        returnRegisterString((i != 0) ? (i - 1) : i));
        }
    }
    sprintf(EOS(returnString), "}");
}

/** 
 *  DecodeLoadStore() has only got one important thing - the decoding of the pseudo-instruction LDR Rx, R15, #PCoffset. 
 *  
 *  This may not be not very fun for the eyes, but once reading, it should be pretty easy to understand. 
 */
void
DecodeLoadStore(unsigned int instr, unsigned int currentAddress,
                char *returnString, char **returnCommentsString,
                int *returnCommentsStringCount)
{
    int top = 0;

    returnString =
        (char *) realloc(returnString, CHARS(WIDTH_INSTRUCTION));

    char shiftTypes[5][4] = { "LSL", "LSR",
        "ASR", "ROR",
        "RRX"
    };

    if (BITMASK_LDST_RN(instr) == 15) { // pseudo-instruction LDR Rx, R15, #PCoffset for loading a DEFW'd value

        int offset = currentAddress + BITMASK_LDST_IMMED(instr) + 8;    // PC always 8 ahead

        if (strlen(getLabelFromAddress(offset)) > 0) {
            char *thisLabel = getLabelFromAddress(offset);

            sprintf(returnString, "LDR\033[%dm%s\033[0m",
                    COLOUR_CONDCODE,
                    gConditionCodes[BITMASK_CONDCODE(instr)]);

            top = strlene(returnString);
            for (int i = 8; i > top; i--)
                sprintf(EOS(returnString), " ");

            sprintf(EOS(returnString), "%s,",
                    returnRegisterString(BITMASK_LDST_RD(instr)));
            top = strlene(returnString);
            for (int i = 16; i > top; i--)
                sprintf(EOS(returnString), " ");

            sprintf(EOS(returnString), "%s", thisLabel);

            returnCommentsString[*returnCommentsStringCount]
                = (char *)
                malloc(snprintf
                       (NULL, 0, "%s\033[%dm is stored at address 0x%x",
                        thisLabel, COLOUR_COMMENT, offset) + 1);
            sprintf(returnCommentsString[(*returnCommentsStringCount)++],
                    "%s\033[%dm is stored at address 0x%x", thisLabel,
                    COLOUR_COMMENT, offset);
        } else {
            sprintf(returnString, "LDR\033[%dm%s\033[0m",
                    COLOUR_CONDCODE,
                    gConditionCodes[BITMASK_CONDCODE(instr)]);

            top = strlene(returnString);
            for (int i = 8; i > top; i--)
                sprintf(EOS(returnString), " ");

            sprintf(EOS(returnString), "%s,",
                    returnRegisterString(BITMASK_LDST_RD(instr)));
            top = strlene(returnString);
            for (int i = 16; i > top; i--)
                sprintf(EOS(returnString), " ");

            sprintf(EOS(returnString), "=%x", offset);

            returnCommentsString[*returnCommentsStringCount]
                = (char *)
                malloc(snprintf
                       (NULL, 0, "Loads the value stored at address 0x%x",
                        offset) + 1);
            sprintf(returnCommentsString[(*returnCommentsStringCount)++],
                    "Loads the value stored at address 0x%x", offset);
        }

        return;
    }                           // end if Rn == PC/R15

    sprintf(returnString, "%s\033[33m%s\033[0m%s",
            (BITMASK_MLDST_LOADSTORE(instr) == 0) ? "STR"
            : "LDR",
            gConditionCodes[BITMASK_CONDCODE(instr)],
            (BITMASK_LDST_WORDBYTE(instr) == 1) ? "B" : "");

    top = strlene(returnString);

    for (int i = 8; i > top; i--)
        sprintf(EOS(returnString), " ");

    sprintf(EOS(returnString), "%s,",
            returnRegisterString(BITMASK_LDST_RD(instr)));

    top = strlene(returnString);
    for (int i = 16; i > top; i--)
        sprintf(EOS(returnString), " ");

    sprintf(EOS(returnString), "[%s%s", returnRegisterString(BITMASK_LDST_RN(instr)), (BITMASK_LDST_PREPOST(instr) == 0) ? "]"  // post-increment
            : "");

    if (BITMASK_LDST_B25(instr) == 1) {
        if (BITMASK_LDST_SH_IMME(instr) == 0) { // don't need to shift
            sprintf(EOS(returnString), ", %s",
                    returnRegisterString(BITMASK_LDST_IMMED(instr)));

            if (BITMASK_DP_SH_TYPE(instr) == 3) {
                sprintf(EOS(returnString), ", ");

                top = strlene(returnString);

                for (int i = 32; i > top; i--)
                    sprintf(EOS(returnString), " ");
                sprintf(EOS(returnString), "RRX");

            }

        } else {                //if( (instr & BITMASK_LDST_SH_IMME) >> 7 == 0 ){
            sprintf(EOS(returnString),
                    ", %s, %s #%u",
                    returnRegisterString(BITMASK_LDST_SH_RM(instr)),
                    shiftTypes[BITMASK_LDST_SH_TYPE(instr)],
                    BITMASK_LDST_SH_IMME(instr));
        }
    } else if (BITMASK_LDST_B25(instr) == 0)
        sprintf(EOS(returnString), ", #%u", BITMASK_LDST_IMMED(instr));


    if (BITMASK_LDST_PREPOST(instr) == 1)       // if pre-increment
        sprintf(EOS(returnString), "]");        // close after offset output

    if (BITMASK_LDST_WRITEBACK(instr) == 1)
        sprintf(EOS(returnString), "!");

}

/** 
 *  DecodeMultiply() has nothing special whatsoever!
 *
 */
void DecodeMultiply(unsigned int instr, char *returnString)
{
    returnString =
        (char *) realloc(returnString, CHARS(WIDTH_INSTRUCTION));


    sprintf(returnString, "%s%s", (BITMASK_MULT_ACCUM(instr) == 0) ? "MUL" : "MLA",     // == 1
            gConditionCodes[BITMASK_CONDCODE(instr)]);

    if (BITMASK_MULT_STATUS(instr) == 1)
        sprintf(EOS(returnString), "\033[%dmS\033[0m", COLOUR_STATUSBIT);


    int top = strlene(returnString);

    for (int i = 8; i > top; i--)
        sprintf(EOS(returnString), " ");

    sprintf(EOS(returnString), "%s,",
            returnRegisterString(BITMASK_MULT_RD(instr)));



    top = strlene(returnString);
    for (int i = 16; i > top; i--)
        sprintf(EOS(returnString), " ");

    sprintf(EOS(returnString), "%s,",
            returnRegisterString(BITMASK_MULT_RN(instr)));


    top = strlene(returnString);
    for (int i = 24; i > top; i--)
        sprintf(EOS(returnString), " ");

    sprintf(EOS(returnString), "%s",
            returnRegisterString(BITMASK_MULT_RS(instr)));


    if (BITMASK_MULT_ACCUM(instr) == 1) {
        sprintf(EOS(returnString), ",");

        top = strlene(returnString);
        for (int i = 32; i > top; i--)
            sprintf(EOS(returnString), " ");

        sprintf(EOS(returnString), "%s",
                returnRegisterString(BITMASK_MULT_RM(instr)));
    }
}

/** 
 *  DecodeADR() doesn't do anything big, apart from when it sees there's no label at the offset, and can guess whether it's embedded in data, or an ADRL (which will only work when it's not outputting them by default)
 *
 */
void
DecodeADR(unsigned int instr, unsigned int currentAddress,
          char *returnString, char **returnCommentsString,
          int *returnCommentsStringCount)
{
    returnString = (char *) realloc(returnString, CHARS(64));

    sprintf(returnString, "\033[%dmADR\033[0m", COLOUR_PSEUDO);

    int top = strlene(returnString);

    for (int i = 8; i > top; i--)
        sprintf(EOS(returnString), " ");

    sprintf(EOS(returnString), "%s,",
            returnRegisterString(BITMASK_DP_RD(instr)));

    top = strlene(returnString);

    for (int i = 16; i > top; i--)
        sprintf(EOS(returnString), " ");


    int offset = currentAddress + 8;

    if (BITMASK_DP_OPCODE(instr) == 4)  // ADD
        offset += BITMASK_DP_OP2(instr);
    else if (BITMASK_DP_OPCODE(instr) == 2)     // SUB
        offset -= BITMASK_DP_OP2(instr);

    char *thisLabel = getLabelFromAddress(offset);

    const char *op2 = decodeOperand2(instr, returnCommentsString,
                                     returnCommentsStringCount,
                                     NO_COMMENTS);

    sprintf(EOS(returnString), "%s", thisLabel);
    /*sprintf( EOS(returnString), "%s", decodeOperand2(instr,   returnCommentsString, returnCommentsStringCount)); */

    returnCommentsString[*returnCommentsStringCount]
        = (char *) malloc(snprintf(NULL, 0, "%s %s, %s, %s",
                                   (BITMASK_DP_OPCODE(instr) == 4) ? "ADD"
                                   : "SUB",
                                   returnRegisterString(BITMASK_DP_RD
                                                        (instr)),
                                   returnRegisterString(BITMASK_DP_RN
                                                        (instr)),
                                   op2) + 1);
    sprintf(returnCommentsString[(*returnCommentsStringCount)++],
            "%s %s, %s, %s",
            (BITMASK_DP_OPCODE(instr) == 4) ? "ADD" : "SUB",
            returnRegisterString(BITMASK_DP_RD(instr)),
            returnRegisterString(BITMASK_DP_RN(instr)), op2);
    // ++*returnCommentsStringCount;


    if (strlen(thisLabel) > 0) {
        returnCommentsString[*returnCommentsStringCount] = (char *) malloc(snprintf(NULL, 0, "%s\033[%dm is stored at address 0x%x", getLabelFromAddress(offset), COLOUR_COMMENT,       // need this because we return the string with colours embedded - resets to \033[0m automatically
                                                                                    offset)
                                                                           +
                                                                           1);
        sprintf(returnCommentsString[*returnCommentsStringCount],
                "%s\033[%dm is stored at address 0x%x",
                getLabelFromAddress(offset), COLOUR_COMMENT, offset);
        ++*returnCommentsStringCount;

    } else {
        returnCommentsString[*returnCommentsStringCount]
            = (char *)
            malloc(snprintf
                   (NULL, 0,
                    "No label associated with the address 0x%u, therefore this could be embedded in data%s",
                    offset,
                    (gOutputADRL ==
                     1) ? "." : ", or it could be an ADRL") + 1);
        sprintf(returnCommentsString[*returnCommentsStringCount],
                "No label at 0x%u, therefore could be embedded in data%s",
                offset,
                (gOutputADRL == 1) ? "." : ", or it could be an ADRL");
        ++*returnCommentsStringCount;
    }

}

/** 
 *  DecodeADRL() is pretty self explanatory, although the code behind it can be quite hard to read.  
 * 
 *  We pass in the instructions array, the current position of our program, and the start address in memory. Then we return the string, fully decoded. Note that all checks (i.e. IS_ADRL() ) are done before the function is called.
 */
int
DecodeADRL(unsigned int *armI, int i, unsigned int startAddress,
           char *returnString, char **returnCommentsString,
           int *returnCommentsStringCount)
{

    int inADRL = 0,
        typeADRL = 0, currentOffset = 0, numInstr = 0, top = 0, j = 0;

    returnCommentsString = (char **) malloc(CHARS(1));

    while ((BITMASK_DP_OPCODE(armI[i]) == 2)
           || (BITMASK_DP_OPCODE(armI[i]) == 4)) {

        returnCommentsString[*returnCommentsStringCount]
            = (char *)
            malloc(snprintf
                   (NULL, 0,
                    "ADRL; %s %s, %s, %s",
                    (BITMASK_DP_OPCODE(armI[i]) == 4) ? "ADD" : "SUB",
                    returnRegisterString(BITMASK_DP_RD(armI[i])),
                    returnRegisterString(BITMASK_DP_RN(armI[i])),
                    decodeOperand2(armI[i],
                                   returnCommentsString,
                                   returnCommentsStringCount,
                                   NO_COMMENTS)) + 1);
        sprintf(returnCommentsString[(*returnCommentsStringCount)++],
                "ADRL; %s %s, %s, %s",
                (BITMASK_DP_OPCODE(armI[i]) ==
                 4) ? "ADD" : "SUB",
                returnRegisterString(BITMASK_DP_RD(armI[i])),
                returnRegisterString(BITMASK_DP_RN(armI[i])),
                decodeOperand2(armI[i], returnCommentsString,
                               returnCommentsStringCount, NO_COMMENTS));


        if (BITMASK_DP_RN(armI[i]) == 15) {
            // printf("ADR");
            if ((BITMASK_DP_RN(armI[i + 1]) != 15))
                inADRL = 1;

            typeADRL = (BITMASK_DP_OPCODE(armI[i]) / 2);        // SUB = 1, ADD = 2
            currentOffset =
                (BITMASK_DP_ROT(armI[i]) ==
                 0) ? BITMASK_DP_ROT_IM(armI[i]) :
                Rotate(BITMASK_DP_ROT_IM(armI[i]),
                       2 * BITMASK_DP_ROT(armI[i]));
        } else if ((BITMASK_DP_RN(armI[i]) == BITMASK_DP_RD(armI[i])
                    && BITMASK_DP_RD(armI[i]) ==
                    BITMASK_DP_RD(armI[i - 1]))
                   && (inADRL == 1)) {
            // printf("ADD/SUB");
            if (typeADRL == 2)
                currentOffset +=
                    (BITMASK_DP_ROT(armI[i]) ==
                     0) ? BITMASK_DP_ROT_IM(armI[i])
                    : Rotate(BITMASK_DP_ROT_IM(armI[i]),
                             2 * BITMASK_DP_ROT(armI[i]));
            else if (typeADRL == 1)
                currentOffset -=
                    (BITMASK_DP_ROT(armI[i]) ==
                     0) ? BITMASK_DP_ROT_IM(armI[i])
                    : Rotate(BITMASK_DP_ROT_IM(armI[i]),
                             2 * BITMASK_DP_ROT(armI[i]));
        }                       // end if R15

        numInstr++;
        i++;

    }                           // end while() - will break out if it's an else            

    int finalOffset = currentOffset + startAddress;

    if (typeADRL == 2)
        finalOffset += (i * 4); // ADD Rx, PC
    else if (typeADRL == 1)
        finalOffset -= (i * 4); // SUB Rx, PC

    char *l = getLabelFromAddress(finalOffset); // do this before realloc, otherwise memory addresses adjusted

    if (strlen(l) > 0) {
        returnCommentsString[*returnCommentsStringCount] = (char *)
            malloc(snprintf
                   (NULL, 0,
                    "%s\033[%dm is stored at address 0x%x\033[0m",
                    l, COLOUR_COMMENT, finalOffset) + 1);
        sprintf(returnCommentsString[(*returnCommentsStringCount)++],   // increment after it's used
                "%s\033[%dm is stored at address 0x%x\033[0m",
                l, COLOUR_COMMENT, finalOffset);
    } else {
        // No label found: happens if there's an ADD Rd, Rn which uses the same Rd as the ADRL. 
        returnCommentsString[*returnCommentsStringCount] = (char *)
            malloc(snprintf
                   (NULL, 0,
                    "\033[%dmERR: \033[%dmNo label found at 0x%x, please disable the ADRL function via command-line. \033[0m",
                    COLOUR_ERROR, COLOUR_COMMENT, finalOffset) + 1);
        sprintf(returnCommentsString[(*returnCommentsStringCount)++],   // increment after it's used
                "\033[%dmERR: \033[%dmNo label found at 0x%x, please disable the ADRL function via command-line. \033[0m",
                COLOUR_ERROR, COLOUR_COMMENT, finalOffset);
    }                           // end strlen

    returnString =
        (char *) realloc(returnString, CHARS(WIDTH_INSTRUCTION));

    sprintf(returnString, "\033[%dmADRL\033[0m", COLOUR_PSEUDO);

    top = strlene(returnString);
    for (j = 8; j > top; j--)
        sprintf(EOS(returnString), " ");

    sprintf(EOS(returnString), "%s,",
            returnRegisterString(BITMASK_DP_RD(armI[i])));

    top = strlene(returnString);
    for (j = 16; j > top; j--)
        sprintf(EOS(returnString), " ");

    sprintf(EOS(returnString), "%s", l);        //getLabelFromAddress(currentOffset + startAddress + i*4) );

    printf("%s", returnString);

    top = strlene(returnString);

    for (int x = 64; x > top; x--)
        printf(" ");

    if (*returnCommentsStringCount > 0) {
        printf("\033[%dm%s\033[0m", COLOUR_COMMENT,
               returnCommentsString[0]);
        int x, y;

        if (*returnCommentsStringCount - 1 >= 1) {      // increments after each usage              TODO: MAKE IT INCREMENT WHEN NEEDED????
            for (y = 1; y < *returnCommentsStringCount; y++) {
                printf("\n");
                for (x = 0;
                     x <
                     16 + 12 + ((gOutputAscii == 1) ? 6 : 0) +
                     24 + 64 + 4; x++)
                    printf(" ");
                printf("\033[90m%s\033[0m", returnCommentsString[y]);
            }
        }                       // end if
    }                           // end if commentCount > 0

    return inADRL;
}

/** 
 *  DecodeDataProcessing() is pretty self explanatory. 
 * 
 *  Only thing to do is note that we decode the pseudo-instruction NOP. 
 */
void
DecodeDataProcessing(unsigned int instr, char *returnString,
                     char **returnCommentsString,
                     int *returnCommentsStringCount)
{
    char dataProcessing[16][4] =
        { "AND", "EOR", "SUB", "RSB", "ADD", "ADC",
        "SBC", "RSC", "TST", "TEQ", "CMP", "CMN",
        "ORR", "MOV", "BIC", "MVN"
    };

    returnString =
        (char *) realloc(returnString, CHARS(WIDTH_INSTRUCTION));

    int doRn = 0, doRd = 0;

    int statusChar;

    int offset = 0, i = 0;

    // catch this before the main program, otherwise it will decode it normally
    if (BITMASK_DP_RD(instr) == 0 && BITMASK_DP_OPCODE(instr) == 13 && strcmp(decodeOperand2(instr, returnCommentsString, returnCommentsStringCount, NO_COMMENTS), "R0") == 0) {        // MOV R0, R0

        sprintf(returnString, "\033[%dmNOP\033[0m", COLOUR_PSEUDO);

        returnCommentsString[*returnCommentsStringCount] = (char *)
            malloc(snprintf(NULL, 0, "Pseudo-operator; MOV R0, R0") + 1);

        sprintf(returnCommentsString[(*returnCommentsStringCount)++],   // increment after we've used it
                "Pseudo-operator; MOV R0, R0");

        return;                 // exit so we don't carry on
    }



    switch (BITMASK_DP_OPCODE(instr)) {
        case 0:                // case "AND":
        case 1:                // case "EOR":
        case 2:                // case "SUB":
        case 3:                // case "RSB":
        case 4:                // case "ADD":
        case 5:                // case "ADC":
        case 6:                // case "SBC":
        case 7:                // case "RSC":
        case 12:               // case "ORR":
        case 14:               // case "BIC":
            doRn = 1;
            doRd = 1;
            break;

        case 8:                // case "TST":
        case 9:                // case "TEQ":
        case 10:               // case "CMP":
        case 11:               // case "CMN":
            doRn = 1;
            break;

        case 13:               // case "MOV":
        case 15:               // case "MVN":
            doRd = 1;
            break;
    }                           //// end switch

    if ((BITMASK_DP_B20(instr) == 1)    // Status Bit (20) set
        && (BITMASK_DP_OPCODE(instr) != 8)      // TST = 1000
        && (BITMASK_DP_OPCODE(instr) != 9)      // TEQ = 1001
        && (BITMASK_DP_OPCODE(instr) != 10)     // CMP = 1010
        && (BITMASK_DP_OPCODE(instr) != 11))    // CMN = 1011
        statusChar = 1;         // "S";
    else
        statusChar = 0;         // "";

    sprintf(returnString, "%s\033[%dm%s\033[0m\033[%dm%s\033[0m",
            dataProcessing[BITMASK_DP_OPCODE(instr)],
            COLOUR_CONDCODE,
            gConditionCodes[BITMASK_CONDCODE(instr)],
            COLOUR_STATUSBIT, (statusChar == 1) ? "S" : "");


    offset += 8;
    int top = strlene(returnString);

    for (i = offset; i > top; i--)
        sprintf(EOS(returnString), " ");


    if (doRd) {
        sprintf(EOS(returnString), "%s,",
                returnRegisterString(BITMASK_DP_RD(instr)));

        offset += 8;
        top = strlene(returnString);
        for (i = offset; i > top; i--)
            sprintf(EOS(returnString), " ");
    }


    if (doRn) {
        sprintf(EOS(returnString), "%s,",
                returnRegisterString(BITMASK_DP_RN(instr)));

        offset += 8;
        top = strlene(returnString);
        for (i = offset; i > top; i--)
            sprintf(EOS(returnString), " ");
    }
    // always need Op2
    sprintf(EOS(returnString), "%s",
            decodeOperand2(instr, returnCommentsString,
                           returnCommentsStringCount, SHOW_COMMENTS));
}

/** 
 *  decodeOperand2() is pretty self explanatory. 
 * 
 *  Note that we use this as a separate function as we need to know what Op2 is for ADR(L) comments, as well as data processing instructions. 
 */
const char *decodeOperand2(unsigned int instr, char **returnCommentsString,
                           int *returnCommentsStringCount, int doComments)
{
    char *returnString = (char *) malloc(CHARS(WIDTH_INSTRUCTION));

    char gShiftTypes[5][4] = { "LSL", "LSR", "ASR", "ROR", "RRX"
    };

    // sprintf(returnString, "");
    strcpy(returnString, "");

    if (BITMASK_DP_B25(instr) == 1) {   // ROTATE

        if (BITMASK_DP_ROT(instr) == 0) {
            sprintf(EOS(returnString), "#%u", BITMASK_DP_ROT_IM(instr));

            if (doComments == 1) {

                if (isprint(BITMASK_DP_ROT_IM(instr))
                    || BITMASK_DP_ROT_IM(instr) == 32) {        // if it's a printable character, which a space apparently isn't

                    returnCommentsString[*returnCommentsStringCount] =
                        (char *)
                        malloc(snprintf
                               (NULL, 0,
                                "\033[%dm%d = 0x%1$x  =  '%1$c'\033[0m",
                                COLOUR_COMMENT,
                                BITMASK_DP_ROT_IM(instr)) + 1);
                    sprintf(returnCommentsString[(*returnCommentsStringCount)++],       // increment after we've used it
                            "\033[%dm%d = 0x%1$x  =  '%1$c'\033[0m",
                            COLOUR_COMMENT, BITMASK_DP_ROT_IM(instr));


                } else if (BITMASK_DP_ROT_IM(instr) == 10 || BITMASK_DP_ROT_IM(instr) == 13) {  // or output a nice newline if we have \r or \n (keep it as \n for nicety)

                    returnCommentsString[*returnCommentsStringCount] =
                        (char *)
                        malloc(snprintf
                               (NULL, 0,
                                "\033[%dm%d = 0x%1$x  =  '\\n'\033[0m",
                                COLOUR_COMMENT,
                                BITMASK_DP_ROT_IM(instr)) + 1);
                    sprintf(returnCommentsString[(*returnCommentsStringCount)++],       // increment after we've used it
                            "\033[%dm%d = 0x%1$x  =  '\\n'\033[0m",
                            COLOUR_COMMENT, BITMASK_DP_ROT_IM(instr));

                }
            } else {

                sprintf(EOS(returnString), " = #0x%x",
                        BITMASK_DP_ROT_IM(instr));

            }                   // end doComments
        } else {                // ROT!= 0
            sprintf(EOS(returnString), "#%u", Rotate(BITMASK_DP_ROT_IM(instr), 2 * BITMASK_DP_ROT(instr)));     // double num. places to rotate, then do it
            if (doComments == 1 && Rotate(BITMASK_DP_ROT_IM(instr), 2 * BITMASK_DP_ROT(instr)) > 9) {   // only show bigger numbers, these are obvious
                returnCommentsString[*returnCommentsStringCount] =
                    (char *) malloc(snprintf(NULL, 0, "%d = 0x%1$x",
                                             Rotate(BITMASK_DP_ROT_IM
                                                    (instr),
                                                    2 *
                                                    BITMASK_DP_ROT(instr)))
                                    + 1);
                sprintf(returnCommentsString[(*returnCommentsStringCount)++],   // increment after we've used it
                        "%d = 0x%1$x",
                        Rotate(BITMASK_DP_ROT_IM(instr),
                               2 * BITMASK_DP_ROT(instr)));
            } else {
                sprintf(EOS(returnString), " = #0x%x",
                        Rotate(BITMASK_DP_ROT_IM(instr),
                               2 * BITMASK_DP_ROT(instr)));
            }
        }                       // end ROT

    } else if (BITMASK_DP_B25(instr) == 0) {
        // SHIFT
        if (BITMASK_DP_SH_IMME(instr) == 0) {

            sprintf(EOS(returnString), "%s%s",
                    returnRegisterString(BITMASK_DP_SH_RM(instr)),
                    (BITMASK_DP_SH_TYPE(instr) == 3) ? ", RRX" : "");

        } else {                // if(BITMASK_DP_SH_IMME(instr) == 1){

            sprintf(EOS(returnString), "%s, %s ",
                    returnRegisterString(BITMASK_DP_SH_RM(instr)),
                    gShiftTypes[BITMASK_DP_SH_TYPE(instr)]);

            if (BITMASK_DP_B4(instr) == 1) {
                sprintf(EOS(returnString), "%s",
                        returnRegisterString(BITMASK_DP_RS(instr)));
            } else if (BITMASK_DP_B4(instr) == 0) {
                sprintf(EOS(returnString), "#%u",
                        BITMASK_DP_SH_IMME(instr));

                if (doComments == 1 && BITMASK_DP_SH_IMME(instr) > 9) { // only show bigger numbers, these are obvious) {


                    if (BITMASK_DP_SH_IMME(instr) == 10
                        || BITMASK_DP_SH_IMME(instr) == 13) {
                        returnCommentsString[*returnCommentsStringCount] =
                            (char *)
                            malloc(snprintf
                                   (NULL, 0,
                                    "\033[%dm%d = 0x%1$x  =  '\\n'\033[0m",
                                    COLOUR_COMMENT,
                                    BITMASK_DP_SH_IMME(instr)) + 1);
                        sprintf(returnCommentsString[(*returnCommentsStringCount)++],   // increment after we've used it
                                "\033[%dm%d = 0x%1$x  =  '\\n'\033[0m",
                                COLOUR_COMMENT, BITMASK_DP_SH_IMME(instr));
                    } else {
                        returnCommentsString[*returnCommentsStringCount] =
                            (char *)
                            malloc(snprintf
                                   (NULL, 0, "%d = 0x%1$x",
                                    BITMASK_DP_SH_IMME(instr))
                                   + 1);
                        sprintf(returnCommentsString[(*returnCommentsStringCount)++],   // increment after we've used it
                                "%d = 0x%1$x", BITMASK_DP_SH_IMME(instr));
                    }








                }
            }                   // end if == 0
        }                       // end immed
    }                           // end B25
    return returnString;
}

/** 
 *  Used for Disassemble() 
 */

/** 
 *  decodeASCII() is used when we've got gOutputAscii set (the -a flag). This basically works out the characters that an instruction may be using. 
 * 
 *  This function is mainly used to help determine where data is stored, showing characters that *could* be a string, or just may be random characters. 
 */
void decodeASCII(unsigned int instr, char *returnString)
{
    int thisChar, x;            // for loop

    int bM[4];                  // bitmasks

    int sh[4];                  // respective shifts

    sprintf(returnString, "|");

    // We use arrays for easy indexing

    if (gEndianism == 0) {      // Little-Endian, RTL
        bM[3] = 0xFF000000;
        bM[2] = 0x00FF0000;
        bM[1] = 0x0000FF00;
        bM[0] = 0x000000FF;

        sh[3] = 24;
        sh[2] = 16;
        sh[1] = 8;
        sh[0] = 0;
    } else if (gEndianism == 1) {       // Big-Endian, LTR
        bM[0] = 0xFF000000;
        bM[1] = 0x00FF0000;
        bM[2] = 0x0000FF00;
        bM[3] = 0x000000FF;

        sh[0] = 24;
        sh[1] = 16;
        sh[2] = 8;
        sh[3] = 0;
    }

    for (x = 0; x < 4; x++) {

        thisChar = (instr & bM[x]) >> sh[x];    // easy way to use indexing to do what we want - makes it easy to change bitmasks as well

        if (isprint(thisChar))  // only output characters we can see - i.e. not control codes!
            sprintf(EOS(returnString), "%c", thisChar);
        else
            sprintf(EOS(returnString), ".");
    }

    sprintf(returnString, "%s|", returnString);
}

/**
 *  Used in the decoder functions
 */


/***************************************************************************************************************************************************
    Some ARM instructions (e.g. the branches) encode a two’s-complement number in less than 32-
    bits. This means that once extracted from the ARM instruction C won’t treat it as negative number (when approrpriate). This function:
          int SignExtend(unsigned int x, int bits)
    Extends the two’s complement number {x} to fill all 32-bits of an int. To use it, extract the bits
    from the instruction and pass them as the parameter {x}. Also, pass the number of bits used to
    store the number, e.g. for a branch, 26. The return value would be a signed int that can be used
    elsewhere in your program, (e.g. to pass to printf()).
*/
int SignExtend(unsigned int x, int bits)
{
    int r;

    int m = 1U << (bits - 1);

    x = x & ((1U << bits) - 1);
    r = (x ^ m) - m;
    return r;
}

/***************************************************************************************************************************************************
    Some instructions (e.g. the data-processing instructions) encode a number as a set of 8 bits and a
    number of bits to rotate them to the right. This function:
          int Rotate(int rotatee, int amount)
    will rotate the parameter {rotatee} to the right by {amount} bits and return the results.  
*/
int Rotate(unsigned int rotatee, int amount)
{
    unsigned int mask, lo, hi;

    mask = (1 << amount) - 1;
    lo = rotatee & mask;
    hi = rotatee >> amount;

    rotatee = (lo << (32 - amount)) | hi;

    return rotatee;
}

/** 
 *  returnRegisterString() returns a nicer register name, such as PC instead of R15. 
 * 
 *  We use this to make the code look a little nicer, and to ensure that there is a bit more help when deciphering ARM instructions. i.e. if you don't know off by heart R13 is SP
 */

const char *returnRegisterString(unsigned int registerValue)
{
    char *returnString = (char *) malloc(CHARS(3));     //HACK: we malloc() but *don't* free the memory as if we do, the program won't be able to access it at a later point - loads of memory trouble

    switch (registerValue) {
        case 11:               // FP
            return "FP";
            break;

        case 13:               // SP
            return "SP";
            break;

        case 14:               // LR
            return "LR";
            break;

        case 15:               // PC
            return "PC";
            break;

        default:               // R0-10, R12
            if (registerValue > 15)
                return "";
            sprintf(returnString, "R%u", registerValue);
            return returnString;
            break;


    }
}

/** 
 *  getLabelFromAddress() is a function to find the label associated with a piece of code. This is done by referencing the symbol table for the address, which then calls on the function getStringNameFromSymName() to get the actual name. 
 * 
 *  We perform this function to make the code a lot closer to the ARM instructions inputted. We will see the actual labels, rather than either a PC-relative, or absolute, memory address. 
 */
char *getLabelFromAddress(int address)
{
    FILE *fp;

    ELFSYMTABLE *symTabl;

    int x;

    const char *p;

    fp = fopen(gFilename, "rb");
    fseek(fp, gSymbolTableOffset, SEEK_SET);
    symTabl = (ELFSYMTABLE *) malloc(sizeof(ELFSYMTABLE));
    if (!symTabl)
        memoryError(fp);

    for (x = 0; x <= gStringTableCount; x++) {
        fread(symTabl, 1, sizeof(ELFSYMTABLE), fp);
        if (x == 0)
            continue;

        if (symTabl[0].value == address && symTabl[0].shndx != SHNDX_EQU) {     // don't let EQUs print, as they're matched from the address, despite technically not existing
            p = getStringNameFromSymName(symTabl[0].name, TABLE_STR);

            char *ret = (char *) malloc(sizeof(char) * (strlen(p) + strlen("\033[94m\033[0m") + 1));    // need to allocate memory, otherwise we're pointing to nothing (after the function ends, all the memory is wiped/overwritten)

            sprintf(ret, "\033[%dm", COLOUR_LABEL);
            strcat(ret, p);
            strcat(ret, "\033[0m");

            free(symTabl);      // need to close and free before we go on with the program - otherwise get segfaults
            fclose(fp);

            return ret;
        }
    }
    // if we've not found anything, close up 
    free(symTabl);
    fclose(fp);

    return "";                  // and then output a blank string
}

/** 
 *  strlene() is a function to calculate the string length of a given {char *string}, excluding the escape codes.  
 * 
 *   This is used as we deal with a lot of colour codes in this program, therefore when trying to output the 
 *   formatted columns, this causes errors as all the escape code characters are counted. 
 */
int strlene(const char *string)
{
    int count = 0;

    for (int i = 0; i < strlen(string); i++) {
        if (string[i] == '\033')        // skip the escape codes
            while (string[i] != 'm')
                i++;            // an 'm' symbolises the end of a escape code
        else
            count++;            // if it's not one, increment our count
    }
    return count;
}
