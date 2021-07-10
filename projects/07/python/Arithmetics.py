class Arithmetics:
    def __init__(self, instructions_list, stack):
        self.if_counter = 0
        self.stack = stack
        self.instr_list = instructions_list

    def instr(self, instruction):
        self.instr_list.append(instruction)

    def decode(self, parts):
        if parts[0] == "add":
            self.add()
        if parts[0] == "sub":
            self.sub()
        if parts[0] == "eq":
            self.test_equal()
        if parts[0] == "lt":
            self.test_lower_than()
        if parts[0] == "gt":
            self.test_greater_than()
        if parts[0] == "neg":
            self.neg()
        if parts[0] == "and":
            self.et()
        if parts[0] == "or":
            self.ou()
        if parts[0] == "not":
            self.non()

    def add(self):
        self.stack.decrement()
        self.stack.goto()
        self.instr("D=M")
        self.stack.decrement()
        self.stack.goto()
        self.instr("M=D+M")
        self.stack.increment()

    def sub(self):
        self.stack.decrement()
        self.stack.goto()
        self.instr("D=M")
        self.stack.decrement()
        self.stack.goto()
        self.instr("M=M-D")
        self.stack.increment()

    def apply_comparator(self, comparator):
        self.init_comparison()
        count = str(self.if_counter)
        self.instr("@TRUE_" + count)
        self.instr("D;" + comparator)
        self.instr("D=0")
        self.instr("@THEN_" + count)
        self.instr("0;" + "JMP")
        self.instr("(TRUE_" + count + ")")
        self.instr("D=-1")
        self.instr("(THEN_" + count + ")")
        self.stack.goto()
        self.instr("M=D")
        self.stack.increment()
        self.if_counter += 1

    def init_comparison(self):
        # At the end, D == stack[n-1] - stack[n]
        self.stack.decrement()
        self.stack.goto()
        self.instr("D=M")
        self.stack.decrement()
        self.stack.goto()
        self.instr("D=M-D")

    def test_equal(self):
        self.apply_comparator("JEQ")

    def test_lower_than(self):
        self.apply_comparator("JLT")

    def test_greater_than(self):
        self.apply_comparator("JGT")

    def neg(self):
        self.stack.decrement()
        self.stack.goto()
        self.instr("D=M")
        self.instr("M=-D")
        self.stack.increment()

    def et(self):
        self.stack.decrement()
        self.stack.goto()
        self.instr("D=M")
        self.stack.decrement()
        self.stack.goto()
        self.instr("M=D&M")
        self.stack.increment()

    def ou(self):
        self.stack.decrement()
        self.stack.goto()
        self.instr("D=M")
        self.stack.decrement()
        self.stack.goto()
        self.instr("M=D|M")
        self.stack.increment()

    def non(self):
        self.stack.decrement()
        self.stack.goto()
        self.instr("D=M")
        self.instr("M=!D")
        self.stack.increment()
