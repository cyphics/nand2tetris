@echo off

cl -W4 -wd4996 -wd4201 -wd4100 -wd4189 -wd4505 -wd4127 -Zi -nologo main.c /DTESTING
IF %ERRORLEVEL% GEQ 1 EXIT /B 2

set all_tests=false

if %all_tests% equ true (
REM Clean old test files
if exist test\Main_tokens.xml ( del test\Main_tokens.xml )
if exist test\Square_tokens.xml ( del test\Square_tokens.xml )
if exist test\SquareGame_tokens.xml (del test\SquareGame_tokens.xml )
if exist test\expressionLess\Main_compile.xml ( del test\expressionLess\Main_compile.xml )
if exist test\expressionLess\Square_compile.xml ( del test\expressionLess\Square_compile.xml )
if exist test\expressionLess\SquareGame_compile.xml (del test\expressionLess\SquareGame_compile.xml )

main.exe test
IF %ERRORLEVEL% GEQ 1 EXIT /B 2
echo "Testing Tokenizer files"
echo "Testing Main.jack tokens..."
call ../../../tools/TextComparer.bat test/MainT.xml test/Main_tokens.xml
echo "Testing Sqare.jack tokens..."
call ../../../tools/TextComparer.bat test/SquareT.xml test/Square_tokens.xml
echo "Testing SquareGame.jack tokens..."
call ../../../tools/TextComparer.bat test/SquareGameT.xml test/SquareGame_tokens.xml
echo "Tokenizer tested!"
echo "" 


main.exe test/expressionLess
echo "Testing simple compiler..."
echo "Testing expressionLess version of Main.jack..."
call ../../../tools/TextComparer.bat test/expressionLess/Main.xml test/expressionLess/Main_compile.xml
echo "Testing expressionLess version of Square.jack..."
REM type "test\expressionLess\Square_compile.xml"
call ../../../tools/TextComparer.bat test/expressionLess/Square.xml test/expressionLess/Square_compile.xml
echo "Testing expressionLess version of SquareGame.jack..."
call ../../../tools/TextComparer.bat test/expressionLess/SquareGame.xml test/expressionLess/SquareGame_compile.xml
echo "Simple compiler tested!"
echo ""

echo "Testing final compiler"
echo "Testing Main.jack..."
call ../../../tools/TextComparer.bat test/Main.xml test/Main_compile.xml
echo "Testing Square.jack..."
call ../../../tools/TextComparer.bat test/Square.xml test/Square_compile.xml
echo "Testing SquareGamejack..."
call ../../../tools/TextComparer.bat test/SquareGame.xml test/SquareGame_compile.xml
echo ""

echo "Testing VM generation..."
REM main.exe test/bank.jack
REM type test\bank.jack
REM type test\bank.vm
main.exe Seven/Main.jack
type Seven\Main.vm

EXIT /B 0
) else (
	main.exe Seven/Main.jack
	type Seven\Main.jack
	type Seven\Main.vm

)


