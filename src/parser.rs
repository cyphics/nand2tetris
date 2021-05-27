use crate::vmcmd::*;

pub fn parse(vm_code: &str) -> Vec<VMCmd> {
    let mut output = Vec::new();
    for line in vm_code.lines() {
        let instruction = line.trim();

        if instruction.is_empty() || instruction.starts_with("//") {
            continue;
        }
        output.push(parse_instruction(instruction));
    }
    return output;
}

fn parse_instruction(instruction: &str) -> VMCmd {
    let tokens: Vec<&str> = instruction.split(' ').collect();
    let cmd = match tokens[0] {
        "push" => VMCmd::Push(PushCmd {
            segment: String::from(tokens[1]),
            value: tokens[2]
                .parse()
                .expect("Expected a numeric value with push command"),
        }),
        "pop" => VMCmd::Pop(PopCmd {
            segment: String::from(tokens[1]),
            value: tokens[2]
                .parse()
                .expect("Expected a numeric value with pop command"),
        }),
        "add" => VMCmd::Add,
        "sub" => VMCmd::Sub,
        "neg" => VMCmd::Neg,
        "eq" => VMCmd::Eq,
        "gt" => VMCmd::Gt,
        "lt" => VMCmd::Lt,
        "and" => VMCmd::And,
        "or" => VMCmd::Or,
        "not" => VMCmd::Not,
        "return" => VMCmd::Return,
        _ => panic!("Unrecognized instruction: {}", tokens[0]),
    };
    return cmd;
}
