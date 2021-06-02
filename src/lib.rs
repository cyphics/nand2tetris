pub mod code_writer;
pub mod parser;
pub mod vmcmd;

use std::env;
use std::fs;
use std::fs::metadata;
use std::fs::File;
use std::io::Write;
use std::path::Path;
use std::path::PathBuf;
use std::process;

pub struct Setup {
    pub input     : Vec<String>,
    pub input_path: String,
    pub output    : String,
    pub nocomment : bool,
}
impl Setup {
    pub fn new() -> Setup {
        let input_files: Vec<String>;
        let mut output_file;
        let args: Vec<String> = env::args().collect();
        check_input_validity(&args);
        let input_path = &args[1];
        if metadata(input_path).unwrap().is_dir() {
            input_files = fs::read_dir(input_path)
                             .unwrap()
                             .map(|entry| entry.unwrap().path())
                             .filter(|path| path.is_file())
                             .filter(|path| path.extension().expect("No file found ending in .vm in this directory") == "vm")
                             .map(|path| path.to_str().unwrap().replace("./", "").to_string())
                             .collect();

            let dir_path = PathBuf::from(&input_path)
                                   .canonicalize()
                                   .unwrap();
            let dir_name = dir_path.file_name()
                                   .unwrap()
                                   .to_str()
                                   .unwrap();
            
            let output_path = dir_path.to_str().unwrap();
            output_file = String::from(output_path);
            output_file.push_str(&format!("/{}.asm", dir_name));
        } else {
            input_files = vec![String::from(input_path)];
            let mut input_file = PathBuf::from(input_path);
            input_file.set_extension("asm");
            output_file = String::from(input_file.to_str().unwrap());
        };

        let mut nocomment = false;
        if args.len() >= 3 {
            if args[2] == "--nocomment" {
                nocomment = true;
            }
        }

        Setup {
            input: input_files,
            input_path: String::from(input_path),
            output: output_file,
            nocomment
        }
    }
}

fn check_input_validity(args: &Vec<String>) {
    // Check that input path is given
    if args.len() < 2 {
        println!("No input file/directory provided");
        process::exit(1);
    };

    // Check that input path exists
    let input_path_raw = Path::new(&args[1]);
    let meta = metadata(input_path_raw).unwrap();
    if !input_path_raw.exists() {
        println!("Error. Provided input file/dir does not exist. Exiting...\n");
        process::exit(1);
    };
    if meta.is_file() {
        assert_eq!(input_path_raw.extension().unwrap(), "vm");
    } else if meta.is_dir() {
        println!("Loading directory.\n");
    }
}

pub fn run(setup: Setup) {
    let mut output_file = File::create(&setup.output).expect("Could not create output file");
    if fs::metadata(&setup.input_path).unwrap().is_dir() {
        let mut assembler = code_writer::Assembler::new(String::from("bootstrap"), setup.nocomment);
        assembler.commit_bootstrap(&mut output_file);
    }

    for file in setup.input {
        let vm_code = fs::read_to_string(&file).expect(&format!("Could not read from file: {}", file));
        let vm_cmds = parser::parse(&vm_code);
        let mut assembler = code_writer::Assembler::new(file, setup.nocomment);
        let assembly = assembler.convert(vm_cmds);
        output_file
            .write_all(assembly.as_bytes())
            .expect("Unable to write assembly code to file");
    }
}

