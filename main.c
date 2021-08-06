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
#define OUTPUT_BUFFER_SIZE 1024 * 1024
// #define MAX_PATH 100000
#define Assert(Expression) if(!(Expression)) {*(volatile int *)0 = 0;}

char *token_buffer_ptr;
char *compile_xml_ptr;
char *compile_vm_ptr;

char current_class_name[100];
char current_routine_name[100];


/*************************************************
 * HELPER FUNCTIONS                              *
 ************************************************/

void __cdecl dprintf(const char *format, ...){
    char buf[4096], *p = buf;
    va_list args;
    int n;
    va_start(args, format);
    n = _vsnprintf_s(p, sizeof buf - 3, _TRUNCATE, format, args);
    va_end(args);
    p += (n < 0) ? sizeof buf - 3 : n;
    while(p > buf && isspace(p[-1]) )
        *--p = '\0';
    *p++ = '\r';
    *p++ = '\n';
    *p   = '\0';
    OutputDebugString(buf);
}



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

typedef struct VMVar{
	char *type;
	int index;
}VMVar;

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

char *KindToStr(Kind k){
	switch (k) {
		case STATIC:
			return "static";
			break;
		case FIELD:
			return "";
			break;
		case ARG:
			return "argument";
			break;
		case VAR:
			return "local";
			break;
		default:
			return "";
			break;
	}
}


/*************************************************
 * WRITE TO BUFFER FUNCTIONS                     *
 ************************************************/

void Write(char *segment){
	compile_vm_ptr += sprintf(compile_vm_ptr, "%s\n", segment);
}	

void WritePush(char *segment, int index){ 
	compile_vm_ptr += sprintf(compile_vm_ptr, "push %s %i\n", segment, index);
}

void WritePushConst(char *constant){
	compile_vm_ptr += sprintf(compile_vm_ptr, "push constant %s\n", constant);
}

void WritePop(char *segment, int index){ 
	compile_vm_ptr += sprintf(compile_vm_ptr, "pop %s %i\n", segment, index);
}

void WriteArithmetic(char *cmd){
	if(cmd[0] == '+'){
		Write("add");
	} else if(cmd[0] == '*'){
		Write("call Math.multiply 2");
		}
}

void WriteLabel(char *label){}

void WriteGoto(char *label){}

void WriteIf(char *label){}

void WriteCall(char *name, int nArgs){
	compile_vm_ptr += sprintf(compile_vm_ptr, "call %s %i\n", name, nArgs);
}

void WriteFunction(char *type, int nLocals){
	compile_vm_ptr += sprintf(compile_vm_ptr, "%s %s.%s %i\n", type, current_class_name, current_routine_name, nLocals);
}


void WriteReturn(){
	compile_vm_ptr += sprintf(compile_vm_ptr, "return\n");
}

void WriteExpression(char *ptr){

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

char *GetKindOf(char *name){
	for(int i = 0; i < var_counter + argument_counter; i++){
		if (strcmp(name, routine_table[i].name)) {
			return KindToStr(routine_table[i].kind);
		}
	}

	for(int i = 0; i < static_counter + field_counter; i++){
		if (strcmp(name, class_table[i].name)) {
			return KindToStr(class_table[i].kind);
		}
	}
	return "unknown";
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

typedef enum TokenId{
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

void DiscardToken(){
    while(*token_buffer_ptr != '\n') token_buffer_ptr++;
    token_buffer_ptr++;
}

Token GetCurrentToken(){
	Token t = {0};
	char *token_name_ptr = &t.value[0];
	char *copy_ptr = token_buffer_ptr;
	while(*copy_ptr != '<') copy_ptr++;
	t.type = GetTokenId(copy_ptr);

	do{ copy_ptr += 1; } while (*copy_ptr != '>');
	copy_ptr++;
	while(*copy_ptr != '<') {
		*(token_name_ptr++) = *copy_ptr;
		copy_ptr++;
	}
	return t;
}

bool CurrentTokenValue(char *value){
	return CompareStr(GetCurrentToken().value, value,    StringSize(value));
}

bool NextTokenValue(char *ptr, const char *value) {
	char token_name[100];
	char *token_name_ptr = &token_name[0];
	
    while(*ptr != '\n') ptr++;
    ptr++;
	while(*ptr !=  '<') ptr++;
	do{ ptr++; } while (*ptr != '>');
	ptr++;
	return *ptr == *value; // We only compare a single character so we can use this shortcut
	//while(*copy_ptr != '<') {
	//	*(token_name_ptr++) = *copy_ptr;
	//	copy_ptr ++;
	//}
	//dprintf("after %s", value);
	//return CompareStr(token_name, value, StringSize(value));
}

Token EatToken(){
	Token t = GetCurrentToken();
	DiscardToken();
	return t;
}

void EmitXml(char *str){
	CopyToBuffer(&compile_xml_ptr, str);
}

void OpenTag(char *tag){
	CopyToBuffer(&compile_xml_ptr, "<");
	CopyToBuffer(&compile_xml_ptr, tag);
	CopyToBuffer(&compile_xml_ptr, ">");
}

void CloseTag(char *tag){
	CopyToBuffer(&compile_xml_ptr, "</");
	CopyToBuffer(&compile_xml_ptr, tag);
	CopyToBuffer(&compile_xml_ptr, ">\n");
}

void OpenCloseTag(char *tag, char *value){
	OpenTag(tag);
	EmitXml(value);
	CloseTag(tag);
}

Token EmitToken(){
	char token_type[256] = {0};
	char *ptr = &token_type[0];
	Token t = EatToken();
	TypeToChar(ptr, t);
	OpenCloseTag(token_type, t.value);
	return t;
}

void CompileStatements();

void CompileExpression();

void CompileSubroutineCall();

void CompileTerm(){
	Token t;
	OpenTag("term");
	EmitXml("\n");
	if(CurrentTokenValue("(")){
		EmitToken(); // (
		CompileExpression();
		EmitToken(); // )
	} else if(CurrentTokenValue("-") || CurrentTokenValue("~")){
		t = EmitToken(); // Unary operator
		CompileTerm(); 
		if(t.value[0] == '-') Write("neg");
		else Write("not");
	} else if (NextTokenValue(token_buffer_ptr, "[")){
			EmitToken(); // array name
			EmitToken(); // [
			CompileExpression();
			EmitToken(); // ]
	} else if(NextTokenValue(token_buffer_ptr, "(") || NextTokenValue(token_buffer_ptr, "."))
		CompileSubroutineCall();
	else {
		t = EmitToken();
		if(t.type == INTEGER_CONSTANT)  
			WritePushConst(t.value);
		else{
			WritePush(GetKindOf(t.value), GetIndexOf(t.value));	
		}

	}
	CloseTag("term");

}

void CompileExpression(){
	char *token_mark = token_buffer_ptr;
	char *compile_mark = compile_xml_ptr;
	OpenTag("expression");
	EmitXml("\n");
	CompileTerm();
	Token t = GetCurrentToken();
	if(IsOp(t.value[0])){
		DiscardToken(); // Operator
		CompileExpression();
		WriteArithmetic(t.value);
	}
	//do{
	//	second_term = false;
	//	CompileTerm();
	//	Token t = GetCurrentToken();
	//	if(IsOp(t.value[0])) {
	//		EmitToken();
	//		second_term = true;
	//	}
	//} while(second_term);
	CloseTag("expression");
}

int CompileExpressionList(){
	int arg_counter = 0;
	char *token_mark = token_buffer_ptr;
	char *compile_mark = compile_xml_ptr;
	OpenTag("expressionList");
	EmitXml("\n");
	while(!CurrentTokenValue(")")){
		arg_counter++;
		CompileExpression();
		if(CurrentTokenValue(",")){
			EmitToken(); // ,
		}
	}
	CloseTag("expressionList");
	return arg_counter;
}

void CompileReturn(){
	char *token_mark = token_buffer_ptr;
	char *compile_mark = compile_xml_ptr;
	OpenTag("returnStatement");
	EmitXml("\n");
	EmitToken(); // return
	if(!CurrentTokenValue(";")){
		CompileExpression();
	} else {
		WritePushConst("0");
		WriteReturn();
	}
	EmitToken(); // ;
	CloseTag("returnStatement");
}

void CompileSubroutineCall(){
	char *token_mark = token_buffer_ptr;
	char *compile_mark = compile_xml_ptr;
	int nArgs = 0;
	char subroutine_name[100];
	strcpy(subroutine_name, EmitToken().value); // identifier 
	if(CurrentTokenValue("(")){
		EmitToken(); // (
		nArgs = CompileExpressionList();
		EmitToken(); // )
	} else{
		EmitToken(); // .
		strcat(subroutine_name, ".");
		strcat(subroutine_name, EmitToken().value); // subroutine name
		EmitToken(); // (
		nArgs = CompileExpressionList();
		EmitToken(); // )
	}
	WriteCall(subroutine_name, nArgs);
}
void CompileDo(){
	char *token_mark = token_buffer_ptr;
	char *compile_mark = compile_xml_ptr;
	OpenTag("doStatement");
	EmitXml("\n");
	EmitToken(); // do 
	CompileSubroutineCall();
    EmitToken(); // ;
	Write("pop temp 0");
	CloseTag("doStatement");
}

void CompileWhile(){
	char *token_mark = token_buffer_ptr;
	char *compile_mark = compile_xml_ptr;
	OpenTag("whileStatement");
	EmitXml("\n");
	EmitToken(); // while
	EmitToken(); // (
	CompileExpression();
	EmitToken(); // )
	EmitToken(); // {
	CompileStatements();
	EmitToken(); // }
    CloseTag("whileStatement");
}
void CompileIf(){
	char *token_mark = token_buffer_ptr;
	char *compile_mark = compile_xml_ptr;
	OpenTag("ifStatement");
	EmitXml("\n");
	EmitToken(); // if
	EmitToken(); // (
	CompileExpression();
	EmitToken(); // )
	EmitToken(); // {
	CompileStatements();
	EmitToken(); // }
	if(CurrentTokenValue("else")){
		EmitToken(); // else
		EmitToken(); // {
		CompileStatements();
		EmitToken(); // }
	}
	CloseTag("ifStatement");
}

void CompileLet(){
	char *token_mark = token_buffer_ptr;
	char *compile_mark = compile_xml_ptr;
	OpenTag("letStatement");
	EmitXml("\n");
	char name[100];
	EmitToken(); 			// let
	strcpy(name, EmitToken().value);
	if(CurrentTokenValue("[")){
	GetKindOf(name);
		EmitToken(); 		// [	
		CompileExpression();
		EmitToken(); 		// ]	
	}
	EmitToken(); 			// =
	CompileExpression();
	GetKindOf(name);
	WritePop(GetKindOf(name), GetIndexOf(name));
	EmitToken(); 			// ;
	CloseTag("letStatement");
}



bool HasStatement(){
	return CurrentTokenValue("let")   ||
		   CurrentTokenValue("if")    ||
		   CurrentTokenValue("while") ||
		   CurrentTokenValue("do")    ||
		   CurrentTokenValue("return");
}

void CompileStatements(){
	char *token_mark = token_buffer_ptr;
	char *compile_mark = compile_xml_ptr;
	OpenTag("statements");
	EmitXml("\n");
	while(HasStatement()){
		if(CurrentTokenValue("let"))    CompileLet();
		if(CurrentTokenValue("if"))     CompileIf();
		if(CurrentTokenValue("while"))  CompileWhile();
		if(CurrentTokenValue("do"))     CompileDo();
		if(CurrentTokenValue("return")) CompileReturn();
	}
	CloseTag("statements");
}

bool HasVarDec(){
	return CurrentTokenValue("var");
}

void CompileVarDec(){
	char *token_mark = token_buffer_ptr;
	char *compile_mark = compile_xml_ptr;
	char type[20];
	char name[100];

	while(HasVarDec()){
		OpenTag("varDec");
		EmitXml("\n");
		EmitToken(); // var
		strcpy(type, EmitToken().value);
		strcpy(name, EmitToken().value);
		DefineSymbol(name, type, VAR);
		
		while(CurrentTokenValue(",")){ 				// rest of inline vars
			EmitToken(); // , 
			strcpy(name, EmitToken().value);
			DefineSymbol(name, type, VAR);
		}

		EmitToken(); // ;
		CloseTag("varDec");
	}
}

int CompileParameterList(){
	char *token_mark = token_buffer_ptr;
	char *compile_mark = compile_xml_ptr;
	char type[20];
	char name[100];
	int nArgs = 0;
	
	OpenTag("parameterList");
	EmitXml("\n");
	if(!CurrentTokenValue(")")){
		nArgs++;
		strcpy(type, EmitToken().value);
		strcpy(name, EmitToken().value);
		DefineSymbol(name, type, ARG);

		while(!CurrentTokenValue(")")){
			nArgs++;
			EmitToken(); // ,
			strcpy(type, EmitToken().value);
			strcpy(name, EmitToken().value);
			DefineSymbol(name, type, ARG);
		}
	}
	CloseTag("parameterList");
	return nArgs;
}

bool HasSubroutine(){
	return CurrentTokenValue("constructor") ||
		   CurrentTokenValue("function")    ||
		   CurrentTokenValue("method");
}

void CompileSubroutine(){
	char *token_mark = token_buffer_ptr;
	char *compile_mark = compile_xml_ptr;
	while( HasSubroutine()){ 
		var_counter = 0;
		argument_counter = 0;
		memset(routine_table, 0, sizeof(routine_table));
		OpenTag("subroutineDec");
		EmitXml("\n");
		char *routine_type = EmitToken().value; // constructor/function/method
		EmitToken(); // void/type
		strcpy(current_routine_name, EmitToken().value); // function name
		char routine_def[200];
		sprintf(routine_def, "%s %s.%s", routine_type, current_class_name, current_routine_name);
		EmitToken(); // (
		int nArgs = CompileParameterList();
		WriteFunction(routine_type, nArgs);
		EmitToken(); // )
		OpenTag("subroutineBody");
		EmitXml("\n");
		EmitToken(); // {
		CompileVarDec();
		CompileStatements();
		EmitToken(); // } 
		CloseTag("subroutineBody");
		CloseTag("subroutineDec");
	}
}

bool HasClassVarDec(){
	char *token_mark = token_buffer_ptr;
	char *compile_mark = compile_xml_ptr;
	return CurrentTokenValue("static") ||
		   CurrentTokenValue("field");
}

void CompileClassVarDec(){
	char *token_mark = token_buffer_ptr;
	char *compile_mark = compile_xml_ptr;
	memset(class_table, 0, sizeof(class_table));
	static_counter = 0;
	field_counter  = 0;
	char type[20];
	char name[100];
	Kind  kind;
	
	while(HasClassVarDec()){
		OpenTag("classVarDec");
		EmitXml("\n");

		if(CompareStr(EmitToken().value, "static", GetWordLength("static"))) 
			kind = STATIC;
		else 
			kind = FIELD;
		strcpy(type, EmitToken().value);
		strcpy(name, EmitToken().value);
		DefineSymbol(name, type, kind);

		while(!CurrentTokenValue(";")){ // Additional inline varNames
			EmitToken(); // ,
			strcpy(name, EmitToken().value);
			DefineSymbol(name, type, kind);
		}

		EmitToken(); // ;
		CloseTag("classVarDec");
	}
}



void CompileClass(){
	char *token_mark = token_buffer_ptr;
	char *compile_mark = compile_xml_ptr;
	DiscardToken(); // remove <tokens>
	OpenTag("class");
	EmitXml("\n");
	EmitToken(); // className
	strcpy(current_class_name, EmitToken().value); // class
	EmitToken(); // {
	CompileClassVarDec();
	CompileSubroutine();
	EmitToken(); // }
	CloseTag("class");
}

void SaveToFile(char *buffer, char *filename){
#ifdef VERBOSE
	printf("saving to file %s\n", filename);
#endif
	FILE *file;
	fopen_s(&file, filename, "w");
	if(file != NULL){
	fprintf(file, "%s", buffer);
	} else{
		printf("Error : Failed to open entry file \"%s\"- %s\n", filename, strerror(errno));
	}
}

void CompileFile(FILE *file_handle, char *filename){
#ifdef VERBOSE
    printf("Processing file %s\n", filename);
#endif
	long length;

    char file_buffer[FILE_BUFFER_SIZE]     = {0};
    char tokens_buffer[TOKENS_BUFFER_SIZE] = {0};
    char output_name[MAX_PATH]             = {0};

	char *compile_xml_buffer = malloc(OUTPUT_BUFFER_SIZE);
	char *compile_vm_buffer  = malloc(OUTPUT_BUFFER_SIZE);

	token_buffer_ptr = &tokens_buffer[0];
	compile_xml_ptr  = &compile_xml_buffer[0];
	compile_vm_ptr   = &compile_vm_buffer[0];

    char *output_name_ptr = &output_name[0];

	fseek(file_handle, 0, SEEK_END);
	length = ftell(file_handle);
	fseek(file_handle, 0, SEEK_SET);
	
	size_t bytes_to_read = fread(file_buffer, 1, length, file_handle);
	fclose(file_handle);

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
	char compile_file[256];
	
#ifdef TESTING
	CopyStr(compile_file, output_name);
	ConcatStr(compile_file, "_compile");
	ConcatStr(compile_file, ".xml");
	SaveToFile(compile_xml_buffer, compile_file);
#endif
	CopyStr(compile_file, output_name);
	ConcatStr(compile_file, ".vm");
	SaveToFile(compile_vm_buffer, compile_file);

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
	FILE *file;
	getcwd(current_dir, MAX_PATH);
		
	if(IsFile(path)){
		fopen_s(&file, path, "r");	
		if(file != NULL){
			CompileFile(file, path);
		} else{
			printf("Error : Failed to open entry file \"%s\"- %s\n", path, strerror(errno));
		}
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


