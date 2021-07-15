## PSEUDO-CODE

# Load file_to_parse.vm
# Transform file in list of lines
# Send list to parser
# Retrieve translated list
# save it to file_to_parse.asm


import os
import sys

from Parser import Parser

default_file = "/home/cyphics/ownCloud/programmation/nand2tetris/projects/07/MemoryAccess/StaticTest/StaticTest.vm"


def run():
    args = sys.argv
    if len(args) > 1:
        file_to_parse = args[1]
        file_base = file_to_parse.split(".")[0]
    else:
        file_to_parse = default_file
        file_base = file_to_parse.split("/")[-1].split(".")[0]
    dir_path = os.path.dirname(os.path.realpath(file_to_parse))
    with open(file_to_parse) as f:
        content = f.readlines()
    content = [x.strip() for x in content]

    # content = [
    #     "push pointer 0"
    # ]
    parser = Parser()
    parser.load_instructions(content, file_base)
    result = parser.instruc_list
    output = ""
    for line in result:
        output += line + "\n"

    output_file_path = dir_path + "/" + file_base + ".asm"
    output_file = open(output_file_path, 'w')
    output_file.write(output)


run()
