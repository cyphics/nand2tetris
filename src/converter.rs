use crate::vmcmd::*;

pub struct Assembler {
    filename: String,
    assembly: String,
    label_counter: usize,
}

impl Assembler {
    pub fn new(filename: String) -> Assembler {
        Assembler {
            assembly: String::new(),
            filename,
            label_counter: 0,
        }
    }

    pub fn convert(&mut self, cmd_list: Vec<VMCmd>) -> String {
        for cmd in cmd_list.iter() {
            self.translate_cmd(cmd);
        }
        return String::from(&self.assembly);
    }

    fn translate_cmd(&mut self, cmd: &VMCmd) -> String {
        match cmd {
            VMCmd::Add => self.commit_binary_operation("D+M", "add"),
            VMCmd::Sub => self.commit_binary_operation("M-D", "sub"),
            VMCmd::Or => self.commit_binary_operation("M|D", "or"),
            VMCmd::And => self.commit_binary_operation("M&D", "and"),
            VMCmd::Neg => self.commit_unary_operation("-M", "neg"),
            VMCmd::Not => self.commit_unary_operation("!M", "not"),
            VMCmd::Eq => self.commit_comparison("JEQ", "eq"),
            VMCmd::Gt => self.commit_comparison("JGT", "gt"),
            VMCmd::Lt => self.commit_comparison("JLT", "lt"),
            VMCmd::Return => {}
            VMCmd::Push(push) => self.commit_push(push),
            VMCmd::Pop(pop) => self.commit_pop(pop),
        }

        String::new()
    }

    fn commit(&mut self, s: &str) {
        self.assembly.push_str(s);
        self.assembly.push('\n');
    }

    fn commit_comment(&mut self, comment: &str) {
        self.commit(&format!("\n//{}", comment));
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

    fn commit_push(&mut self, cmd: &PushCmd) {
        self.commit_comment(&format!("push {} {}", cmd.segment, cmd.value));
        match cmd.segment.as_ref() {
            "local" | "argument" | "this" | "that" => {
                let register = match cmd.segment.as_ref() {
                    "local" => "LCL",
                    "argument" => "ARG",
                    "this" => "THIS",
                    "that" => "THAT",
                    _ => panic!("Unknown segment {}", cmd.segment),
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
                    _ => panic!("Wrong pointer value {}", cmd.value),
                };
                self.commit(&format!("@{}", register));
                self.commit("D=M");
                self.commit_d_to_stack();
                self.commit_increment_stack();
            }

            "temp" => {
                // RAM[5-12]
                assert!(
                    0 < cmd.value && cmd.value < 6,
                    "Temp value {} overflow",
                    cmd.value
                );
                self.commit(&format!("@{}", cmd.value + 5));
                self.commit("D=M");
                self.commit_d_to_stack();
                self.commit_increment_stack();
            }

            _ => panic!("Unknown segment : {}", cmd.segment),
        }
    }

    fn commit_pop(&mut self, cmd: &PopCmd) {
        self.commit_comment(&format!("pop {} {}", cmd.segment, cmd.value));
    }

    fn commit_const_to_d(&mut self, value: i16) {
        self.commit(&format!("@{}", value));
        self.commit("D=A");
    }

    fn commit_d_to_stack(&mut self) {
        self.commit("@SP");
        self.commit("A=M");
        self.commit("M=D");
    }

    fn commit_increment_stack(&mut self) {
        self.commit("@SP");
        self.commit("M=M+1");
    }
}
