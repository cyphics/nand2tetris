use crate::vmcmd::*;

pub fn parse(vm_code: &str) -> Vec<VMCmd> {
    let mut output       = Vec::new();
    let mut line_counter = 1;

    for line in vm_code.lines() {
        let instruction = line.trim()
                              .split("//")
                              .next()
                              .unwrap()
                              .trim();
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
        "push"     => VMCmd::Push(
                             PushCmd {
                                segment: String::from(tokens[1]),
                                value:   tokens[2]
                                          .trim()
                                          .parse()
                                          .unwrap_or_else(|_|panic!(
                                            "Error at line {}, Expected a numeric value with push command, got '{}' instead"
                                         , line_counter, tokens[2])),
                             },
                             line_counter,),
        "pop"      => VMCmd::Pop(
                             PopCmd {
                                segment: String::from(tokens[1]),
                                value: tokens[2]
                                 .trim()
                                 .parse()
                                 .unwrap_or_else(|_|panic!(
                                         "Error at line {}, Expected a numeric value with pop command, got {} instead"
                                         , line_counter, tokens[2])),
                             },
                             line_counter,),
        "add"      => VMCmd::Add(line_counter),
        "sub"      => VMCmd::Sub(line_counter),
        "neg"      => VMCmd::Neg(line_counter),
        "eq"       => VMCmd::Eq(line_counter),
        "gt"       => VMCmd::Gt(line_counter),
        "lt"       => VMCmd::Lt(line_counter),
        "and"      => VMCmd::And(line_counter),
        "or"       => VMCmd::Or(line_counter),
        "not"      => VMCmd::Not(line_counter),
        "label"    => VMCmd::Label( String::from(tokens[1]), line_counter),
        "goto"     => VMCmd::Goto( String::from(tokens[1]), line_counter),
        "if-goto"  => VMCmd::IfGoto(String::from(tokens[1]), line_counter),
        "function" => VMCmd::Function(
                             FunctionCmd{
                                 function_name: String::from(tokens[1]),
                                 local_args: tokens[2]
                                     .parse()
                                     .unwrap_or_else(|x|panic!(
                                             "Error at line {} : Expected numeric value for local argument number of function {}, got {} instead", 
                                             line_counter, tokens[1], x)),
                             },
                             line_counter),
        "call"     => VMCmd::Call(
                             CallCmd{
                                 function_name: String::from(tokens[1]),
                                 args: tokens[2]
                                     .parse()
                                     .unwrap_or_else(|x|panic!(
                                             "Error at line {} : Expected numeric value for argument number of function {}, got {} instead"
                                             , line_counter, tokens[1], x)),
                             },
                             line_counter),
        "return"   => VMCmd::Return(line_counter),
        _ => panic!("Unrecognized instruction: {}", tokens[0]),
    };
    return cmd;
}
