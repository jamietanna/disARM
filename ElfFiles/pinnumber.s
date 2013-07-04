stack EQU	0x1000
B main
strchr	
                MOV R0, #-1     ; initialise position index
                MOV R4, #0      ; initialise index counter
load            LDRB R3, [R1,R4]
                CMP R3, R2
                MOVEQ R0, R4
                MOVEQ PC, LR                
                CMP R3, #0
                ADDNE R4, R4, #1
                BNE load
                MOV PC, LR
; end of strchr. This is a leaf function but R4 is used so take care

func1
                STMFD SP!, {R4-R8, LR}
                    ; strchr trashes R4 and lots of stuff to be added here 
                    ; later that may well trash R5-R8 which APCS says we must save
                    ; now get ready to call strchr 
                    ; R1-3 untouched so should be OK
                BL strchr
                ADD R0, R0, R5
                LDMFD SP!, {R4-R8,PC}	; restore R4-R8 and return result in R0


main
                ADR R0, mesg2    
                SWI 3
                SWI 1		; get the character from keyboard
                MOV R2, R0	;seed char now in R2
                ADR R1, mesg1
                ADR R0, mesg3
                SWI 3		; your pin number is
                LDR R5, =2010
                MOV SP, #stack	
                BL func1
                SWI 4
                SWI 2

mesg1 DEFB 	"tuJKjkNOehSTWXYZabABcdiUVnopqLMxyzvwClmDEFfgGHIrsPQR\0"
mesg2 DEFB 	"Key in a single character: \0"
mesg3 DEFB 	"\nOK your pin number is: \0"
ALIGN

