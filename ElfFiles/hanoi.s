hanoi
                                        ; R0 holds current char
                                        ; R1 holds current 'order'
                                        ; R2 always points to letters

    LDRB    R0,  [R2, R1]               ; char thisch = diskletters[order];
    LDRB    R0,  [R2, R1, LSL #3]               ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    CMP     R1,  #1                     ; when we've hit the base case, 
    SWIEQ   0                           ;  output the character, 
    MOVEQ   PC,  LR                     ;  and then we return back up the stack

    STMFD   SP!, {R1,R0,LR}             ; hanoi(order-1)
    SUB     R1,  R1, #1
    BL      hanoi

    LDMFD   SP,  {R1, R0}               
                                        ; don't update SP so we can pop it again later from the same place, or bother to pop LR, as we're not gonna use it
    SWI     0
    
    SUB     R1,  R1, #1                 ; hanoi(order-1)
    BL      hanoi
    LDMFD   SP!, {R1,R0,PC}             ; and finally return to whence we came by sneakily loading LR into PC

main
    MOV     SP,  #stack                 
                                        ; start off our stack pointer, so it knows where to send data
    ADR     R2,  letters                 
                                        ; initialise the start pointer to the "array" letters
    ADR     R0,  message
    SWI     3
    MOV     R1,  #order                  ; set the order from the constant
    MOV     R0,  R1
    SWI     4
    ADR     R0,  spacer
    SWI     3
    BL      hanoi
    MOV     R0,  #10                     ; beautify with a newline
    SWI     0
    SWI     2

            B main
stack   EQU      0x1000
order   EQU      4
message DEFB     "Here is the sequence for hanoi(\0"
spacer  DEFB     "):   \0"
letters DEFB     " ABCDE\0"
        ALIGN

