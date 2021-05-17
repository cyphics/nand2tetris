@color
M=0

(MAIN_LOOP)
@KBD
D=M
@ON
D; JGT
@color
M=0     // Set pixel color to white
@DRAW
0; JMP
(ON)
@color
M=-1    // Set pixel color to black

(DRAW)
@color
D=M
@SCREEN
M=D
@MAIN_LOOP
0;JMP
