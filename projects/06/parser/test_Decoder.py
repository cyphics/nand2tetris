from unittest import TestCase

from Decoder import Decoder
from Helper import split_string
from Parser import symb_table


class TestDecoder(TestCase):
    def setUp(self) -> None:
        self.decoder = Decoder(symb_table)

    def test_decode_line(self):
        line_to_decode = "@0"
        decoded_line = self.decoder.decode_line(line_to_decode)
        expected = "0000000000000000"
        self.assertEqual(decoded_line, expected)


class TestAInstruction(TestCase):
    def setUp(self) -> None:
        self.decoder = Decoder(symb_table)

    def test_decode_a_instruction_0(self):
        a_instr = "@0"
        decoded = self.decoder.decode_a_instruction(a_instr)
        self.assertEqual(decoded, "0000000000000000")

    def test_decode_a_instruction_1(self):
        a_instr = "@1"
        decoded = self.decoder.decode_a_instruction(a_instr)
        self.assertEqual(decoded, "0000000000000001")

    def test_decode_a_instruction_10(self):
        a_instr = "@10"
        decoded = self.decoder.decode_a_instruction(a_instr)
        self.assertEqual(decoded, "0000000000001010")

    def test_decode_a_instruction_var(self):
        decoded1 = self.decoder.decode_a_instruction("@var")
        decoded2 = self.decoder.decode_a_instruction("@foo")
        self.assertEqual(decoded2, "0000000000010001")


class TestCInstruction(TestCase):
    def setUp(self) -> None:
        self.decoder = Decoder(symb_table)

    def test_decode_c_instruction_0(self):
        a_instr = "M=0"
        decoded = self.decoder.decode_c_instruction(a_instr)
        self.assertEqual("1110101010001000", decoded)

    def test_decode_c_instruction_1(self):
        a_instr = "D=D-A"
        decoded = self.decoder.decode_c_instruction(a_instr)
        self.assertEqual("1110010011010000", decoded)

    def test_decode_c_instruction_2(self):
        a_instr = "0;JMP"
        decoded = self.decoder.decode_c_instruction(a_instr)
        self.assertEqual("1110101010000111", decoded)
