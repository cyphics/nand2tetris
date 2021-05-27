use std::env;
use vmtranslator::Setup;

fn main() {
    let args = env::args();
    let setup = Setup::new(args);
    vmtranslator::run(setup);
}
