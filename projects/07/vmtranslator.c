#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "vmtranslator.h"
#include "utils.h"


void add_cmd(char *line){
    strcat(result_buffer, line);
}

void add_commented_line(char *line){
    printf("Parsing line \"%s\"\n", line);
    char commented_line[100];
    sprintf(commented_line, "\n// %s\n", line);
    add_cmd(commented_line);
}

void parse_lexical_item(char *item){
    if(item != NULL){
        printf("Parsing item %s\n", item);

    }
}

bool is_line_comment(){
    if(sizeof(current_line) < 2) return false;
    if(current_line[0] == '/' && current_line[1] == '/') return true;
    return false;
}

void set_flag(CommandType c_type){

}

bool are_strings_equal(char *first, char *second){
    return strcmp(first, second) == 0;
}

void get_next_token(){
    if(are_strings_equal(current_line, current_line_original)){
        token_ptr = strtok(current_line, " ");
    } else {
        token_ptr = strtok(NULL, " ");
    }
    /* char *ptr = strtok(current_line, " "); */
    strcpy(token, token_ptr);
}

void goto_stack(){
    add_cmd("@SP\n");
    add_cmd("A=M\n");
}

void increment_stack(){
    add_cmd("@SP\n");
    add_cmd("M=M+1\n");
}

void decrement_stack(){
    add_cmd("@SP\n");
    add_cmd("M=M-1\n");
}


void add_push_command(){
    /* @value // First temp store the pushed value
       D=A
       @SP    // Then put it on stack
       A=M
       M=D
       @SP    // Finally, increment stack
       M=M+1
    */
    get_next_token(); // token now holds the memory segment
    if(are_strings_equal(token, "constant")){
        get_next_token();
        add_cmd("@");
        add_cmd(token);
        add_cmd("\n");
    } else {
        get_next_token();
        add_cmd("@static_");
        add_cmd(token);
        add_cmd("\n");
    }
    add_cmd("D=A\n");
    goto_stack();
    add_cmd("M=D\n");
    increment_stack();
}

void add_pop_command(char *line){
    goto_stack();

    // Get value on top of the stack

    // Get the address to store the value

    // Store the value there

    decrement_stack();
}

void parse_vm_code_line(char *line){
    strcpy(current_line_original, line);
    strcpy(current_line, line);
    if(is_line_comment()) return;

    add_commented_line(line);

    get_next_token();

    // test arithmetics
    if (are_strings_equal(token, "add")){
        decrement_stack();
        goto_stack();
        add_cmd("D=M");
        decrement_stack();
        goto_stack();
        add_cmd("M=D+M");
        increment_stack();
    }
    else if (are_strings_equal(token, "sub")){
        decrement_stack();
        goto_stack();
        add_cmd("D=M");
        decrement_stack();
        goto_stack();
        add_cmd("M=D-M");
        increment_stack();
    }
    else if (are_strings_equal(token, "neg")){

    }
    else if (are_strings_equal(token, "eq")){

    }
    else if (are_strings_equal(token, "gt")){

    }
    else if (are_strings_equal(token, "lt")){

    }
    else if (are_strings_equal(token, "and")){

    }
    else if (are_strings_equal(token, "or")){

    }
    else if (are_strings_equal(token, "not")){

    }

    // memory management
    else if(are_strings_equal(token, "push")){
        add_push_command();
    }
    else if (are_strings_equal(token, "pop")){

    }

}
