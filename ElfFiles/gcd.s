badinput
        ADR     R0, strBadInput
        SWI     3
        SWI     2        
        
rem     
        SUBS    R0, R0, R1
        BGT     rem
        ADDLT   R0, R0, R1              
                                        ; no point doing a CMP before to check if R3>R4, inefficient in the grand scale of things
        MOV     PC, LR

gcd        
        STMFD   SP!, {LR}             
                                        ; only store this the first time we enter the function, as then we don't need a load of MOV PC, LR etc to return to the main code, it's a single one
gcdloop
        BL      rem
        MOV R3, R0
        MOV R0, R1                      ; value of (y)
        MOV R1, R3                      ; value of (x % y)
        BNE     gcdloop                 
                                        ; keep going if we have (R3!=0) from remloop
        LDMFD   SP!, {PC}               
                                        ; finally return to the main program body
main                                    
                                        ; code can be done in 8 less cycles without the nicer output
        MOV     SP, #stack    
        ADR     R0, result1
        SWI     3                       ; put out the string 
        LDR     R0, =input1
        SWI     4
        MOV     R0, #','
        SWI     0
        LDR     R0, =input2
        SWI     4
        
        MOV     R1, R0                  ; backup so don't need to do another LDR
        
        ADR     R0, result2
        SWI     3                       ; put out the string 
        
        LDR     R0, =input1 ;;;;;;;;;;;;;;;; why = not # ??? - large numbs
                                        ; only check once, instead of after each execution
        CMP     R0, R1
        CMPGE   R0, #0                  ; cut down another branch
        BLT     badinput
        CMP     R1, #0
        BLE     badinput
        
        BL      gcd
        SWI     4                       ; gcd's result is returned in R0
        MOV     R0, #10
        SWI     0
        SWI     2
                B main
stack           EQU         0x1000
input1          EQU         259;91
input2          EQU         111;26
result1         DEFB        "GCD(\0"
result2         DEFB        ") = \0"
strBadInput     DEFB        "Error, bad input to [rem]\0"
                ALIGN

