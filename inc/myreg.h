#include <regex>
using namespace std;

//regex local_lab("(\\w)");//labele pokupljene u prvom prolazu
regex local_lab("[a-zA-Z0-9]+:");//ovako radi a sa \\w nece! Pazi da ima imena koja ta labela ne sme da uzme!
//regex syscall("call");
regex directive(/*"\\.[a-z]*"*/".global|.extern|.equ");//\.[a-z\D]+
regex section_begin(".section");
regex command("halt|int|iret|call|ret|jmp|jeq|jne|jgt|push|pop|xchg|add|sub|mul|div|cmp|not|and|or|xor|test|shl|shr|ldr|str");
regex no_op_command("halt|iret|ret");
regex one_op_command("int|call|jmp|jeq|jne|jgt|push|pop|not");
regex two_op_command("xchg|add|sub|mul|div|cmp|and|or|xor|test|shl|shr|ldr|str");
regex comment("#[\\w\\s]+");
regex end_reg(".end");
regex word_reg(".word");
regex skip_reg(".skip");
regex equ_reg(".equ");
regex num_regex("[0-9]+|0[0-9]+|0[xX][0-9a-fA-F]+");
regex separator("\\s+");
regex sep_plus("\\s*\\+\\s*");
regex jumps("jmp|jeq|jne|jgt|call");
regex registers("r[0-7]|sp|pc|psw");
regex init_list_directives(".global|.extern|.word");
regex unallowed_registers("r[8-9]|r([0-9][0-9]+)");
regex data_operand("\\$\\w+|\\w+|%\\w+");
regex data_operand_register_simple("r[0-7]|sp|pc|psw|\\[r[0-7]\\]|\\[sp\\]|\\[pc\\]|\\[psw\\]");
regex data_operand_register_payload("\\[r[0-7] \\+ \\w+\\]|\\[sp \\+ \\w+\\]|\\[pc \\+ \\w+\\]|\\[psw \\+ \\w+\\]");
regex jump_operand("\\w+|%\\w+|\\*\\w+");//%w ne sme da bude literal vec samo simbol! Ovo ce biti bitno u drugom prolazu!
regex jump_operand_register_simple("\\*r[0-7]|\\*sp|\\*pc|\\*psw|\\*\\[r[0-7]\\]|\\*\\[sp\\]|\\*\\[pc\\]|\\*\\[psw\\]");//pakao za citanje
regex jump_operand_register_payload("\\*\\[r[0-7] \\+ \\w+\\]|\\*\\[sp \\+ \\w+\\]|\\*\\[pc \\+ \\w+\\]|\\*\\[psw \\+ \\w+\\]");
regex second_pass_skip(".word|.skip|.section");

regex vezba("[a-z]");
regex whitespace_only_line("^[ \\t\\r\\n\\s]*$");
regex whitespace("[\\t\\s]*");//namerno ne ukljucujem enter i \n jer ako ih ukljucim i onda osisam liniju od njega
//(za sta mi ovaj regex sluzi) onda cu sledecu liniju da prebacim u trenutnu, i preskocicu je kada opet 
//pozovvem getline() u while petlji
