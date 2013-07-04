; comment how to use

    B main
C   DEFB    "ASDVFGJHBGTRR198325342/L()8,.HNGHFCXXERHLKOPOOMNGDDXSA94625120\0"
K   DEFB    "6<3WF>%=B/5$71SM@F5WQQF<@LJIJH7(=4C>17!<L=*\"!. C$-40639YSAFPUU\0"

K0  DEFB    "HELLO WORLD GOODBYE UNIVERSE! THE HIGGS BOSON IS VERY ELUSIVE !!\0"
C0  DEFB     "<-)L-A\"+=8DC(+*D+*EAU[D>*>6ERY'< MH$2$; 7<6+NF&! 3$ 5YE/<#!37S!\0"

K1  DEFB    "K9U&T087+318!MNVLBE&^NWEQ4*%<>+&!$(%%CSDGHIPQWERZ434/ML05VGSA3\0"
C1  DEFB     ",V!&;EL7DU1LN: V#,EG^,8$%4LJN>_ND$[JP7;!5&I9\"$<>G3<L>\"I5G^DU:\0"

C2  DEFB    "?N:&7_TXX@DK! /5$++C-N:*'QN%ZLDK!FD@Q ;(\"1I :YE&54TWG<LY[VVJU&[\0"

    ALIGN

lorenz    ; performs a while() loop

    LDRB    R3, [R1], #1       ; p++
    LDRB    R4, [R2], #1       ; q++

    CMP     R3, #0
    CMPNE   R4, #0
    MOVEQ   PC, LR
    
    SUB     R3, R3, #32        ; (*p - 32)
    SUB     R4, R4, #32        ; (*q - 32)
    
    EOR     R0, R3, R4         ; ((*p - 32)  ^ (*q - 32))
    ADD     R0, R0, #32        ; ((*p - 32)  ^ (*q - 32)) + 32;

    SWI     0
   
    B       lorenz 

main    

    ADRL    R1, K
    ADRL    R2, C
 
    BL      lorenz

    MOV     R0, #10
    SWI     0

;;;;;;;;;;;;;;;;; KC0

    ADRL    R1, K0
    ADRL    R2, C0

    BL      lorenz
    
    MOV     R0, #10
    SWI     0

;;;;;;;;;;;;;;;;;; KC1

    ADRL    R1, K1
    ADRL    R2, C1

    BL      lorenz
    
    MOV     R0, #10
    SWI     0

;;;;;;;;;;;;;;;;;; C2

    ADRL    R1, K1
    ADRL    R2, C2

    BL      lorenz

    MOV     R0, #10
    SWI     0

    SWI     2
