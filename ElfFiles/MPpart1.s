; Hello Someone program - version 1

	B main
hello	DEFB	"Hello \0"
goodbye	DEFB	"and good-bye!\n\0"
	ALIGN

main	ADR	R0, hello	; printf("Hello ");
	SWI 	3

	; while (R0 != 10) {/* translate this into ARM code*/
loop	cmp R0, #10
	SWINE	1		; input a character to R0
	SWINE	0		; output the character in R0
	BNE loop
	; }/* translate to ARM code */

end	ADR	R0, goodbye 	; System.out.println("and good-bye!");
	SWI	3

	SWI  	2		; stop the program
