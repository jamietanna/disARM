main

    ;;;;;;;;;;;;; DATAPROCESSING

ADDEQ   R0,     R1,     #120
SUBNE   R2,     R1,     #1000
RSBGE   R1,     R2,     R2

ANDGT   R1,     R1,     R2
ORRLT   R3,     R4,     R5
EORLE   R4,     R5,     R6
BICHI   R5,     R6,     R7

MOVLS   R6,     #65536 
MVN     R7,     LR

CMPMI   R8,     #42
CMNPL   R9,     #128

TSTVS   R10,    R11
TEQVC   R4,     R5

MULAL   R0,     R1,     R5
MLAS     R1,     R0,     R7,     R3

; LOGICAL SHIFTS
; COND CODES

    ;;;;;;;;;;; LOAD/STORE

LDR     R0,     aval
LDR		R1, 	=1000
LDR     R2,     [R0, R1, LSL #3]
LDRB    R0,     [R0, #10]
LDRB    R0,     [R0], #10

STR     R5,     [R8], #1
STRB    R7,     [R0, R1]!

LDMPLFD   SP!,  {R0, R1, R3, R4, R6, R7, R9, R10, R12, R13, R15}


LDMEQFD   SP!,    {R0, R5, R8, PC, LR, R1}
LDMALFA   R0,     {R1, SP, PC}
LDMNEED   SP!,    {R0, R5, R8, PC, LR, R1}
LDMPLEA   R0,     {R1, SP, PC}

STMFD   SP!,    {R0, R5, R8, PC, LR, R1}
STMFA   R0,     {R1, SP, PC}
STMED   SP!,    {R0, R5, R8, PC, LR, R1}
STMEA   R0,     {R1, SP, PC}

    ;;;;;;;;;; BRANCH
BL main
    ;;;;;;;;;; SWI
SWI     0
SWI     1
SWI     2
SWI     3
SWI     4
   
   ;;;;;;;;; PSEUDO

MOV     R6,     #-1
ADR     R0,     bval
ADRL    R0,     cval
;LDR     R0,     =2012
NOP
    ;;;;;;;;; ROT/SH
    
ADD     R0,     R1,     R5, LSL #3
ADD     R1,     R0,     R6, LSR R8
RSB     R0,     R1,     R4, ROR #3
RSB     R0,     R1,     R4, RRX  ; should be RRX
SUB     R4,     R2,     R1, ASR PC










        B main
aval    DEFW    2000
bval    DEFB    "This is a string\0"
        ALIGN
        
        ORIGIN 0x2000
cval    DEFB    "This string is ages away :O\0"
        ALIGN
        
        ORIGIN 0x3000
        ALIGN
        ORIGIN 0x4000
        ENTRY
MOV     R0,     #10
SWI     0
ADRL    R0, cval
SWI     3
SWI     2
