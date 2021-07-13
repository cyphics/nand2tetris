class Stack:
    def __init__(self, instr_list):
        self.instr_list = instr_list

    def instr(self, instruction):
        self.instr_list.append(instruction)

    def increment(self):
        self.instr("@R0")
        self.instr("M=M+1")

    def decrement(self):
        self.instr("@R0")
        self.instr("M=M-1")

    def goto(self):
        self.instr("@R0")
        self.instr("A=M")
