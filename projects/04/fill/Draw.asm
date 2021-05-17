// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Fill.asm

// Runs an infinite loop that listens to the keyboard input.
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel;
// the screen should remain fully black as long as the key is pressed.
// When no key is pressed, the program clears the screen, i.e. writes
// "white" in every pixel;
// the screen should remain fully clear as long as no key is pressed.

// Put your code here.

// While true
// If pressed
// press = 1
// else press = 0
// Fill all pixels with press
// Got start

@color
M=-1

@8192
D=A
@segs
M=D             // total lines to draw = 8192


// DRAW SCREEN

// Set draw variables
@SCREEN
D=A
@draw_addr
M=D             // draw_addr = 16384

@cur_seg
M=0

(DRAW_LOOP)
@cur_seg
D=M
@segs
D=D-M
@END
D;JGT           // if cur_seg > lines, end

@color
D=M
@draw_addr
A=M
M=D            // Draw 11111 at RAM[draw_addr]

@cur_seg
M=M+1

@draw_addr
M=M+1

@DRAW_LOOP
0;JMP

(END)
@END
0;JMP
