#ifndef REGEXES_HPP
#define REGEXES_HPP

#include <string>
#include <regex>

using namespace std;

// match any single character except new line, 
regex regex_input_file_extension("^.*.s$");
regex regex_ouput_file_extension("^.*.o$");

// one or more occurence of white_character
regex regex_empty_line("^\\s*$");
regex regex_additionalSpace(" {2,}|\\t");
regex regex_punctuationSpace(" ?, ?");

regex regex_for_label("^([a-zA-Z_][a-zA-Z0-9_]*):$");
regex regex_for_global("^\\.global [a-zA-Z_][a-zA-Z0-9_]*(, [a-zA-Z_][a-zA-Z0-9_]*)*$");
regex regex_for_extern("^\\.extern [a-zA-Z_][a-zA-Z0-9_]*(, [a-zA-Z_][a-zA-Z0-9_]*)*$");
regex regex_for_skip_hex_literal("^\\.skip 0x[0-9A-Fa-f]+$");
regex regex_for_skip_dec_positive_literal("^\\.skip [0-9]+$");
regex regex_for_skip_dec_negative_literal("^\\.skip -[0-9]+$");
regex regex_for_section("^\\.section [a-zA-Z_][a-zA-Z0-9_]*$");
regex regex_for_end("^(\\.end)|(\\.END)$");

regex regex_for_word("^\\.word [a-zA-Z_][a-zA-Z0-9_]*(, [a-zA-Z_][a-zA-Z0-9_]*)*$");
regex regex_for_word_single("^[a-zA-Z_][a-zA-Z0-9_]*|[0-9]*|-[0-9]*|0x[0-9A-Fa-f]+$");

regex regex_word_1("^0x[0-9A-Fa-f]+$");
regex regex_word_2("^[0-9]+$");
regex regex_word_3("^-[0-9]+$");
regex regex_word_4("^[a-zA-Z_][a-zA-Z0-9_]*$");

regex regex_for_instruction_with_no_operands("^(halt|iret|ret)$");
regex regex_for_instruction_with_one_operand("^(push|pop|not|int) (r[0-7]|psw|sp|pc)$");
regex regex_for_one_operand_jump_instruction("^(call|jmp|jeq|jne|jgt) (.*)$");
regex regex_for_instruction_with_two_operands_ldr_str("^(ldr|str) (r[0-7]|sp|pc|psw),(.*)$");
regex regex_for_other_instructions_with_two_operands("(xchg|add|sub|mul|div|cmp|and|or|xor|test|shl|shr) (r[0-7]|psw|pc|sp), (r[0-7]|psw|pc|sp)");

string single_word = "[a-zA-Z_][a-zA-Z0-9_]*|[0-9]*|-[0-9]*|0x[0-9A-Fa-f]+";
string jmp_absolute =  "[a-zA-Z_][a-zA-Z0-9_]*|[0-9]*|0x[0-9A-Fa-f]+";
string jmp_memdir = "[0-9]*|0x[0-9A-Fa-f]+|[a-zA-Z_][a-zA-Z0-9_]*";
regex regex_for_absolute_addr_ldr_str("^\\$(" + single_word + ")$");
regex regex_for_regdir_addr_ldr_str("^(r[0-7]|pc|sp|psw)$");
regex regex_for_regind_addr_ldr_str("^\\[(r[0-7]|pc|sp|psw)\\]$");
regex regex_for_pc_rel_addr_ldr_str("^%([a-zA-Z_][a-zA-Z0-9_]*)$");
regex regex_for_regind_with_disp_addr_ldr_str("^\\[(r[0-7]|psw|pc|sp) \\+ ([a-zA-Z_][a-zA-Z0-9_]*|[0-9]*|-[0-9]*|0x[0-9A-Fa-f]+)\\]$");
regex regex_for_memdir_addr_ldr_str("^(a-zA-Z_][a-zA-Z0-9_]*|[0-9]*|0x[0-9A-Fa-f]+)$");

regex regex_for_pc_rel_jump_instructions("^%([a-zA-Z_][a-zA-Z0-9_]*)$");
regex regex_for_regdir_jump_instructions("^\\*(r[0-7]|psw|pc|sp)$");
regex regex_for_regind_jump_instructions("^\\*\\[(r[0-7]|psw|pc|sp)\\]$");
regex regex_for_absolute_jump_instructions("^(" + jmp_absolute + ")$");
regex regex_for_memdir_jump_instructions("^\\*(" + jmp_memdir + ")$");
regex regex_for_regind_with_disp_jump_instructions("^\\*\\[(r[0-7]|psw|pc|sp) \\+ (" + single_word + ")\\]$");

#endif