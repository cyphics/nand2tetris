use crate::vmcmd::*;

pub struct Assembler {
    filename: String,
    label_index: u32,
    assembly: String,
    label_counter: usize,
}

impl Assembler {
    pub fn new(filename: String) -> Assembler {
        Assembler {
            assembly: String::new(),
            filename,
            label_counter: 0,
            label_index: 0
        }
    }

    pub fn convert(&mut self, cmd_list: Vec<VMCmd>) -> String {
        for cmd in cmd_list.iter() {
            self.translate_cmd(cmd);
        }
        return String::from(&self.assembly);
    }

    fn translate_cmd(&mut self, cmd: &VMCmd) {
        match cmd {
            VMCmd::Add(_)                  => self.commit_binary_operation("D+M", "add"),
            VMCmd::Sub(_)                  => self.commit_binary_operation("M-D", "sub"),
            VMCmd::Or(_)                   => self.commit_binary_operation("M|D", "or"),
            VMCmd::And(_)                  => self.commit_binary_operation("M&D", "and"),
            VMCmd::Neg(_)                  => self.commit_unary_operation("-M", "neg"),
            VMCmd::Not(_)                  => self.commit_unary_operation("!M", "not"),
            VMCmd::Eq(_)                   => self.commit_comparison("JEQ", "eq"),
            VMCmd::Gt(_)                   => self.commit_comparison("JGT", "gt"),
            VMCmd::Lt(_)                   => self.commit_comparison("JLT", "lt"),
            VMCmd::Push(push, line)        => self.commit_push(push, *line),
            VMCmd::Pop(pop, line)          => self.commit_pop(pop, *line),
            VMCmd::Label(label, _)         => self.commit_label(label),
            VMCmd::Goto(label, _)          => self.commit_goto(label),
            VMCmd::IfGoto(label, _)        => self.commit_if_goto(label),
            VMCmd::Function(fct_cmd, line) => self.commit_function_declaration(fct_cmd, *line),
            VMCmd::Call(call_cmd, line)    => self.commit_function_call(call_cmd, *line),
            VMCmd::Return(_)               => self.commit_return(),
        }
    }

    fn commit(&mut self, s: &str) {
        self.assembly.push_str(s);
        self.assembly.push('\n');
    }

    fn commit_comment(&mut self, comment: &str) {
        self.commit(&format!("\n// {}", comment));
    }

    fn commit_d_to_register(&mut self, reg: &str) {
        self.commit(&format!("@{}", reg));
        self.commit("M=D");
    }

    fn commit_register_to_d(&mut self, reg: &str) {
        self.commit(&format!("@{}", reg));
        self.commit("D=M");
    }

    fn commit_const_to_d(&mut self, value: i16) {
        self.commit(&format!("@{}", value));
        self.commit("D=A");
    }

    fn commit_stack_to_d(&mut self) {
        self.commit("@SP");
        self.commit("A=M");
        self.commit("D=M");
    }

    fn commit_d_to_stack(&mut self) {
        self.commit("@SP");
        self.commit("A=M");
        self.commit("M=D");
    }

    fn commit_decrement_stack(&mut self) {
        self.commit("@SP");
        self.commit("M=M-1");
    }

    fn commit_increment_stack(&mut self) {
        self.commit("@SP");
        self.commit("M=M+1");
    }

    fn commit_register_to_stack(&mut self, reg: &str){
        self.commit(&format!("@{}", reg));
        self.commit("D=M");
        self.commit_d_to_stack();
        self.commit_increment_stack();
    }



    fn commit_binary_operation(&mut self, operation: &str, comment: &str) {
        self.commit_comment(comment);
        self.commit("@SP");
        self.commit("AM=M-1");
        self.commit("D=M");
        self.commit("A=A-1");
        self.commit(&format!("M={}", operation));
    }

    fn commit_unary_operation(&mut self, operation: &str, comment: &str) {
        self.commit_comment(comment);
        self.commit("@SP");
        self.commit("A=M-1");
        self.commit(&format!("M={}", operation));
    }

    fn commit_comparison(&mut self, operation: &str, comment: &str) {
        // first, do D == stack[n-1] - stack[n]
        let true_label = format!(
            "_TRUE_{}_{}_{}",
            self.filename, operation, self.label_counter
        );
        let false_label = format!(
            "_FALSE_{}_{}_{}",
            self.filename, operation, self.label_counter
        );

        self.commit_binary_operation("M-D", comment);
        self.commit("D=M");
        self.commit(&format!("@{}", true_label));
        self.commit(&format!("D;{}", operation));
        self.commit("@SP");
        self.commit("A=M-1");
        self.commit("M=0");
        self.commit(&format!("@{}", false_label));
        self.commit("0;JMP");
        self.commit(&format!("({})", true_label));
        self.commit("@SP");
        self.commit("A=M-1");
        self.commit("M=-1");
        self.commit(&format!("({})", false_label));
        self.label_counter += 1;
    }

    fn commit_push(&mut self, cmd: &PushCmd, line_number: u32) {
        self.commit_comment(&format!("push {} {}", cmd.segment, cmd.value));
        match cmd.segment.as_ref() {
            "local" | "argument" | "this" | "that" => {
                let register = match cmd.segment.as_ref() {
                    "local" => "LCL",
                    "argument" => "ARG",
                    "this" => "THIS",
                    "that" => "THAT",
                    _ => panic!(
                        "Error at line {} : \"push {} {}\" -- Unknown segment {}",
                        line_number, cmd.segment, cmd.value, cmd.value
                    ),
                };
                self.commit(&format!("@{}", register));
                self.commit("D=M");
                self.commit(&format!("@{}", cmd.value));
                self.commit("A=D+A");
                self.commit("D=M");
                self.commit_d_to_stack();
                self.commit_increment_stack();
            }
            "constant" => {
                self.commit_const_to_d(cmd.value);
                self.commit_d_to_stack();
                self.commit_increment_stack();
            }
            "static" => {
                self.commit(&format!("@_static_{}_{}", self.filename, cmd.value));
                self.commit("D=M");
                self.commit_d_to_stack();
                self.commit_increment_stack();
            }
            "pointer" => {
                let register = match cmd.value {
                    0 => "THIS",
                    1 => "THAT",
                    _ => panic!("Wrong pointer value {}, line {}", cmd.value, line_number),
                };
                self.commit(&format!("@{}", register));
                self.commit("D=M");
                self.commit_d_to_stack();
                self.commit_increment_stack();
            }

            "temp" => {
                // RAM[5-12]
                assert!(
                    0 < cmd.value && cmd.value < 9,
                    "Error at line {} : \"push temp {}\" -- Temp value {} overflow",
                    line_number,
                    cmd.value,
                    cmd.value
                );
                self.commit(&format!("@{}", cmd.value + 5));
                self.commit("D=M");
                self.commit_d_to_stack();
                self.commit_increment_stack();
            }

            _ => panic!("Unknown segment : {}, line {}", cmd.segment, line_number),
        }
    }

    fn commit_pop(&mut self, cmd: &PopCmd, line_number: u32) {
        self.commit_comment(&format!("pop {} {}", cmd.segment, cmd.value));
        match cmd.segment.as_ref() {
            "temp" => {
                self.commit_const_to_d(5);
                self.commit(&format!("@{}", cmd.value));
                self.commit("D=D+A");
                self.commit_d_to_register("R13");
                self.commit_decrement_stack();
                self.commit_stack_to_d();
                self.commit("@R13");
                self.commit("A=M");
                self.commit("M=D");
            }
            "static" => {
                self.commit_decrement_stack();
                self.commit_stack_to_d();
                self.commit(&format!("@_static_{}_{}", self.filename, cmd.value));
                self.commit("M=D");
            }
            "local" | "argument" | "this" | "that" => {
                let register = match cmd.segment.as_ref() {
                    "local" => "LCL",
                    "argument" => "ARG",
                    "this" => "THIS",
                    "that" => "THAT",
                    _ => panic!(
                        "Error at line {} : \"push {} {}\" -- Unknown segment {}",
                        line_number, cmd.segment, cmd.value, cmd.value
                    ),
                };
                self.commit_register_to_d(register);
                self.commit(&format!("@{}", cmd.value));
                self.commit("D=D+A");
                self.commit_d_to_register("R13");
                self.commit_decrement_stack();
                self.commit_stack_to_d();
                self.commit("@R13");
                self.commit("A=M");
                self.commit("M=D");
            }
            "pointer" => {
                let register = match cmd.value {
                    0 => "THIS",
                    1 => "THAT",
                    _ => panic!("Wrong pointer value {}, line {}", cmd.value, line_number),
                };
                self.commit_decrement_stack();
                self.commit_stack_to_d();
                self.commit(&format!("@{}", register));
                self.commit("M=D");
            }

            "constant" => {
                panic!(
                    "Error at line {} : \"pop constant {}\" -- pop constant not authorized.",
                    line_number, cmd.value
                )
            }

            _ => {
                panic!(
                    "Error at line {} : \"pop {} {}\" -- Segment {} not recognized.",
                    line_number, cmd.segment, cmd.value, cmd.segment
                )
            }
        }
    }

    fn commit_label(&mut self, label: &str){
        self.commit_comment(&format!("label {}", label));
        self.commit(&format!("({})", label));
    }

    fn commit_goto(&mut self, label: &str){
        self.commit_comment(&format!("goto {}", label));
        self.commit(&format!("@{}", label));
        self.commit("0; JMP");
    }

    fn commit_if_goto(&mut self, label: &str) {
        self.commit_comment(&format!("if-goto {}", label));
        self.commit_decrement_stack();
        self.commit_stack_to_d();
        self.commit(&format!("@{}", label));
        self.commit("D; JNE");
    }

    fn commit_function_declaration(&mut self, fct_cmd: &FunctionCmd, line_number: u32){
        self.commit_comment(&format!("declaration of fn {}.{} {}", self.filename, fct_cmd.function_name, fct_cmd.local_args));
        let fct_label = get_function_label(&fct_cmd.function_name, &self.filename);
        self.commit(&format!("({})", fct_label));
        for _ in 0..fct_cmd.local_args{
            self.commit_push(&PushCmd{
                                segment: String::from("constant"),
                                value: 0
                             }, line_number);
        }
    }

    fn commit_function_call(&mut self, call_cmd: &CallCmd, line_number: u32){
        self.commit_comment(&format!("call of fn {}.{} {}", self.filename, call_cmd.function_name, call_cmd.args));
        let func_label = get_function_label(&call_cmd.function_name, &self.filename);
        let return_address = format!("__{}$return.{}__", call_cmd.function_name, self.label_index);

        // Push return-address
        self.commit(&format!("@{}", return_address));
        self.commit("D=A");
        self.commit_d_to_stack();
        self.commit_increment_stack();

        // Save caller's frame
        self.commit_register_to_stack("LCL");
        self.commit_register_to_stack("ARG");
        self.commit_register_to_stack("THIS");
        self.commit_register_to_stack("THAT");

        // ARG = SP-n-5
        self.commit_register_to_d("SP");
        self.commit("@5");
        self.commit("D=D-A");
        self.commit(&format!("@{}", call_cmd.args));
        self.commit("D=D-A");
        self.commit_d_to_register("ARG");

        // LCL = SP
        self.commit_register_to_d("SP");
        self.commit_d_to_register("LCL");

        // goto called function
        self.commit(&format!("@{}", func_label));
        self.commit("0;JMP");

        // set return address label
        self.commit_label(&return_address);

        self.label_index += 1;
    }

    fn commit_return(&mut self){}
}

fn get_function_label(fct_name: &str, filename: &str) -> String{
    return format!("__fn_{}_{}__", fct_name, filename);
}
