#include "vmtranslator.c"
#include "test.c"
#include "utils.c"

int main(int argc, char *argv[]){
    run_tests();
    /* if(argc < 2){ */
    /*     printf("You must provide a file to parse\n"); */
    /*     return 1; */
    /* } */
    /* const char *file_path = argv[1]; */
    /* //TODO: check file extension is .vm and save filename */
    /* FILE *vm_file = LoadFile(file_path, "r"); */

    /* char line[100]; */
    /* while(fgets(line, sizeof(line), vm_file)){ */
    /*     parse_vm_code_line(line); */
    /* } */

    /* /\* strcat(result_buffer, "test1"); *\/ */

    /* /\* WriteToFile("test.txt", result_buffer); *\/ */

    /* printf("Result:\n"); */
    /* printf(result_buffer); */

    return 0;
}
