	AREA CAESAR, CODE, READONLY

SWI_WriteC 	EQU &0 	; System call to output a character in R0
SWI_Write0 	EQU &2 	; System call to output a string until a null terminator
SWI_ReadC 	EQU &4 	; System call to read a character from the keyboard and store it in R0
SWI_Exit 	EQU &11 ; System call to EXIT the program
SWI_Clock 	EQU &61 ; System call to obtain the number of clock ticks since the system was started
SWI_Time 	EQU &63 ; System call to obtain the current time

		ENTRY
;r1：for adr 
;r2：@ and counter of delay
;r3：" " and delay time
;r4：counter of move
;r5：number of move 
;r6：number of space with SCR1 
;r7：counter of delay
;r8：number of space with SCR2 
;r9：counter of delay
;r10：crash detect for adr
;r11：crash detect for reg
;r12：reg of r6

START	BL ZERO
		BL DISP
		MOV r2,#&40		;@
		MOV r3,#" "		;" "
		MOV r7,#&41		;B
		MOV r6,#30		;initial r6
		MOV r8,#20		;initial r8
LOOP	
		ADD r5,r5,#1	;global time or score
		
		BL CRASH		;crash detect
		
		ADRL r1,SCR1
		BL SPACE		;add obstable
		BL LEFT			;move obstable
		BL RESET		;detect r6 r8 is 0?
		SUB r6,r6,#1	;SCR1 left move 1 
		
		MOV r12,r6		
		MOV r6,r8		
		
		ADRL r1,SCR2	;second road is SCR2
		BL SPACE		;add obstable
		BL LEFT			;move obstable
		BL RESET		;detect r6 r8 is 0?
		SUB r8,r8,#1	;SCR2 left move 1 

		BL OP			;input from keyboard		
		BL OUTH			;print score
		BL RESRC2		;print all screen
		
		BL MODE			;select game mode
		BL DELAY		;delay
		
		CMP r5,#300		

		MOV r7,#&41		
		MOV r6,r12
		MOV r2,#&40		;@
		MOV r3,#" "		
		BLT LOOP		
EXIT2	SWI SWI_Exit
;===================================================================================
MODE	CMP r5,#300
		BEQ MODE3
		CMP r5,#200
		BEQ MODE2
		CMP r5,#98
		BEQ MODE1
		MOV pc,r14
MODE1	MOV r8,#39
		MOV r3,#50
		MOV pc,r14
MODE2	MOV r8,#39
		MOV r13,#10
		MOV pc,r14
MODE3	B REWIN		
;-----------------------------------------------------------------------------------
OUTH
		MOV r2,#0 		;Initialize counter in R2 to 0
		MOV r4,#10
		MOV r0,r5
NXT 	MOV r1,#0 		;Initialize quotient in R1 to 0
DIV0 	CMP r0,r4 		;Compare R0 with R4
		BLT DIV1 		;If R0 < R4, go to DIV1
		SUB r0,r0,r4 	;Subtract R4 from R0
		ADD r1,r1,#1 	;Increment quotient in R1 for each division
		B DIV0 			;Go back to DIV0 to continue dividing
;"Change number to ASCII code, 09, AF-----------------------------------------------------------"
DIV1 	CMP r0,#10 		;Compare R0 with 10
		ADD r0,r0,#&30 	;Add ASCII code for 0 to convert remainder to ASCII character
		ADDGE r0,r0,#7 	;If remainder is greater than or equal to 10, add 7 to convert remainder to ASCII character from A to F
		STMFD r13!,{r0}	;Store ASCII character from r0 to stack
		MOV r0,r1 		;Move quotient in R1 to R0
		CMP r1,#0 		;Compare quotient in R1 with 0
		ADD r2,r2,#1 	;Increment counter in R2 for each digit converted to ASCII character
		BNE NXT 		;If quotient is not equal to 0, go back to NXT to continue dividing
;"Output to the screen--------------------------------------------------------------------------"
NXT1 	CMP r2,#0 		;Compare counter in R2 with 0
		BEQ CPLT1 		;If counter in R2 is equal to 0, go to CPLT to return to main program
		LDMFD r13!,{r0} ;Load ASCII character from stack to R0
		SWI SWI_WriteC 	;Output ASCII character to screen
		SUB r2,r2,#1 	;Decrement counter in R2 for each digit output to screen
		B NXT1 			;Go back to NXT1 to output remaining digits
;"-----------------------------------------------------------------------------------------------"
CPLT1	ADRL r0,SCORE
		SWI SWI_Write0
CPLT 	MOV pc,r14 		;Return to main program
;===================================================================================
CRASH   CMP r6,#0
		BEQ CRASH1
CRASH8	CMP r8,#0
		BEQ CRASH2
		B	ENDCR
CRASH1	ADRL r10,SCR1
		LDRB r11,[r10]
		CMP  r11,r2
		BEQ  RENON
		B	CRASH8
CRASH2	ADRL r10,SCR2
		LDRB r11,[r10]
		CMP  r11,r2
		BEQ  RENON
ENDCR	MOV pc,r14
;===================================================================================
RESET	CMP r8,#0
		BEQ	RER8
LOOPR	CMP r6,#0		
		BEQ	RER6
		B 	ENDR
RER6	MOV r6,#3		;CAN'T CHANGE R6 	
ENDR	MOV pc,r14
RER8	MOV r8,#19		;initial r8
		B LOOPR
;===================================================================================
OP		SWI	SWI_ReadC
		CMP r0,#&57 	;Compare W
		BEQ	KEYW
		CMP r0,#&53 	;Compare S
		BEQ	KEYS
		CMP r0,#&51 	;Compare Q
		BEQ	KEYQ		
KEYS	ADRL r1,SCR1
		STRB r3, [r1], #1	
		ADRL r1,SCR2
		STRB r2, [r1], #1	;add@
		B	ENDK
KEYW	ADRL r1,SCR2
		STRB r3, [r1], #1
		ADRL r1,SCR1
		STRB r2, [r1], #1	;add@	
ENDK	MOV pc,r14
KEYQ	B RENON
;===================================================================================		
MOVE 	
		MOV r4,#0		
MOVE1	STRB r3, [r1], #1	;add" "
		ADD r4,r4,#1
		CMP r4,r5
		BNE MOVE1
		STRB r2, [r1], #1	;add@
		MOV pc,r14
;===================================================================================
SPACE	MOV r4,#0
		CMP r6,#0
		BEQ RESPA
SPACEL	STRB r3, [r1], #1
		ADD r4,r4,#1
		CMP r4,r6
		BNE SPACEL
		MOV r7,#&42
		STRB r7, [r1], #1
		MOV pc,r14			;RESPA  can reset r6 when r6==0
RESPA	MOV r6,#17			;initial r6 
		B	SPACE
;===================================================================================
LEFT	CMP r6,#0
		BEQ LEFTZ
LEFTZ	MOV r4,r6
LEFTL	STRB r3, [r1], #1
		CMP r4,#0
		BEQ LEFTX
		SUB	r4,r4,#1
		BNE LEFTL
LEFTX	MOV pc,r14
;===================================================================================
;random control. for.   SCR1~6
;===================================================================================				
RESRC	ADR r0,SCRTOP
		SWI SWI_Write0
		MOV pc,r14
;===================================================================================
RENON	ADR r0,NONTOP
		SWI SWI_Write0
		B EXIT2
		MOV pc,r14
;===================================================================================
REWIN	ADRL r0,WINTOP
		SWI SWI_Write0
		B EXIT2
		MOV pc,r14
;===================================================================================		
DELAY	MOV r9,#0
		MOV r3,#500
		CMP r5,#200
		BGT	FAST2			
		CMP r5,#100
		BGT	FAST1
DELAY1	MOV r2,#0		
		MOV r7,#0
		ADD r9,r9,#1
		CMP r9,R3
		BEQ DEEXIT
DELAY3	ADD r2,r2,#1
		CMP r2,r3
		BNE DELAY3
DELAY2	ADD r7,r7,#1	
		CMP r7,r3
		BNE DELAY2
		BEQ DELAY1		
DEEXIT	MOV pc,r14
FAST1	MOV r3,#100
		B	DELAY1
FAST2	MOV r3,#1
		B	DELAY1
;===================================================================================
RESRC2	ADRL r0,SCRTOP
		SWI SWI_Write0
		ADRL r0,SCR1
		SWI SWI_Write0
		ADRL r0,SCR2
		SWI SWI_Write0
		ADRL r0,SCREND
		SWI SWI_Write0
		MOV pc,r14
;===================================================================================
ZERO	AND	R0,R0,#0
		AND	R1,R1,#0
		AND	R2,R2,#0
		AND	R3,R3,#0
		AND	R4,R4,#0
		AND	R5,R5,#0
		AND	R6,R6,#0
		AND	R7,R7,#0
		AND	R8,R8,#0
		AND	R9,R9,#0
		AND	R10,R10,#0
		MOV	PC,R14
;====================================================================================
DISP	ADRL r0,SIGN0
		SWI SWI_Write0
		MOV	PC,R14
;====================================================================================	
SCRTOP  = "========================================",&0a,&0d,0
SCR1  	= "                                                                                ",&0a,&0d,0
SCR2  	= "                                                                                ",&0a,&0d,0
SCR3  	= "                                                                                ",&0a,&0d,0
SCR4  	= "                                                                                ",&0a,&0d,0
SCR5  	= "                                                                                ",&0a,&0d,0
SCR6  	= "                                                                                ",&0a,&0d,0
SCREND  = "========================================",&0a,&0d,0
	ALIGN
NONTOP  = "GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG",&0a,&0d
NON1  	= "                                         ",&0a,&0d
NON2  	= "                                         ",&0a,&0d
NON3  	= "                 GAME OVER               ",&0a,&0d
NON4  	= "                                         ",&0a,&0d
NON5  	= "                                         ",&0a,&0d
NON6  	= "                                         ",&0a,&0d
NONEND  = "GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG",&0a,&0d,0
	ALIGN
WINTOP  = "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW",&0a,&0d
WIN1  	= "                                                                                ",&0a,&0d
WIN2  	= "                                                                                ",&0a,&0d
WIN3  	= "                              YOU  WIN                                          ",&0a,&0d
WIN4  	= "                                                                                ",&0a,&0d
WIN5  	= "                                                                                ",&0a,&0d
WIN6  	= "                                                                                ",&0a,&0d
WINEND  = "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW",&0a,&0d,0
	ALIGN
SIGN = "===================================================================================",&0a,&0d,0
SIGN1 = &0d,&0a," ",&0a,&0d,0
SCORE = &0d,&0a,"SCORE",&0a,&0d,0
	ALIGN
SIGN0 = "-----------------------------------------------------------------------------------",&0a,&0d
HOW 	= "  <<How to Play>>",&0a,&0d
SIGN2 = "-----------------------------------------------------------------------------------",&0a,&0d
TEXT 	= "		1.Press W is UP,Press S is DOWN	",&0a,&0d
TEXT0 	= "		2.Press Q to Exit Game 	",&0a,&0d
SIGN6 = "===================================================================================",&0a,&0d
TEXT6 	= "		<<Important>>	",&0a,&0d
SIGN7 = "===================================================================================",&0a,&0d
TEXT7 	= "		If Game Start ,Do Not Leave Control Keys W or S!!!	",&0a,&0d
SIGN3 = "-----------------------------------------------------------------------------------",&0a,&0d
TEXT1 	= "		<<Game Rule>>",&0a,&0d
SIGN4 = "-----------------------------------------------------------------------------------",&0a,&0d
TEXT2 	= "		1.Your Car is @",&0a,&0d
TEXT3 	= "		2.Obstacle is B",&0a,&0d
TEXT4 	= "		3.If B In Front Of @,Then <<Game Over>> ",&0a,&0d
TEXT5 	= "		4.Score Reach To 300 Then <<You Win>> ",&0a,&0d
SIGN5 = "-----------------------------------------------------------------------------------",&0a,&0d,0

	ALIGN 	
 		END