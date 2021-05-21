from unittest import TestCase

from Parser import Parser


class TestParserFirstPass(TestCase):
    def test_first_pass_nothing(self):
        file_stream = [
            "@1",
            "M=M1"
        ]
        parser = Parser(file_stream)
        parser.first_pass()
        result = parser.cleaned_list
        expected = file_stream
        self.assertEqual(result, expected)

    def test_first_pass_label(self):
        file_stream = [
            "(start)",
            "@1",
            "M=M1"
        ]
        parser = Parser(file_stream)
        parser.first_pass()
        result = parser.cleaned_list
        expected = [
            "@1",
            "M=M1"
        ]
        self.assertEqual(result, expected)

    def test_first_pass_whiteline(self):
        file_stream = [
            "(start)",
            "@1",
            "   ",
            "M=M1"
        ]
        parser = Parser(file_stream)
        parser.first_pass()
        result = parser.cleaned_list
        expected = [
            "@1",
            "M=M1"
        ]
        self.assertEqual(result, expected)

    def test_first_pass_comment(self):
        file_stream = [
            "(start)",
            "@1 // Do something   ",
            "M=M1"
        ]
        parser = Parser(file_stream)
        parser.first_pass()
        result = parser.cleaned_list
        expected = [
            "@1",
            "M=M1"
        ]
        self.assertEqual(result, expected)

    def test_first_pass_comment_in_line(self):
        file_stream = [
            "(start)",
            "@1 "
            " // Do something   ",
            "M=M1"
        ]
        parser = Parser(file_stream)
        parser.first_pass()
        result = parser.cleaned_list
        expected = [
            "@1",
            "M=M1"
        ]
        self.assertEqual(result, expected)

    def test_first_pass_start_with_comment(self):
        file_stream = [
            "// Do stuff",
            "@1 ",
            "M=M1"
        ]
        parser = Parser(file_stream)
        parser.first_pass()
        result = parser.cleaned_list
        expected = [
            "@1",
            "M=M1"
        ]
        self.assertEqual(result, expected)


class TestParserComplete(TestCase):
    def test_first_pass_basic_prog(self):
        file_stream = [
            "// This file is part of www.nand2tetris.org",
            "// and the book The Elements of Computing Systems",
            "// by Nisan and Schocken, MIT Press.",
            "// File name: projects/06/add/Add.asm",
            "",
            "// Computes R0 = 2 + 3  (R0 refers to RAM[0])",
            "",
            "@2",
            "D=A",
            "@3",
            "D=D+A",
            "@0",
            "M=D"
        ]

        parser = Parser(file_stream)
        parser.parse_file()
        result = parser.get_output()
        expected = [
            "0000000000000010",
            "1110110000010000",
            "0000000000000011",
            "1110000010010000",
            "0000000000000000",
            "1110001100001000"
        ]
        self.assertEqual(expected, result)

    def test_first_pass_small_prog(self):
        file_stream = [
            "// Adds 1 + ... + 100",
            "@i",
            "M=1 // i=1",
            "@sum",
            "M=0 // sum=0",
            "(LOOP)",
            "@i",
            "D=M // D=i",
            "@100",
            "D=D-A // D=i-100",
            "@END",
            "D;JGT // if (i-100)>0 goto END",
            "@i",
            "D=M // D=i",
            "@sum",
            "M=D+M // sum=sum+i",
            "@i",
            "M=M+1 // i=i+1",
            "@LOOP",
            "0;JMP // goto LOOP",
            "(END)",
            "@END",
            "0;JMP // infinite loop"
        ]
        parser = Parser(file_stream)
        parser.parse_file()
        result = parser.get_output()
        expected = [
            "0000000000010000",
            "1110111111001000",
            "0000000000010001",
            "1110101010001000",
            "0000000000010000",
            "1111110000010000",
            "0000000001100100",
            "1110010011010000",
            "0000000000010010",
            "1110001100000001",
            "0000000000010000",
            "1111110000010000",
            "0000000000010001",
            "1111000010001000",
            "0000000000010000",
            "1111110111001000",
            "0000000000000100",
            "1110101010000111",
            "0000000000010010",
            "1110101010000111"
        ]
        self.assertEqual(expected, result)
