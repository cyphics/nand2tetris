import os
import sys

from Parser import Parser

default_file = "/home/cyphics/ownCloud/programmation/nand2tetris/projects/06/pong/Pong.asm"


def run():
    args = sys.argv
    if len(args) > 1:
        file_to_parse = args[1]
    else:
        file_to_parse = default_file
    dir_path = os.path.dirname(os.path.realpath(file_to_parse))
    with open(file_to_parse) as f:
        content = f.readlines()
    content = [x.strip() for x in content]
    parser = Parser(content)
    parser.parse_file()
    result = parser.get_output()
    output = ""
    for line in result:
        output += line + "\n"

    output_file_path = dir_path + "/result.hack"
    output_file = open(output_file_path, 'w')
    output_file.write(output)


run()
