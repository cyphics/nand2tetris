from Helper import split_string, int_to_binary


class Decoder:
    def __init__(self, symbol_table):
        self.symbol_table = symbol_table
        self.variable_counter = 16

    def decode_line(self, encoded_line):
        if encoded_line[0] == '@':
            return self.decode_a_instruction(encoded_line)
        else:
            return self.decode_c_instruction(encoded_line)

    # def binary_list_to_string(self):
    #     output = ""
    #     for bit in self.binary_line:
    #         output += str(bit)
    #     return output

    @staticmethod
    def decode_c_instruction(instruction):
        result = ""
        dest = ''
        jmp = ''

        colon_split = instruction.split(';')
        if len(colon_split) > 1:
            jmp = colon_split[1]
        comp_dest = colon_split[0]
        comp_dest_split = comp_dest.split('=')
        if len(comp_dest_split) > 1:
            dest = comp_dest_split[0]
            comp = comp_dest_split[1]
        else:
            comp = comp_dest_split[0]
        bin_dest = destDict[dest]
        bin_comp = compDict[comp]
        bin_jmp = jumpDict[jmp]

        return "111" + bin_comp + bin_dest + bin_jmp

    def decode_a_instruction(self, instruction):
        instruction_value = instruction[1:]
        if instruction[1] != '0' and instruction[1] != '1':
            try:
                int_value = int(instruction_value)
                bin_int = int_to_binary(int_value)
                return self.format_binary_value(bin_int)
            except ValueError:
                if instruction_value not in self.symbol_table:
                    self.symbol_table[instruction_value] = self.variable_counter
                    self.variable_counter += 1
                bin_value = int_to_binary(self.symbol_table[instruction_value])
                return self.format_binary_value(bin_value)

        decimal_string = ""
        for bit in instruction_value:
            decimal_string += bit
        decimal_value = int(decimal_string)
        binary_value = int_to_binary(decimal_value)
        return self.format_binary_value(binary_value)

    def format_binary_value(self, binary_value):
        formatted_result = ""
        while len(formatted_result) + len(binary_value) < 16:
            formatted_result += "0"
        return formatted_result + binary_value


compDict = {
    '0': '0101010',
    '1': '0111111',
    '-1': '0111010',
    'D': '0001100',
    'A': '0110000',
    'M': '1110000',
    '!D': '0001101',
    '!A': '0110001',
    '!M': '1110001',
    '-D': '0001111',
    '-A': '0110011',
    '-M': '1110011',
    'D+1': '0011111',
    'A+1': '0110111',
    'M+1': '1110111',
    'D-1': '0001110',
    'A-1': '0110010',
    'M-1': '1110010',
    'D+A': '0000010',
    'D+M': '1000010',
    'D-A': '0010011',
    'D-M': '1010011',
    'A-D': '0000111',
    'M-D': '1000111',
    'D&A': '0000000',
    'D&M': '1000000',
    'D|A': '0010101',
    'D|M': '1010101'
}

destDict = {
    '': '000',
    'M': '001',
    'D': '010',
    'MD': '011',
    'A': '100',
    'AM': '101',
    'AD': '110',
    'AMD': '111'
}

jumpDict = {
    '': '000',
    'JGT': '001',
    'JEQ': '010',
    'JGE': '011',
    'JLT': '100',
    'JNE': '101',
    'JLE': '110',
    'JMP': '111'
}
