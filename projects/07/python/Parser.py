from Arithmetics import Arithmetics
from Stack import Stack


class Parser:
    def __init__(self):
        self.instruc_list = []
        self.if_counter = 0
        self.stack = Stack(self.instruc_list)
        self.arithm = Arithmetics(self.instruc_list, self.stack)

    def load_instructions(self, instructions_list, file_name=""):
        for instr in instructions_list:
            if instr[0:2] == "//":
                pass
            elif instr.isspace() or not instr:
                pass
            else:
                parts = instr.split(" ")
                if parts[0] in arithmetic_codes:
                    self.arithm.decode(parts)
                elif parts[0] == "push":
                    self.push(parts[1], parts[2])
                elif parts[0] == "pop":
                    self.pop(parts[1], parts[2])
        self.close()

    def instr(self, instruction):
        self.instruc_list.append(instruction)

    def push(self, segment, value):
        if segment == "constant":
            self.push_constant(value)
        elif segment == "temp":
            self.push_temp(value)
        elif segment == "pointer":
            if value == "0":
                mem = "R3"
            else:
                mem = "R4"
            self.instr("@" + mem)
            self.instr("D=M")
            # Put value in stack
            self.stack.goto()
            self.instr("M=D")
            self.stack.increment()
        elif segment == "static":
            self.instr("@" + "static_" + value)
            self.instr("D=M")
            # Put value in stack
            self.stack.goto()
            self.instr("M=D")
            self.stack.increment()
        else:
            self.push_segment(segment, value)

    def push_segment(self, segment, value):
        # Compute where to take data (mem + value)
        mem = get_memory_starter(segment)  # mem = "R1", "R2", etc
        self.instr("@" + mem)
        self.instr("D=M")
        self.instr("@" + value)
        self.instr("D=D+A")
        self.instr("A=D")
        self.instr("D=M")
        # Put value in stack
        self.stack.goto()
        self.instr("M=D")
        self.stack.increment()

    def push_temp(self, value):
        self.instr("@5")
        self.instr("D=A")
        self.instr("@" + value)
        self.instr("D=D+A")
        self.instr("A=D")
        self.instr("D=M")
        # Put value in stack
        self.stack.goto()
        self.instr("M=D")
        self.stack.increment()

    def push_constant(self, value):
        self.instr("@" + str(value))
        self.instr("D=A")
        self.stack.goto()
        self.instr("M=D")
        self.stack.increment()

    def pop(self, segment, value):
        # Compute where to store data (mem + value)
        if segment == "temp":
            self.instr("@5")
            self.instr("D=A")
            self.instr("@" + value)
            self.instr("D=D+A")
            self.instr("@addr")
            self.instr("M=D")
        elif segment == "pointer":
            if value == "0":
                mem = "R3"
            else:
                mem = "R4"
            self.instr("@" + mem)
            self.instr("D=A")
            self.instr("@addr")
            self.instr("M=D")
        elif segment == "static":
            self.instr("@" + "static_" + value)
            self.instr("D=A")
            self.instr("@addr")
            self.instr("M=D")
        else:
            mem = get_memory_starter(segment)  # mem = "R1", "R2", etc
            self.instr("@" + mem)
            self.instr("D=M")
            self.instr("@" + value)
            self.instr("D=D+A")
            self.instr("@addr")
            self.instr("M=D")

        self.store_stack_in_address()

    def store_stack_in_address(self):
        # Get value of stack
        self.stack.decrement()
        self.stack.goto()
        self.instr("D=M")
        # Put value in location
        self.instr("@addr")
        self.instr("A=M")
        self.instr("M=D")

    def close(self):
        self.instr("(END)")
        self.instr("@END")
        self.instr("0;JMP")


def get_memory_starter(segment):
    if segment == "local":
        return "R1"
    if segment == "argument":
        return "R2"
    if segment == "this":
        return "R3"
    if segment == "that":
        return "R4"
    if segment == "temp":
        return "R5"


arithmetic_codes = [
    "add",
    "sub",
    "neg",
    "eq",
    "gt",
    "lt",
    "and",
    "or",
    "not"
]
