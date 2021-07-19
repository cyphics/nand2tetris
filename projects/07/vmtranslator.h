/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Thierry Raeber $
   ======================================================================== */
#if !defined(VMTRANSLATOR_H)

#define VMTRANSLATOR_H
#define C_COMMAND (1 << 0)
#define A_COMMAND (1 << 1)

#define result_buffer_size 1000000
char result_buffer[result_buffer_size];
char token[20];
char current_line[100];
char current_line_original[100];
char *token_ptr;

typedef enum {
    C_ARITHMETIC = 0,
    C_PUSH,
    C_POP,
    C_LABEL,
    C_GOTO,
    C_IF,
    C_FUNCTION,
    C_RETURN,
    C_CALL
} CommandType;

void add_cmd(char *line);
void add_commented_line(char *line);
void parse_vm_code_line(char *line);
#endif
