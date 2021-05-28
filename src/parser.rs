use crate::vmcmd::*;

pub fn parse(vm_code: &str) -> Vec<VMCmd> {
    let mut output = Vec::new();
    let mut line_counter = 1;
    for line in vm_code.lines() {
        let instruction = line.trim();

        if instruction.is_empty() || instruction.starts_with("//") {
            line_counter += 1;
            continue;
        }
        output.push(parse_instruction(instruction, line_counter));
        line_counter += 1;
    }
    return output;
}

fn parse_instruction(instruction: &str, line_counter: u32) -> VMCmd {
    let tokens: Vec<&str> = instruction.split(' ').collect();
    let cmd = match tokens[0] {
        "push" => VMCmd::Push(
            PushCmd {
                segment: String::from(tokens[1]),
                value: tokens[2]
                    .parse()
                    .expect("Expected a numeric value with push command"),
            },
            line_counter,
        ),
        "pop" => VMCmd::Pop(
            PopCmd {
                segment: String::from(tokens[1]),
                value: tokens[2]
                    .parse()
                    .expect("Expected a numeric value with pop command"),
            },
            line_counter,
        ),
        "add" => VMCmd::Add(line_counter),
        "sub" => VMCmd::Sub(line_counter),
        "neg" => VMCmd::Neg(line_counter),
        "eq" => VMCmd::Eq(line_counter),
        "gt" => VMCmd::Gt(line_counter),
        "lt" => VMCmd::Lt(line_counter),
        "and" => VMCmd::And(line_counter),
        "or" => VMCmd::Or(line_counter),
        "not" => VMCmd::Not(line_counter),
        "return" => VMCmd::Return(line_counter),
        _ => panic!("Unrecognized instruction: {}", tokens[0]),
    };
    return cmd;
}
