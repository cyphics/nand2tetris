#include <stdio.h>
#include <stdbool.h>

#include "base.c"

#define ArrayCount(array)(sizeof(array) / sizeof(array[0]))

#define FILE_BUFFER_SIZE  5000
#define TOKENS_BUFFER_SIZE 200000
#define OUTPUT_BUFFER_SIZE 200000
char *token_buffer_ptr;
char *compilation_buffer_ptr;

const char symbolsList[] = { '{', '}', '(', ')', '[', ']', '.', ',', ';', '+', '-', '*', '/', '&', '|', '<', '>', '=', '~'};
const char* keywordsList[] = { "class", "constructor", "function", "method", "field", "static", "var", "int", "char", "boolean", "void", "true", "false", "null", "this", "let", "do", "if", "else", "while", "return"};
const char opList[] = {'+', '-', '*', '/', '&', '|', '<', '>', '='};
const char unaryOpList[] = {'-', '~'};

typedef enum {
    NONE,
    SYMBOL,
    KEYWORD,
    INTEGER_CONSTANT,
    STRING_CONSTANT,
    IDENTIFIER
} TokenId;

TokenId TokensIdList[6]   = {NONE, SYMBOL, KEYWORD, INTEGER_CONSTANT, STRING_CONSTANT, IDENTIFIER};
char *  TokensCharList[6] = {"none", "symbol", "keyword", "integerConstant", "stringConstant", "identifier"};

TokenId GetTokenId(char *token_char){
	for (int i = 0; i < ArrayCount(TokensIdList); i++) {
		if(CompareStr(token_char + 1, TokensCharList[i], StringSize(TokensCharList[i]))) {
			token_char += StringSize(TokensCharList[i]) + 2;
			return TokensIdList[i];
		}
	}
	return NONE;
}

typedef struct Token{
    char value[256];
    TokenId type;
}Token;


void TypeToChar(char *buffer, Token t){
        switch(t.type){
            case KEYWORD:
				CopyStr(buffer, "keyword");
                break;
            case SYMBOL: 
				CopyStr(buffer, "symbol");
                break;
            case INTEGER_CONSTANT:
				CopyStr(buffer, "integerConstant");
                break;
            case STRING_CONSTANT:
				CopyStr(buffer, "stringConstant");
                break;
            case IDENTIFIER:
				CopyStr(buffer, "identifier");
                break;
            case NONE:
				CopyStr(buffer, "none");
                break;
        }
}

bool IsKeyword(char *keyword){
    for(size_t i = 0; i<ArrayCount(keywordsList); i++){
        if(CompareStr(keyword, keywordsList[i], GetWordLength(keywordsList[i]))) return true;
    }
    return false;
}

bool IsSymbol(char c){
    for(size_t i = 0; i<ArrayCount(symbolsList); i++){
        if(c == symbolsList[i]) return true;
    }
    return false;
}

bool IsOp(char c){
    for(size_t i = 0; i<ArrayCount(opList); i++){
        if(c == opList[i]) return true;
    }
    return false;
}

Token ParseNextToken(char **current_pos){
    int counter = 0;
    Token token = { .value = "" };
    if(IsAlphabetic(**current_pos)){
        while(IsAlphabetic(**current_pos)){
            token.value[counter++] = **current_pos;
            *current_pos += 1;
        }
        if(IsKeyword(token.value)) token.type = KEYWORD;
        else token.type = IDENTIFIER;

    } else if(IsNumerical(**current_pos)){
        do{
            token.value[counter++] = **current_pos;
            token.type = INTEGER_CONSTANT;
            *current_pos += 1;
        }while(IsNumerical(**current_pos));
    } else if (**current_pos == '"'){
        *current_pos += 1;
        while(**current_pos != '"'){
            token.value[counter++] = **current_pos;
            token.type = STRING_CONSTANT;
            *current_pos += 1;
        }
        *current_pos += 1;
    }  else if (IsSymbol(**current_pos)){
        token.value[counter++] = **current_pos;
        token.type = SYMBOL; 
        *current_pos += 1;
    }
    token.value[counter] = '\0';
    return token;
}

bool IsComment(char *pos){
    return *pos == '/' && ((*(pos + sizeof(char)) == '/') || (*(pos + sizeof(char)) == '*'));
}

void TrimBuffer(char **pos){
    bool has_trimed = true;
    while(has_trimed){
        has_trimed = false;
        if (**pos == '/' && *(*pos + 1) == '/'){
            while(**pos != '\n') *pos += 1;
            *pos += 1;
            has_trimed = true;
        } else if (**pos == '/' && *(*pos + 1) == '*'){
            while(**pos != '*' || *(*pos + 1) != '/') *pos += 1;
            *pos += 2;
            has_trimed = true;
        } else if(**pos == ' ') {
            while(**pos == ' ') *pos += 1;
            has_trimed = true;
	//	} else if (**pos == '\"') {
	//		while(**pos == '\"') *pos += 1; 
	//		has_trimed = true;
		} else if (**pos == '\t') {
			while(**pos == '\t') *pos += 1; 
			has_trimed = true;
        } else if (**pos == '\n') {
            while(**pos == '\n') *pos += 1;
            has_trimed = true;
        } else if (**pos == '\r' && *(*pos + 1) == '\n') {
            *pos += 2;
            has_trimed = true;
        }
    }
}


static void TokenizeBuffer(char *input_buffer, char *tokens_buffer, size_t bytes_to_read){
	CopyToBuffer(&token_buffer_ptr, "<tokens>\n");
    char *current_pos = input_buffer;
    while((size_t)(current_pos - input_buffer) < bytes_to_read){
        TrimBuffer(&current_pos);
        Token token = ParseNextToken(&current_pos);
        char type[256] = "";
		TypeToChar(&type[0], token);
        if(token.type != NONE){
            char tag[30] = "<";
            ConcatStr(tag, type); 
            ConcatStr(tag, ">");
            CopyToBuffer(&token_buffer_ptr, tag);
            switch(token.value[0]){
                case '<':
                    CopyToBuffer(&token_buffer_ptr, "&lt;");
                    break;
                case '>':
                    CopyToBuffer(&token_buffer_ptr, "&gt;");
                    break;
                case '"':
                    CopyToBuffer(&token_buffer_ptr,"&quot;");
                    break;
                case '&':
                    CopyToBuffer(&token_buffer_ptr, "&amp;");
                    break;
                default:
                    CopyToBuffer(&token_buffer_ptr, token.value);
                    break;
            }

            CopyStr(tag, "</");
            ConcatStr(tag, type); 
            ConcatStr(tag, ">\n");
            CopyToBuffer(&token_buffer_ptr, tag);
        }
    }
    CopyToBuffer(&token_buffer_ptr, "</tokens>\n");
}

void EatToken(){
    while(*token_buffer_ptr != '\n') token_buffer_ptr+= 1;
    token_buffer_ptr += 1;
}

Token GetNextToken(){
	Token t = {0};
	char *token_name_ptr = &t.value[0];
	char *copy_ptr = token_buffer_ptr;
	while(*copy_ptr != '<') copy_ptr ++;
	t.type = GetTokenId(copy_ptr);

	do{ copy_ptr += 1; } while (*copy_ptr != '>');
	copy_ptr += 1;
	while(*copy_ptr != '<') {
		*(token_name_ptr++) = *copy_ptr;
		copy_ptr += 1;
	}
	return t;
}

bool NextTokenValue(char *value){
	return CompareStr(GetNextToken().value, value,    StringSize(value));
}

Token GetAndEatToken(){
	Token t = GetNextToken();
	EatToken();
	return t;
}

void Emit(char *str){
	CopyToBuffer(&compilation_buffer_ptr, str);
}

void OpenTag(char *tag){
	CopyToBuffer(&compilation_buffer_ptr, "<");
	CopyToBuffer(&compilation_buffer_ptr, tag);
	CopyToBuffer(&compilation_buffer_ptr, ">");
}

void CloseTag(char *tag){
	CopyToBuffer(&compilation_buffer_ptr, "</");
	CopyToBuffer(&compilation_buffer_ptr, tag);
	CopyToBuffer(&compilation_buffer_ptr, ">\n");
}

void OpenCloseTag(char *tag, char *value){
	OpenTag(tag);
	Emit(value);
	CloseTag(tag);
}

void EmitNextToken(){
	char token_type[256] = {0};
	char *ptr = &token_type[0];
	Token t = GetAndEatToken();
	TypeToChar(ptr, t);
	OpenCloseTag(token_type, t.value);
}


void CompileStatements();

void CompileExpression();

void CompileSubroutineCall();
void CompileTerm(){
	OpenTag("term");
	Emit("\n");
	if(NextTokenValue("(")){
		EmitNextToken(); // (
		CompileExpression();
		EmitNextToken(); // )
	} else if(NextTokenValue("-") || NextTokenValue("~")){
		EmitNextToken(); // Unary operator
		CompileTerm(); 
	} else {
		EmitNextToken();
		if(NextTokenValue("[")){
			EmitNextToken(); // [
			CompileExpression();
			EmitNextToken(); // ]
		} else if(NextTokenValue("(") || NextTokenValue(".")){ // test subroutine call
			CompileSubroutineCall();
		} 
	}
	CloseTag("term");

}

void CompileExpression(){
	bool second_term;
	OpenTag("expression");
	Emit("\n");
	do{
		second_term = false;
		CompileTerm();
		Token t = GetNextToken();
		if(IsOp(t.value[0])) {
			EmitNextToken();
			second_term = true;
		}
	} while(second_term);
	CloseTag("expression");
}

void CompileExpressionList(){
	OpenTag("expressionList");
	Emit("\n");
	while(!NextTokenValue(")")){
		CompileExpression();
		if(NextTokenValue(",")){
			EmitNextToken(); // ,
		}
	}
	CloseTag("expressionList");
}

void CompileReturn(){
	OpenTag("returnStatement");
	Emit("\n");
	EmitNextToken(); // return
	if(!NextTokenValue(";")){
		CompileExpression();
	}
	EmitNextToken(); // ;
	CloseTag("returnStatement");
}

void CompileSubroutineCall(){
	if(NextTokenValue("(")){
		EmitNextToken(); // (
		CompileExpressionList();
		EmitNextToken(); // )
	} else{
		EmitNextToken(); // .
		EmitNextToken(); // subroutine name
		EmitNextToken(); // (
		CompileExpressionList();
		EmitNextToken(); // )
	}
}
void CompileDo(){
	OpenTag("doStatement");
	Emit("\n");
	EmitNextToken(); // do 
	EmitNextToken(); // 
	CompileSubroutineCall();
    EmitNextToken(); // ;
	CloseTag("doStatement");
}

void CompileWhile(){
	OpenTag("whileStatement");
	Emit("\n");
	EmitNextToken(); // while
	EmitNextToken(); // (
	CompileExpression();
	EmitNextToken(); // )
	EmitNextToken(); // {
	CompileStatements();
	EmitNextToken(); // }
    CloseTag("whileStatement");
}
void CompileIf(){
	OpenTag("ifStatement");
	Emit("\n");
	EmitNextToken(); // if
	EmitNextToken(); // (
	CompileExpression();
	EmitNextToken(); // )
	EmitNextToken(); // {
	CompileStatements();
	EmitNextToken(); // }
	if(NextTokenValue("else")){
		EmitNextToken(); // else
		EmitNextToken(); // {
		CompileStatements();
		EmitNextToken(); // }
	}
	CloseTag("ifStatement");
}

void CompileLet(){
	OpenTag("letStatement");
	Emit("\n");
	EmitNextToken(); 			// let
	EmitNextToken(); 			// varName
	if(NextTokenValue("[")){
		EmitNextToken(); 		// [	
		CompileExpression();
		EmitNextToken(); 		// ]	
	}
	EmitNextToken(); 			// =
	CompileExpression();
	EmitNextToken(); 			// ;
	CloseTag("letStatement");
}



bool HasStatement(){
	return NextTokenValue("let")   ||
		   NextTokenValue("if")    ||
		   NextTokenValue("while") ||
		   NextTokenValue("do")    ||
		   NextTokenValue("return");
}

void CompileStatements(){
	OpenTag("statements");
	Emit("\n");
	while(HasStatement()){
		if(NextTokenValue("let"))    CompileLet();
		if(NextTokenValue("if"))     CompileIf();
		if(NextTokenValue("while"))  CompileWhile();
		if(NextTokenValue("do"))     CompileDo();
		if(NextTokenValue("return")) CompileReturn();
	}
	CloseTag("statements");
}

bool HasVarDec(){
	return NextTokenValue("var");
}

void CompileVarDec(){
	while(HasVarDec()){
		OpenTag("varDec");
		Emit("\n");
		EmitNextToken(); // var
		EmitNextToken(); // type
		EmitNextToken(); // varName
		while(NextTokenValue(",")){ 				// rest of inline vars
			EmitNextToken(); // , 
			EmitNextToken(); // varName 
		}
		EmitNextToken(); // ;
		CloseTag("varDec");
	}
}

void CompileParameterList(){
	OpenTag("parameterList");
	Emit("\n");
	if(!NextTokenValue(")")){
		EmitNextToken(); // type
		EmitNextToken(); // paramName
		while(!NextTokenValue(")")){
		EmitNextToken(); // ,
		EmitNextToken(); // type
		EmitNextToken(); // paramName
		}
	}
	CloseTag("parameterList");
}

bool HasSubroutine(){
	return NextTokenValue("constructor") ||
		   NextTokenValue("function")    ||
		   NextTokenValue("method");
}

void CompileSubroutine(){
	while( HasSubroutine()){ 
		OpenTag("subroutineDec");
		Emit("\n");
		EmitNextToken(); // constructor/function/method
		EmitNextToken(); // void/type
		EmitNextToken(); // function name
		EmitNextToken(); // (
		CompileParameterList();
		EmitNextToken(); // )
		OpenTag("subroutineBody");
		Emit("\n");
		EmitNextToken(); // {
		CompileVarDec();
		CompileStatements();
		EmitNextToken(); // } 
		CloseTag("subroutineBody");
		CloseTag("subroutineDec");
	}
}

bool HasClassVarDec(){
	return NextTokenValue("static") ||
		   NextTokenValue("field");
}

void CompileClassVarDec(){
	while(HasClassVarDec()){
		OpenTag("classVarDec");
		Emit("\n");
		EmitNextToken(); // static / field
		EmitNextToken(); // type
		EmitNextToken(); // varName
		while(!NextTokenValue(";")){ // Additional inline varNames
			EmitNextToken(); // ,
			EmitNextToken(); // varName
		}
		EmitNextToken();
		CloseTag("classVarDec");
	}
}



void CompileClass(){
	EatToken(); // remove <tokens>
	OpenTag("class");
	Emit("\n");
	EmitNextToken(); // class
	EmitNextToken(); // className
	EmitNextToken(); // {
	CompileClassVarDec();
	CompileSubroutine();
	EmitNextToken(); // }
	CloseTag("class");
}

void SaveToFile(char *buffer, char *filename){
    HANDLE output_file_handle = CreateFileA(filename, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(output_file_handle == INVALID_HANDLE_VALUE){
        DWORD file_error_id = GetLastError();
        dprintf("Couldn't open output file: %s\n", filename);
        ReadErrorMessage("Error", file_error_id);
        exit(1);
    } else {
        DWORD bytes_written = 0;
        WriteFile(output_file_handle, buffer, (DWORD)StringSize(buffer), &bytes_written, NULL);
       dprintf("Successfully successfully wrote buffer to file %s\n", filename);
    }
}

void ProcessFileHandle(HANDLE file_handle, char *filename){
    dprintf("Processing file %s\n", filename);
    printf("Processing file %s\n", filename);

    char file_buffer[FILE_BUFFER_SIZE]          = {0};
    char tokens_buffer[TOKENS_BUFFER_SIZE]      = {0};
	char compilation_buffer[OUTPUT_BUFFER_SIZE] = {0};

	token_buffer_ptr       = &tokens_buffer[0];
	compilation_buffer_ptr = &compilation_buffer[0];

    char output_name[MAX_PATH] = {0};
    char *output_name_ptr = &output_name[0];
    DWORD bytes_to_read = GetFileSize(file_handle, NULL);
    DWORD bytes_read;

    if(!ReadFile(file_handle, file_buffer, bytes_to_read, &bytes_read, NULL)){
        ReadErrorMessage("Error reading file.", GetLastError());
        exit(1);
    }

    while(*filename != '.'){
        *output_name_ptr++ = *filename++;
    }

    TokenizeBuffer(file_buffer, tokens_buffer, bytes_to_read);
	token_buffer_ptr = &tokens_buffer[0]; // reposition pointer to parse identified tokens in compiler

#ifdef TESTING
	char tokens_file[256];
	CopyStr(tokens_file, output_name);
	ConcatStr(tokens_file, "_tokens");
	ConcatStr(tokens_file, ".xml");
	SaveToFile(tokens_buffer, tokens_file);
#endif

    CompileClass();
	
#ifdef TESTING
	char compile_file[256];
	CopyStr(compile_file, output_name);
	ConcatStr(compile_file, "_compile");
	ConcatStr(compile_file, ".xml");
	SaveToFile(compilation_buffer, compile_file);
#endif
}

#include "tests.c"
int main(int argc, char *argv[]){

#ifdef TESTING 
	printf("Running test build.\n");
#else
	printf("Running normal build.n");
#endif

    if(argc < 2){
        dprintf("You must provide a filename or a directory path");
        exit(1);
    }

    char *path = argv[1];

	// Deal with path stuff
    WIN32_FIND_DATA ffd;
    HANDLE search_handle = FindFirstFileA(path, &ffd);
	if(search_handle == INVALID_HANDLE_VALUE){
		DWORD file_error_id = GetLastError();
		printf("Couldn't open input path: %s\n", path);
		ReadErrorMessage("Error", file_error_id);
		exit(1);
	}
    if(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
        char search_pattern[MAX_PATH];
        CopyStr(search_pattern, path);
        ConcatStr(search_pattern, "\\*.jack");
        search_handle = FindFirstFile(search_pattern, &ffd);
        do{
            char full_file_path[MAX_PATH] = {0};
            CopyStr(full_file_path, path); 
            ConcatStr(full_file_path, "\\");
            ConcatStr(full_file_path, ffd.cFileName);
            dprintf("Parsing file %s\n", full_file_path);
            HANDLE file_handle = CreateFile(full_file_path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if(file_handle == INVALID_HANDLE_VALUE){
                DWORD file_error_id = GetLastError();
                dprintf("Couldn't open output file: %s\n", full_file_path);
                ReadErrorMessage("Error", file_error_id);
            } else {
                ProcessFileHandle(file_handle, full_file_path);
            }
        } while(FindNextFileA(search_handle, &ffd) != 0);
    } else{
            HANDLE file_handle = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
            ProcessFileHandle(file_handle, path);
    }
    return 0;
}


