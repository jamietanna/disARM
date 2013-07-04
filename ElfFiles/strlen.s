; Framework for strlen question

main	ADR R0, text1	; get address of starting byte of string
	MOV R2, #0	; Initialise the counter in Register 2 
again	LDRB R1, [R0]	; this loads into r1 the byte whose address is currently in R0
			; Replace this comment with your own code to check that the contents 
			; of R1 are not the null byte. If R1 is not null then you need to 
			; increment both  the count and the byte address and branch back to 
			; label `again'. If the null byte HAS been encountered then print out
			;  the answer in the form stipulated by the question.

; you'll need a few more string declarations here to get the answer `looking right'.
text1 	DEFB	"Hello World. Goodbye Universe\0"
text2 	DEFB	"1234567890 are the ten decimal digits\0"
nulltxt	DEFB	"\0"
	ALIGN

