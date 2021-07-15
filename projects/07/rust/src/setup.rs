use std::env;
use std::fs;
use std::fs::metadata;
use std::path::Path;
use std::path::PathBuf;
use std::process;

pub struct Setup {
    pub input: Vec<String>,
    pub output: String,
}

impl Setup {
    pub fn generate() -> Setup {
        let args: Vec<String> = env::args().collect();
        let input_files;
        let mut output_file;
        check_input_validity(&args);
        if metadata(&args[1]).unwrap().is_dir() {
            input_files = fs::read_dir(&args[1])
                .unwrap()
                .map(|entry| entry.unwrap().path())
                .filter(|path| path.is_file())
                .filter(|path| path.extension().unwrap() == "vm")
                .map(|path| path.to_str().unwrap().to_string())
                .collect();

            let dir_name = Path::new(&args[1]).file_name().unwrap().to_str().unwrap();
            output_file = String::from(dir_name);
            output_file.push_str(".asm");
        } else {
            input_files = vec![String::from(&args[1])];
            let mut input_file = PathBuf::from(&args[1]);
            input_file.set_extension("asm");
            output_file = String::from(input_file.to_str().unwrap());
        };

        Setup {
            input: input_files,
            output: output_file,
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
