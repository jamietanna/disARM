; Passcode character program

	B main

main	ADR	R0, hello	; prompt for the passcode character;
	SWI 	3
getch	SWI	1		; input a character to R0
	CMP	R0, #42 	; compare input character to * 
	BNE	skip
	ADR	R0, welcome	; print the welcome message
	SWI 	3
	B exit
skip	ADR	R0, goodbye 	;   printf("Passcode incorrect -- good-bye!");
	SWI	3
exit	SWI  	2		;   stop the program
hello	DEFB	"Hello - please type the passcode character \0"
goodbye	DEFB	"\nPasscode incorrect -- good-bye!\n\0"
welcome	DEFB	"\nWelcome to the Komodo Dragon's lair !!\n\0"
	ALIGN

