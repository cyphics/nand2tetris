// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Mult.asm

// Multiplies R0 and R1 and stores the result in R2.
// (R0, R1, R2 refer to RAM[0], RAM[1], and RAM[2], respectively.)

// Put your code here.


// Set variables
@R2
M=0
@i
M=0

(LOOP)
// if i > R1
// Goto END
@i
D=M     // D=i
@R1
D=D-M   // D=i-R1
@END
D; JGE  // If (i-R1) >= 0 goto END

// Else
// mult += R0
@R0
D=M
@R2
M=D+M

@i
M=M+1

@LOOP
0;JMP   // Goto Loop

(END)
@END
0;JMP
