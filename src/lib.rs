pub mod converter;
pub mod parser;
pub mod vmcmd;

use std::env;

pub struct Setup {}
impl Setup {
    pub fn new(_args: env::Args) -> Setup {
        Setup {}
    }
}

pub fn run(_setup: Setup) {
    let codes = parser::parse("push constant 8");
    let _assembler = converter::Assembler::new(codes);
}
