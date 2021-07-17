pub struct Parser {}
impl Parser {
    fn parse_instruction(line: &str) {}
}

fn get_file_content(filename: String) -> String {
    return std::fs::read_to_string(filename).expect("Error reading file content");
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_add() {
        let result = Parser::parse_instruction("add");
    }
}
