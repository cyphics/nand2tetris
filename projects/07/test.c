#include <assert.h>
#include <string.h>
#include "vmtranslator.h"

#define size_list_items 100
#define size_item 40
char input_buffer[1000];
char list_items_first[size_list_items][size_item];
char list_items_second[size_list_items][40];
int first_item_length = 30;

void build_list_items(char *input, char **vector_list){
    memset(list_items_first, 0, sizeof(list_items_first));
    memset(list_items_second, 0, sizeof(list_items_second));
    strcpy(input_buffer, input);
    char *ptr = strtok(input_buffer, "\n");
    int idx = 0;
    vector_list[idx] = ptr;
    while(ptr != NULL){
        idx++;
        ptr = strtok(NULL, "\n");
        vector_list[idx] = ptr;
        printf(vector_list[idx]);
        printf("\n");
    }
    printf("--\n");

}

void print_string_diff(char *first, char *second){
    printf("Printing string diff:\n");
    char line_to_print[40];
    build_list_items(first, list_items_first);
    build_list_items(second, list_items_second);
    /* return; */
    for(int i = 0; i<size_list_items; i++){
        char *item1 = list_items_first[i];
        char *item2 = list_items_second[i];
        printf(item1);
        printf("--");
        printf(item2);

        /* if(item1 != 0 || item2 != 0){ */
        /*     /\* strcpy(line_to_print, item1); *\/ */
        /*     printf(item1); */
        /*     for(int j = 0; j<(first_item_length - sizeof(item1)); j++){ */
        /*         printf(" "); */
        /*     } */
        /*     printf("---  "); */

        /*     printf(item2); */
        /*     printf("\n"); */
        /* } */
        printf(line_to_print);
    }
}
void test_strings(char *first, char *second, char *error_message){
    if(!are_strings_equal(first, second)){
        printf("Error with test \"%s\"\n", error_message);
        print_string_diff(first, second);

        /* printf("Expected:\n %s\n", first); */
        /* printf("Result:\n %s\n", second); */
    }
}
void test_parse_push_line(){
    parse_vm_code_line("push constant 4");

    test_strings("\n// push constant 4\n@4\nD=A\n@SP\nA=M\nM=D\n@SP\nM=M+1", result_buffer, "test_parse_push_line");

}


void run_tests(){
    test_parse_push_line();
}
