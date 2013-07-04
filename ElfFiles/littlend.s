; Hello World Version 1

	B main

main	ADR R0, hello1	; get the start address of the string
	SWI 3		; print the message
	MOV R0, #10	; newline character
	SWI 0
	ADR R0, hello2
	SWI 3		; put out the defined integer as if it was a string
	ADR R0,mess1
	SWI 3		; put out the long message
	SWI 2		; stop the program

hello1	DEFB	"Hi!\0"
hello2  DEFW	0x00216948
mess1	DEFB	"\nThis is a very long message\n\0"
	ALIGN

