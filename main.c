#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>

#ifdef _WIN32
    #include <direct.h>
    #define getcwd _getcwd // stupid MSFT "deprecation" warning
#elif
    #include <unistd.h>
#endif

#define ArrayCount(array)(sizeof(array) / sizeof(array[0]))

#define FILE_BUFFER_SIZE  5000
#define TOKENS_BUFFER_SIZE 200000
#define OUTPUT_BUFFER_SIZE 200000
// #define MAX_PATH 100000
#define Assert(Expression) if(!(Expression)) {*(volatile int *)0 = 0;}

char *token_buffer_ptr;
char *compilation_buffer_ptr;


/*************************************************
 * HELPER FUNCTIONS                              *
 ************************************************/


void CopyStr(char *dest, const char *source){
    while(*source != '\0'){
        *dest++ = *source++;
    }
    *dest = '\0';
}

void CopyStringSize(char *dest, const char *source, size_t size){
	size_t counter = 0;
	while(counter++ < size){
		*dest++ = *source++;
	}
}

void ConcatStr(char *dest, const char *source){
    while(*dest != '\0'){
        dest += 1;
    }
    CopyStr(dest, source);
}

void CopyToBuffer(char **buffer, const char *content){
	while(*content != '\0'){
		*(*buffer)++ = *content++;
	}
}

size_t StringSize(const char *str){
    size_t counter = 0;
    while(*str++ != '\0') counter++;
    return counter;
}

bool CompareStr(const char *first, const char *second, size_t length){
    for(size_t i = 0; i < length; i++){
        if(first[i] != second[i]) return false; 
    }
    return true;
}

size_t GetWordLength(const char *word){
    size_t length  = 1;
    while(*word++ != '\0') length++; 
    return length;
}

static bool IsNumerical(char c){
    return 48 <= c && c <= 57;
}

static bool IsAlphabetic(char c){
    return (65 <= c && c <= 90) || (97 <= c && c <= 122);
}

/*************************************************
 * SYMBOL TABLE FUNCTIONS                        *
 ************************************************/
typedef enum Kind{
	OTHER,
	STATIC,
	FIELD,
	ARG,
	VAR,
}Kind;

typedef struct Symbol{
	char name[100];
	char type[20];
	int id;
	Kind kind;
}Symbol;

int static_counter;
int field_counter;
int argument_counter;
int var_counter;

Symbol class_table[20];
Symbol routine_table[20];

void DefineSymbol(char *name, char *type, Kind kind){
	int count = -1;
	bool class_var = false;
	switch (kind) {
		case STATIC:
			count = static_counter++;
			class_var = true;
			break;
		case FIELD:
			count = field_counter++;
			class_var = true;
			break;
		case ARG:
			count = argument_counter++;
			break;
		case VAR:
			count = var_counter++;
			break;
		case OTHER:
			break;
	}

	Symbol s = {.kind = kind, .id = count};
	strcpy(s.name, name);
	strcpy(s.type, type);
	if(class_var) class_table[static_counter + field_counter - 1] = s;
	else routine_table[argument_counter + var_counter - 1] = s;
}


int GetVarCount(Kind kind){
	switch (kind) {
		case STATIC:
			return static_counter;
			break;
		case FIELD:
			return field_counter;
			break;
		case ARG:
			return argument_counter;
			break;
		case VAR:
			return var_counter;
			break;
		case OTHER: 
			return 0;
			break;
	}
	exit(1);
}

Kind GetKindOf(char *name){
	for(int i = 0; i < var_counter + argument_counter; i++){
		if (strcmp(name, routine_table[i].name)) {
			return routine_table[i].kind;
		}
	}

	for(int i = 0; i < static_counter + field_counter; i++){
		if (strcmp(name, class_table[i].name)) {
			return class_table[i].kind;
		}
	}
	return OTHER;
}

void GetTypeOf(char *buffer, char *name){
	for(int i = 0; i < var_counter + argument_counter; i++){
		if (strcmp(name, routine_table[i].name)) {
			strcpy(buffer, routine_table[i].type);	
		}
	}

	for(int i = 0; i < static_counter + field_counter; i++){
		if (strcmp(name, class_table[i].name)) {
			strcpy(buffer, class_table[i].name);
		}
	}
}

int GetIndexOf(char *name){
	for(int i = 0; i < var_counter + argument_counter; i++){
		if (strcmp(name, routine_table[i].name)) {
			return routine_table[i].id;
		}
	}

	for(int i = 0; i < static_counter + field_counter; i++){
		if (strcmp(name, class_table[i].name)) {
			return class_table[i].id;
		}
	}
	exit(1);
}




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

Token EmitNextToken(){
	char token_type[256] = {0};
	char *ptr = &token_type[0];
	Token t = GetAndEatToken();
	TypeToChar(ptr, t);
	OpenCloseTag(token_type, t.value);
	return t;
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
	char type[20];
	char name[100];

	while(HasVarDec()){
		OpenTag("varDec");
		Emit("\n");
		EmitNextToken(); // var
		strcpy(type, EmitNextToken().value);
		strcpy(name, EmitNextToken().value);
		DefineSymbol(name, type, VAR);
		
		while(NextTokenValue(",")){ 				// rest of inline vars
			EmitNextToken(); // , 
			strcpy(name, EmitNextToken().value);
			DefineSymbol(name, type, VAR);
		}

		EmitNextToken(); // ;
		CloseTag("varDec");
	}
}

void CompileParameterList(){
	char type[20];
	char name[100];
	
	OpenTag("parameterList");
	Emit("\n");
	if(!NextTokenValue(")")){
		strcpy(type, EmitNextToken().value);
		strcpy(name, EmitNextToken().value);
		DefineSymbol(name, type, ARG);

		while(!NextTokenValue(")")){
			EmitNextToken(); // ,
			strcpy(type, EmitNextToken().value);
			strcpy(name, EmitNextToken().value);
			DefineSymbol(name, type, ARG);
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
		var_counter = 0;
		argument_counter = 0;
		memset(routine_table, 0, sizeof(routine_table));
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
	memset(class_table, 0, sizeof(class_table));
	static_counter = 0;
	field_counter  = 0;
	char type[20];
	char name[100];
	Kind  kind;
	
	while(HasClassVarDec()){
		OpenTag("classVarDec");
		Emit("\n");

		if(CompareStr(EmitNextToken().value, "static", GetWordLength("static"))) 
			kind = STATIC;
		else 
			kind = FIELD;
		strcpy(type, EmitNextToken().value);
		strcpy(name, EmitNextToken().value);
		DefineSymbol(name, type, kind);

		while(!NextTokenValue(";")){ // Additional inline varNames
			EmitNextToken(); // ,
			strcpy(name, EmitNextToken().value);
			DefineSymbol(name, type, kind);
		}

		EmitNextToken(); // ;
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
	printf("saving to file %s\n", filename);
	FILE *file;
	fopen_s(&file, filename, "w");
	if(file != NULL){
	fprintf(file, "%s", buffer);
	} else{
		printf("Error : Failed to open entry file \"%s\"- %s\n", filename, strerror(errno));
	}
}

void CompileFile(FILE *file_handle, char *filename){
    printf("Processing file %s\n", filename);
    char file_buffer[FILE_BUFFER_SIZE]          = {0};
    char tokens_buffer[TOKENS_BUFFER_SIZE]      = {0};
	char compilation_buffer[OUTPUT_BUFFER_SIZE] = {0};

	token_buffer_ptr       = &tokens_buffer[0];
	compilation_buffer_ptr = &compilation_buffer[0];

    char output_name[MAX_PATH] = {0};
    char *output_name_ptr = &output_name[0];
	size_t bytes_to_read = fread(file_buffer, sizeof(char), FILE_BUFFER_SIZE - 1, file_handle);

    while(*filename != '.'){
        *output_name_ptr++ = *filename++;
    }

    TokenizeBuffer(file_buffer, tokens_buffer, bytes_to_read);
	token_buffer_ptr = &tokens_buffer[0]; // reposition pointer to parse identified tokens in compiler

    CompileClass();
	
#ifdef TESTING
	char compile_file[256];
	CopyStr(compile_file, output_name);
	ConcatStr(compile_file, "_compile");
	ConcatStr(compile_file, ".vm");
	SaveToFile(compilation_buffer, compile_file);
#else
	char compile_file[256];
	CopyStr(compile_file, output_name);
	ConcatStr(compile_file, ".vm");
	printf("Saving to file %s\n", compile_file);
	SaveToFile(compilation_buffer, compile_file);
#endif

}

bool IsFile(char *path){
	struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

bool EndsWith(const char *str, const char *suffix)
{
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix >  lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

int main(int argc, char *argv[]){

#ifdef TESTING 
	printf("Running test build.\n");
#else
	printf("Running normal build\n");
#endif

    if(argc < 2){
        exit(1);
    }

    char *path = argv[1];
	char current_dir[MAX_PATH];
	getcwd(current_dir, MAX_PATH);
		
	if(IsFile(path)){
		
	} else {
		DIR *d = opendir(path);
		struct dirent *dir;
		if(d){
			while((dir = readdir(d)) != NULL){
				if(!EndsWith(dir->d_name, ".jack"))
					continue;
				char file_path[MAX_PATH] = {0};
				strcat(file_path, current_dir);
				strcat(file_path, "/");
				strcat(file_path, path);
				strcat(file_path, "/");
				strcat(file_path, dir->d_name);
				FILE *file;
				fopen_s(&file, file_path, "r");
				if(file != NULL){
 					CompileFile(file, file_path);
				} else{
					printf("Error : Failed to open entry file \"%s\"- %s\n", file_path, strerror(errno));
				}

			}
			closedir(d);
		}
	}
	
    return 0;
}


