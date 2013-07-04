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
; end of strchr

main
                ADR R5, plain
                ADR R1, cipher    
again		    SWI 1			; get char into R0
                CMP R0, #10		; is it a newline?
                SWIEQ 2
                MOV R2, R0
                BL strchr
                CMP R0, #-1    ; if -1 returned then use original char in R2 else use R0 to index plain
                MOVEQ R0, R2
                LDRNEB R0, [R5,R0]  ; else use index to get corresponding plaintext char
                SWI  0              ; put out character in R0
                B again

            plain  DEFB "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ\0"
            cipher DEFB "tuJKjkNOehSTWXYZabABcdiUVnopqLMxyzvwClmDEFfgGHIrsPQR\0"
