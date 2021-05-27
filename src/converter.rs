use crate::vmcmd::*;

pub struct Assembler {
    filename: String,
    cmd: Vec<VMCmd>,
    assembly: String,
    label_counter: usize,
}

impl Assembler {
    pub fn new(cmd_list: Vec<VMCmd>) -> Assembler {
        Assembler {
            cmd: cmd_list,
            assembly: String::new(),
            filename: String::new(),
            label_counter: 0,
        }
    }

    pub fn convert(&mut self, cmd_list: Vec<VMCmd>) -> String {
        for cmd in cmd_list.iter() {
            self.translate_cmd(cmd);
        }
        return String::new();
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
            VMCmd::Push(push) => {}
            VMCmd::Pop(pop) => {}
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
}
