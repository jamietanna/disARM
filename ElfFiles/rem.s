            B main
xval        EQU     259
yval        EQU     111
strBadInput DEFB    "Error, bad input to [rem]\0"
            ALIGN 

rem                         ; modulus(R4,R5) returns R3
    CMP     R4, R5
    CMPGE   R5, #0      ; cut down another branch
    BLT     badinput
    CMP     R4, #0
    BLE     badinput
    
    MOV     R3, R4
  remwhile
    CMP     R3, #0      ; need to make it so it only subtracts if bigger by 'divisor' 
    SUBGTS  R3, R3, R5
    BGT     remwhile 

    MOVEQ   PC, LR        ; if (x==0) return (x);
    ADD     R3, R3, R5
    MOV     PC, LR          ; else return (y + x);
    
badinput
    ADR     R0, strBadInput
    SWI     3
    MOV     R0, #32 
    SWI     0
    MOV     R0, R4
    SWI     4
    MOV     R0, #32 
    SWI     0
    MOV     R0, R5
    SWI     4
    SWI     2

main
    LDR     R4, =xval 
    LDR     R5, =yval
 
    BL      rem

    MOV     R0, R4
    SWI     4
    MOV     R0, #'%'
    SWI     0
    MOV     R0, R5
    SWI     4
    MOV     R0, #'='
    SWI     0
    MOV     R0, R3
    SWI     4
    MOV     R0, #10
    SWI     0

    SWI     2
