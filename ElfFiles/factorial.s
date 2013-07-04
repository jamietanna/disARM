stack EQU	0x1000
input EQU	6
B main

factorial
		CMP R1, #0
		MOVEQ R1, #1
		BEQ exit		; base case -- no need to create a new frame	
		STMFD SP!, {R1, LR}
		SUB R1, R1, #1
		BL factorial
		LDMFD SP!, {R1,LR}	; restore R1 and LR -- note, back to LR not PC
exit		MUL R0, R0, R1		; answer progressively builds up in R0
		MOV PC, LR
main
		MOV R1, #input
		MOV SP, #stack	
		MOV R0, R1
		SWI 4
		ADR R0, result
		SWI 3
		MOV R0, #1
		BL factorial
		SWI 4
		SWI 2

result DEFB	" factorial is \0"
ALIGN
