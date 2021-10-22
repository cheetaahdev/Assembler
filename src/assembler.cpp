#include <iostream>
#include <fstream>
#include <iomanip>
#include "../inc/assembler.h"
#include "../inc/myreg.h"


void Assembler::cut_ws_before(string *s)
{
    while (isspace((*s)[0]))
        (*s).erase(0, 1);
}

void Assembler::cut_ws_after(string *s)
{
    while((*s).back()==' ')
            (*s).pop_back();
}

void Assembler::cut_r_after(string *s)
{
    while((*s).back()=='\r')
            (*s).pop_back();
}

string Assembler::cut_comment(string s)
{
    smatch comment_matches;
    string org = s;
    string content = "";
    if (regex_search(s, comment_matches, comment))
    {
        cut_ws_before(&s);

        while (!(org.at(0) == '#'))
        {
            content.push_back(org.front());
            org.erase(0, 1);
        }
        while(content.back()==' ')
            content.pop_back();

        return content;
    }
    else
        return s;
}

bool Assembler::check_string_table(string s)
{
    for (string_table_element ste : string_table)
    {
        if (ste.value == s)
            return true;
    }

    return false;
}

int Assembler::check_string_table_index(string s)
{
    for (string_table_element ste : string_table)
    {
        if (ste.value == s)
            return ste.index;
    }

    return -1;
}

int Assembler::get_st_value(int i)
{
    for (symbol_table_element ste : symbol_table)
    {
        if (ste.name == i)
            return ste.value;
    }
    return -1;
}

void Assembler::relocation_patch(string rel_type, int offset, int index, int addend){
    
    rel_table_element rte{current_section, offset, rel_type, index, addend};
    relocation_table.push_back(rte);
}


void Assembler::tokenize(string s)
{
    auto const vec = std::vector<std::string>(
        std::sregex_token_iterator{begin(s), end(s), separator, -1},
        std::sregex_token_iterator{});

    current_line_tokens = vec;
}

void Assembler::location_counter_increase(string s)
{
    string par = cut_comment(s);
    cut_ws_before(&par);
    cut_ws_after(&par);    
    tokenize(par);

    bool long_instruction = false;//postoji reg. ind. adresiranje sa pomerajem, pa je tokenize razbije na 5 tokena, zato se ovako zove
    if(regex_search(par, simple_matches, data_operand_register_payload) || regex_search(par, simple_matches, jump_operand_register_payload))
    {
        long_instruction = true;
    }
    if ((long_instruction && current_line_tokens.size() > 5)
    || (!long_instruction && current_line_tokens.size() > 3))
    {
        cout << "Wrong instruction format at line " << line_cnt << endl;
        ::exit(8);
    }

    if (!(regex_match(current_line_tokens[0], simple_matches, command) |
          regex_match(current_line_tokens[0], simple_matches, directive)))
    {
        cout << "Error: unallowed token present in line " << line_cnt << endl;
        ::exit(3);
    }

    if (regex_match(current_line_tokens[0], simple_matches, no_op_command))
    {
        if (current_line_tokens.size() != 1)
        {
            cout << "Wrong instruction format at line " << line_cnt << endl;
            ::exit(8);
        }

        location_counter++;
    }

    if (regex_match(current_line_tokens[0], simple_matches, one_op_command))
    {
        if ((!long_instruction && current_line_tokens.size() != 2)
        || (long_instruction && current_line_tokens.size() != 4))
        {
            cout << "Wrong instruction format at line " << line_cnt << endl;
            ::exit(8);
        }

        if (regex_match(current_line_tokens[0], simple_matches, jumps))
        {
            if (!long_instruction && regex_match(current_line_tokens[1], simple_matches, jump_operand_register_simple))
                location_counter += 3;
            else if (long_instruction || regex_match(current_line_tokens[1], simple_matches, jump_operand))
                if(regex_search(current_line_tokens[1], simple_matches, unallowed_registers))
                {
                    cout << "Wrong instruction format at line " << line_cnt << endl;
                    ::exit(8);
                }
                else
                    location_counter += 5;
            // else if (!long_instruction && regex_match(current_line_tokens[2], simple_matches, data_operand_register_simple))
            //     location_counter += 3;//ostavio sam stari redosled ako zatreba
            else
            {
                cout << "Syntax error: bad addressing at line " << line_cnt << endl;
                ::exit(12);
            }

        }

        else //int, push, pop, not
        {
            if (!regex_match(current_line_tokens[1], simple_matches, registers))
            {
                cout << "Wrong instruction format at line " << line_cnt << endl;
                ::exit(8);
            }
            if (current_line_tokens[0] == "int" || current_line_tokens[0] == "not")//hardcode ali bas me briga
                location_counter += 2;
            else
                location_counter += 3;
        }
    }

    if (regex_match(current_line_tokens[0], simple_matches, two_op_command))
    {
        if ((!long_instruction && current_line_tokens.size() != 3)
        || (long_instruction && current_line_tokens.size() != 5))
        {
            cout << "Wrong instruction format at line " << line_cnt << endl;
            ::exit(8);
        }
        if (current_line_tokens[1].back() != ',')
        {
            cout << "Syntax error: missing ',' character at line " << line_cnt << endl;
            ::exit(10);
        }

        current_line_tokens[1].pop_back();

        if(!regex_match(current_line_tokens[1], simple_matches, registers))
        {
            cout << "Wrong instruction format at line " << line_cnt << endl;
            ::exit(8);
        }

        if (current_line_tokens[0] != "ldr" && current_line_tokens[0] != "str")
        {
            if (!regex_match(current_line_tokens[2], simple_matches, registers))
            {
                cout << "Wrong instruction format at line " << line_cnt << endl;
                ::exit(8);
            }
            else
                location_counter += 2;
        }
        else
        {
            if (!long_instruction && regex_match(current_line_tokens[2], simple_matches, data_operand_register_simple))
                location_counter += 3;
            else if (long_instruction || regex_match(current_line_tokens[2], simple_matches, data_operand))
                if(regex_match(current_line_tokens[2], simple_matches, unallowed_registers))
                {
                    cout << "Wrong instruction format at line " << line_cnt << endl;
                    ::exit(8);
                }
                else
                    location_counter += 5;
            // else if (!long_instruction && regex_match(current_line_tokens[2], simple_matches, data_operand_register_simple))
            //     location_counter += 3;//ostavio sam stari redosled ako zatreba
            else
            {
                cout << "Syntax error: bad addressing at line " << line_cnt << endl;
                ::exit(12);
            }
        }
    }
    cout << "Location counter has value " << location_counter << " after line " << line_cnt << endl;
}

bool Assembler::directive_processing(string first_word, string temp_line)
{
    short current_iter_sym_cnt = 0;
    //OBRADA ZA PRVU REC .WORD------------------------------------------------------
    if (regex_match(first_word, defining_matches, word_reg))
    {
        if(current_section=="UND"){
            cout<<">Word direktiva mora se naći u sekciji!"<<endl;
            ::exit(15);
        }
        word_defined_symbols.clear();
        current_iter_sym_cnt = 0;
        string curr_symbol = "";

        temp_line = cut_comment(temp_line);
        cut_ws_before(&temp_line);
        cut_ws_before(&temp_line);
        cut_ws_after(&temp_line);
        
        while (!(temp_line == ""))
        {
            curr_symbol.push_back(temp_line.front());
            temp_line.erase(0, 1);
            if (temp_line == "")
            {
                if(current_iter_sym_cnt > 0)
                    curr_symbol.erase(0,1);//brise razmak
                this->word_defined_symbols.push_back(curr_symbol);
                current_iter_sym_cnt++;
                curr_symbol = "";
            }
            else if (temp_line.at(0) == ',' && temp_line.length() > 1)
            {
                if(current_iter_sym_cnt > 0)
                    curr_symbol.erase(0,1);
                this->word_defined_symbols.push_back(curr_symbol);
                current_iter_sym_cnt++;
                temp_line.erase(0, 1);
                curr_symbol = "";
            }
            else if (temp_line.at(0) == ',' && temp_line.length() == 1)
            {
                cout << "Pogresno navedena inicijalizatorska lista na liniji " << line_cnt << endl;
                ::exit(5);
            }
        }

        for (short i = 0; i < current_iter_sym_cnt; i++)
        {
            int ind = check_string_table_index(word_defined_symbols[i]);

            if (ind!=-1)
            {
                if (symbol_table[ind].info == 'G')
                {
                    //treba da se pravi rel. zapis jer njegovu adresu ne znam!
                    //da li ovo raditi u prvom ili u drugom prolazu?
                    //U drugom prolazu.
                }
                
                //ako je simbol global ne radim nista, cekam da bude neki definisan kao L kasnije da mogu da mu azuriram value
                //ako je definisan kao L onda se vrednost simbola azurira u drugom prolazu
                // cout << "Symbol at line " << line_cnt << " has already been defined!" << endl;
                // ::exit(6);
            }
            else if (!regex_match(word_defined_symbols[i], simple_matches, num_regex))
            {
                symbol_table_element st_entry{sym_table_index, location_counter, 8, 'U', 'N', current_section};
                symbol_table.push_back(st_entry);

                string_table_element stringt_entry{sym_table_index, word_defined_symbols[i], 0};
                string_table.push_back(stringt_entry);

                sym_table_index++;
            }

            location_counter += 2;
        }

        return true; //idi na sledecu liniju, ovde si gotov, ove continue bi bilo dobro da izbacim.
    }
//OBRADA ZA .EQU
    else if (regex_match(first_word, defining_matches, equ_reg))
    {
        cut_ws_before(&temp_line);
        cut_ws_after(&temp_line);
        temp_line = cut_comment(temp_line);
        tokenize(temp_line);

        if(current_line_tokens.size()!=2)
        {
            cout << "Wrong instruction format at line " << line_cnt << endl;
            ::exit(8);
        }
        if (current_line_tokens[0].back() != ',')
        {
            cout << "Syntax error: missing ',' character at line " << line_cnt << endl;
            ::exit(10);
        }
        if(!regex_match(current_line_tokens[1], simple_matches, num_regex))
        {
            cout << "Syntax error: number expected but not provided at line " << line_cnt << endl;
            ::exit(11);
        }
        
        current_line_tokens[0].pop_back();
        
        int ind = check_string_table_index(current_line_tokens[0]);

        bool word = false;

        if(ind!=-1)
        {
            if(symbol_table[ind].info=='G' && symbol_table[ind].section=="UND")
            {
                symbol_table[ind].section="ABS";

                string literal = current_line_tokens[1];
                uint32_t num = 0;
                if (literal.substr(0, 1) != "0")
                {
                    num = stoi(literal);
                }
                else if (literal.substr(0, 2) == "0x" || literal.substr(0, 2) == "0X")
                {
                    num = stoi(literal, 0, 16);
                }
                else if (literal.substr(0, 1) == "0")
                {
                    num = stoi(literal, 0, 8);
                }

                symbol_table[ind].value=num;
            }
            else if(symbol_table[ind].info=='U')
            {
                word = true;
            }
            else
            {
                cout << "Symbol at line " << line_cnt << " is already defined." << endl;
                ::exit(6);
            }
        }

        else
        {
            string literal = current_line_tokens[1];
            uint32_t num = 0;
            if (literal.substr(0, 1) != "0")
            {
                num = stoi(literal);
            }
            else if (literal.substr(0, 2) == "0x" || literal.substr(0, 2) == "0X")
            {
                num = stoi(literal, 0, 16);
            }
            else if (literal.substr(0, 1) == "0")
            {
                num = stoi(literal, 0, 8);
            }

            if(word){
                symbol_table[ind].value = num;
                symbol_table[ind].section="ABS";
                symbol_table[ind].info='L';
            }
            else
            {
                symbol_table_element st_entry{sym_table_index, num, 8, 'A', 'N', "ABS"}; //'ABS' jer uvek pripada sekciji apsolutnih simbola.
                symbol_table.push_back(st_entry);

                string_table_element stringt_entry{sym_table_index, current_line_tokens[0], 0};
                string_table.push_back(stringt_entry);

                //uint32_t index = (uint32_t)check_string_table_index(v);
                // literal_pool_element lpe{sym_table_index, num};
                // literal_pool.push_back(lpe);
                //Predomislio sam se oko koriscenja ove literal_poola ali nek ostane kod ako zatreba!
                sym_table_index++;
            }
        }
        
    }

    //OBRADA ZA .SKIP------------------------------------------------------
    else if (regex_match(first_word, defining_matches, skip_reg))
    {
        
        cut_ws_before(&temp_line);
        temp_line = cut_comment(temp_line);
        //radim u slucaju da je ulazni fajl ispravan sintaksno, imam jedan broj iza .skip
        //ovde valja uvesti tokenizaciju ako stignes
        if (regex_match(temp_line, simple_matches, num_regex))
        {
            int num;
            if (temp_line.substr(0, 1) != "0")
            {
                num = stoi(temp_line);
            }
            else if (temp_line.substr(0, 2) == "0x")
            {
                num = stoi(temp_line, 0, 16);
            }
            else if (temp_line.substr(0, 1) == "0")
            {
                num = stoi(temp_line, 0, 8);
            }
            location_counter += num;
        }
        else
        {
            cout << "Skip directive at line" << line_cnt << " contains unallowed parameter";
            ::exit(7);
        }

        label_previous_line = false;
        //line_cnt++;
        return true; //idi na sledecu liniju, ovde si gotov
    }
    //OBRADA AKO JE PRVA REC .SECTION
    else if (regex_search(curr_line, simple_matches, section_begin) && regex_match(first_word, section_matches, section_begin))
    {
        
        new_section_begins = true;
        location_counter = 0;
        cut_ws_before(&curr_line);
        cut_ws_after(&curr_line);
        curr_line = cut_comment(curr_line);
        tokenize(curr_line);

        if(current_line_tokens.size()!=2)
        {
            cout << "Wrong instruction format at line " << line_cnt << endl;
            ::exit(8);
        }

        if (regex_match(current_line_tokens[1], section_matches, local_lab) |
            regex_match(current_line_tokens[1], section_matches, command) |
            regex_match(current_line_tokens[1], section_matches, directive) |
            regex_match(current_line_tokens[1], section_matches, end_reg) |
            regex_match(current_line_tokens[1], section_matches, word_reg) |
            regex_match(current_line_tokens[1], section_matches, skip_reg) |
            regex_match(current_line_tokens[1], section_matches, section_begin))
        {
            cout << "Error: unallowed section name in line " << line_cnt << endl;
            ::exit(2);
        }

        string section_name = current_line_tokens[1];
        this->current_section = section_name;
        
        if (!check_string_table(current_line_tokens[1]))
        {
            symbol_table_element st_entry{sym_table_index, location_counter, 8, 'L', 'N', current_section};
            symbol_table.push_back(st_entry);

            string_table_element stringt_entry{sym_table_index, section_name, 0};
            string_table.push_back(stringt_entry);

            sym_table_index++;
            curr_line = "";

            return true;
        }
        else
        {
            int n = check_string_table_index(current_line_tokens[1]);
            if(!(symbol_table[n].info=='U' || (symbol_table[n].info=='G' && symbol_table[n].section=="UND")))
            {//ovde mozda nije 100% ispravno
                cout << "Symbol at line " << line_cnt << " is already defined." << endl;
                ::exit(6);
            }
        }
    }
    else if(first_word==".global" || first_word==".extern")
    {
        cut_ws_before(&temp_line);
        cut_ws_after(&temp_line);
        temp_line = cut_comment(temp_line);
        tokenize(temp_line);

        for(int i=0; i<current_line_tokens.size(); i++)
        {
            if(current_line_tokens[i].back()==',')
                current_line_tokens[i].pop_back();

            if (regex_match(current_line_tokens[i], section_matches, local_lab) |
            regex_match(current_line_tokens[i], section_matches, command) |
            regex_match(current_line_tokens[i], section_matches, directive) |
            regex_match(current_line_tokens[i], section_matches, end_reg) |
            regex_match(current_line_tokens[i], section_matches, word_reg) |
            regex_match(current_line_tokens[i], section_matches, skip_reg) |
            regex_match(current_line_tokens[i], section_matches, section_begin))
            { //mozda ne moras sve ove da zabranis, otom potom
                cout << "Error: unallowed symbol name in line " << line_cnt << endl;
                ::exit(2);
            }

            char info = (first_word==".global") ? 'G' : 'E';

            int ind = check_string_table_index(current_line_tokens[i]);

            if(ind!=-1)
            {
                if((info == 'E' &&  symbol_table[ind].info!='U') || symbol_table[ind].info=='E' || symbol_table[ind].info=='G' /*|| (symbol_table[ind].info=='L' && info=='E')*/)//Da li je ovaj uslov kompletan vidi debagovanjem
                {
                    cout << "Symbol " << current_line_tokens[i] << " at line " << line_cnt << " has already been defined!" << endl;
                    ::exit(13);
                }
                else{
                    symbol_table[ind].info = 'G';//simbol koji je bio lokalni sad postaje globalni; ova if grana verovatno nije 100% tacna!
                }
            }
            else
            {
                symbol_table_element st_entry{sym_table_index, 0, 8, info, 'N', "UND"};
                symbol_table.push_back(st_entry);

                string_table_element stringt_entry{sym_table_index, current_line_tokens[i], 0};
                string_table.push_back(stringt_entry);

                sym_table_index++;
            }
        }
    return true;
    }

    return false;
}

string Assembler::print_hex_number(int num){
    stringstream ss;
    ss << std::hex  << num;
    string res = ss.str();
    if (res.size() < 4)
        res.insert(0, 4 - res.size(), '0');
    else while(res.size() > 4)
    {
        res.erase(0,1);//jako nepravilno ali nek izgleda lepo za sad
    }
    for (auto &c : res)
        c = toupper(c);

    return res;
}



void Assembler::first_pass(ifstream &doc)
{
    this->location_counter = 0;
    this->current_section = "UND";
    bool wss = false;
    
    while (!doc.eof())
    {//promeniti uslov; ovo je petlja koja obradjuje jednu liniju
    //ovde valja da probas sa citanjem iz stream da vidis da li je sledeci karakter eof, jer uslov u petlji vrati true tek nakon sto probas da procitas iz fajla; ili to ili menjaj uslov!
        getline(doc, this->curr_line); //curr_line ne sadrzi \n!

        if (regex_match(curr_line, white_matches, whitespace_only_line))
        {
            line_cnt++;
            continue;
        }

        cut_r_after(&curr_line);
        cut_ws_before(&curr_line);
        if (regex_match(curr_line, end_match, end_reg))
        {
            cout << "End directive reached at line " << line_cnt << endl;
            break;
        }

        if (curr_line.at(0) == '#')
        {
            cout << "Na liniji " << line_cnt << " nalazi se komentar" << endl;
            line_cnt++;
            continue;
        }

        string first_word = "", temp_line = curr_line; //first_word je prva rec u liniji, ovde je izdvajam pa gledam je li labela

        while (!(isspace(temp_line.front()) || temp_line == ""))
        {
            first_word.push_back(temp_line.front());
            temp_line.erase(0, 1);
        }

        wss = directive_processing(first_word, temp_line);

        if (wss)
        {
            if (new_section_begins)
            {
                new_section_begins = false;
            }
            
        }
        else
        {
            if (regex_search(first_word, lab_matches, local_lab))
            {
                if (label_previous_line)
                {
                    cout << "Error: more than one label defined in line " << line_cnt << endl;
                    ::exit(1);
                }

                temp_line = curr_line;

                while (regex_search(temp_line, lab_matches, local_lab))
                {
                    label_cnt++;
                    temp_line = lab_matches.suffix();
                }
                if (label_cnt == 1)
                {
                    first_word.pop_back();
                    int ind = check_string_table_index(first_word);

                    if (ind!=-1)
                    {
                        if(symbol_table[ind].info=='U' || (symbol_table[ind].info=='G' && symbol_table[ind].section=="UND")){

                            if(symbol_table[ind].info=='U')
                                symbol_table[ind].info = 'L';
                            
                            symbol_table[ind].section = current_section;
                            symbol_table[ind].value = location_counter;

                        }
                        else if (symbol_table[ind].info=='A' || (symbol_table[ind].info=='G' && symbol_table[ind].section=="ABS") || symbol_table[ind].info=='E')//ova linije je sumnjiva ali za sada nema problema.
                        {
                            cout << "Label at line " << line_cnt << " is already defined or declared as extern" << endl; 
                            ::exit(1);
                        }
                    }
                    else
                    {
                        string label = first_word;

                        symbol_table_element st_entry{sym_table_index, location_counter, 8, 'L', 'N', current_section};
                        symbol_table.push_back(st_entry);

                        string_table_element stringt_entry{sym_table_index, label, 0};
                        string_table.push_back(stringt_entry);

                        sym_table_index++;
                        label_previous_line = true;
                        
                    }//valjda sam dobro stavio else kraj

                    cut_ws_before(&temp_line);
                    curr_line = cut_comment(temp_line);

                    if (temp_line != "")
                    {
                        first_word = "";
                        while (!(isspace(temp_line.front()) || temp_line == ""))
                        {
                            first_word.push_back(temp_line.front());
                            temp_line.erase(0, 1);
                        }
                        if (!directive_processing(first_word, temp_line))
                            location_counter_increase(curr_line);

                        label_previous_line = false;
                    }

                    label_cnt = 0;
                    
                    line_cnt++;
                    continue; //samo cu ovde ostaviti continue jer tu mora da se postavi label_previous_line = true;
                }
                else
                {
                    cout << "Error: more than one label defined in line " << line_cnt << endl;
                    ::exit(1);
                }
            }
            //dole: prva rec nije ni wss ni labela
            else if (!(regex_match(first_word, section_matches, command) |
                       regex_match(first_word, section_matches, directive)))
            {
                cout << "Error: unallowed token present in line " << line_cnt << endl;
                ::exit(3); //zapisi negde kodove gresaka, ako je ovo uopste kod greske
                //ovo je dobro odradilo posao, u fajlu code.txt prepoznalo je da je mov nedozvoljena instrukcija :) Ali ne radi za .string!
            }
            //OBRADA AKO PRVA REC NIJE NI LABELA, NI .WORD, NI .SKIP, NI .SECTION, TJ. OBRADA UVECANJA LOCATION COUNTERA
            else
            {
                location_counter_increase(curr_line);
            }
        }
        cout<< "Location counter has value "<< location_counter << " after line " << line_cnt << endl;//radi testa, brisi kad zavrsis
        label_previous_line=false;
        line_cnt++;
        
    }

    doc.close();
}
//Sta se desava u prvom prolazu kada naidjem na .extern? Trebalo bi nista, jer se prvi prolaz odnosi na lokalno def. simbole (ovo mozda nije tacno...)
//Mozda ubaciti u nekom trenutku mehanizam bacanja gresaka.

void Assembler::second_pass(ifstream &doc, ofstream &out)
{
    string instruction_code;
    string op_code;
    string dest_reg_number;
    string src_reg_number;
    string addr;
    string payload;

    location_counter = 0;
    line_cnt = 1;
    current_section = "UND";


    while (!doc.eof())
    {
        getline(doc, this->curr_line);
        instruction_code = "";
        if (regex_match(curr_line, white_matches, whitespace_only_line))
        {
            line_cnt++;
            continue;
        }

        cut_ws_before(&curr_line); //ova metoda radi posao! :)
        cut_ws_after(&curr_line);
        

        if (regex_match(curr_line, end_match, end_reg))
        {
            cout << "End directive reached at line " << line_cnt << endl;
            break;
        }

        if (curr_line.at(0) == '#')
        {
            cout << "Na liniji " << line_cnt << " nalazi se komentar" << endl;
            line_cnt++;
            continue;
        }

        curr_line = cut_comment(curr_line);

        string first_word = "", temp_line = curr_line; //upotrebljivost temp_line je pod znakom pitanja

        tokenize(curr_line);
        first_word = current_line_tokens[0];
        if(current_line_tokens[1].back()==',')
            current_line_tokens[1].pop_back(); //da se odmah resim zareza!
        //temp_line = ostatak, hmm

        bool rip = false;//postoji reg. ind. adresiranje sa pomerajem
        if (regex_search(curr_line, simple_matches, data_operand_register_payload) || regex_search(curr_line, simple_matches, jump_operand_register_payload))
        {
            rip = true;
        }

        if ((!rip && current_line_tokens.size() > 3 && !regex_match(first_word, simple_matches, local_lab) && !regex_match(first_word, simple_matches, init_list_directives))
        ||  ( rip && current_line_tokens.size() > 5 && !regex_match(first_word, simple_matches, local_lab) && !regex_match(first_word, simple_matches, init_list_directives)))
        {
            cout << "Wrong instruction format at line " << line_cnt << endl;
            ::exit(8);
        }

        if (regex_match(first_word, simple_matches, local_lab))
        {
            if ((!rip && current_line_tokens.size() > 4)
            ||   (rip && current_line_tokens.size() > 6))
            {
                cout << "Wrong instruction format at line " << line_cnt << endl;
                ::exit(8);
            }
            else
            {
                current_line_tokens.erase(current_line_tokens.begin());
                first_word = current_line_tokens[0];
            }
        }

        if (regex_match(first_word, simple_matches, second_pass_skip))//direktive
        {
            if (first_word == ".skip")
            {
                int num = 0;
                if (current_line_tokens[1].substr(0, 1) != "0")
                {
                    num = stoi(current_line_tokens[1]);
                }
                else if (current_line_tokens[1].substr(0, 2) == "0x")
                {
                    num = stoi(current_line_tokens[1], 0, 16);
                }
                else if (current_line_tokens[1].substr(0, 1) == "0")
                {
                    num = stoi(current_line_tokens[1], 0, 8);
                }

                for(int i=0; i<num; i++)
                {
                    instruction_code += "00";
                    location_counter++;
                }
            }
            else if (first_word == ".word")
            {
                int num = 0;
                for(int i=1; i<current_line_tokens.size(); i++)
                {
                    if(current_line_tokens[i].back()==',')
                        current_line_tokens[i].pop_back();
                    
                    if(regex_match(current_line_tokens[i], simple_matches, num_regex))
                    {
                        
                        if (current_line_tokens[i].substr(0, 1) != "0")
                        {
                            num = stoi(current_line_tokens[i]);
                        }
                        else if (current_line_tokens[i].substr(0, 2) == "0x" || current_line_tokens[i].substr(0, 2) == "0X")
                        {
                            num = stoi(current_line_tokens[i], 0, 16);
                        }
                        else if (current_line_tokens[i].substr(0, 1) == "0")
                        {
                            num = stoi(current_line_tokens[i], 0, 8);
                        }
                        
                        string org = print_hex_number(num);
                        string s1 = org.substr(0,2);
                        string s2 = org.substr(2,2);
                        org = s2 + s1;
                        instruction_code.append(org).append(" ");                      
                    }
                    else
                    {
                        
                        num = check_string_table_index(current_line_tokens[i]);

                        if(num==-1 || symbol_table[num].info=='U' || (symbol_table[num].section=="UND" && symbol_table[num].info=='G'))//znaci nije nadjen, ili pronadjen ali nije azuriran nigde u prvom prolazu ovaj info
                        {
                            cout << "Symbol " << current_line_tokens[i] << " at line " << line_cnt << " hasn't been defined!" << endl;
                            ::exit(13);
                        }

                        else
                        {
                            if (num != -1 && symbol_table[num].section != "ABS")//pazi na ovo mesto!
                            {
                                int offset = location_counter;
                                int addend = 0;
                                relocation_patch(REL, offset, num, addend);
                            }

                            string org = print_hex_number(symbol_table[num].value);
                            string s1 = org.substr(0, 2);
                            string s2 = org.substr(2, 2);
                            org = s2 + s1;
                            instruction_code.append(org).append(" ");

                            //instruction_code.append(print_hex_number(symbol_table[num].value)).append(" ");
                        }
                    }

                    location_counter+=2;
                }
            }
            else if (first_word==".section"){
                current_section = current_line_tokens[1];
                location_counter = 0;
                instruction_code = current_section;
            }

        }

        if (regex_match(first_word, simple_matches, command))
        {
            if (regex_match(first_word, simple_matches, no_op_command))
            {
                if (current_line_tokens.size() != 1)
                {
                    cout << "Wrong instruction format at line " << line_cnt << endl;
                    ::exit(8);
                }

                if (first_word == "halt")
                {
                    instruction_code = "00";
                }
                if (first_word == "iret")
                {
                    instruction_code = "20";
                }
                if (first_word == "ret")
                {
                    instruction_code = "40";
                }

                location_counter++;
            }


            if (regex_match(first_word, simple_matches, one_op_command))
            {
                // if (current_line_tokens.size() != 2)
                // {
                //     cout << "Wrong instruction format at line " << line_cnt << endl;
                //     ::exit(8);
                // }
                //provera odozgo je visak jer sam sve to odradio u prvom prolazu!

                if (regex_match(current_line_tokens[0], simple_matches, jumps))
                {
                    dest_reg_number = "F";

                    if(current_line_tokens[0]=="call")
                        op_code = "30";
                    else if(current_line_tokens[0]=="jmp")
                        op_code = "50";
                    else if(current_line_tokens[0]=="jeq")
                        op_code = "51";
                    else if(current_line_tokens[0]=="jne")
                        op_code = "52";
                    else if(current_line_tokens[0]=="jgt")
                        op_code = "53";

                    if (regex_match(current_line_tokens[1], simple_matches, jump_operand_register_simple))
                    {
                        if (current_line_tokens[1].at(1) == '[')
                        {
                            src_reg_number = current_line_tokens[1].substr(3, 1);
                            addr = "02";
                        }
                        else
                        {
                            src_reg_number = current_line_tokens[1].substr(2, 1);
                            addr = "01";
                        }

                        location_counter+=3;
                        instruction_code = op_code + dest_reg_number + src_reg_number + addr;
                    }

                    else if (regex_match(current_line_tokens[1], simple_matches, jump_operand))
                    {
                        bool pcrel = false;
                        int data_payload = 0;
                        src_reg_number = "F";//recimo da ću staviti F tamo gde se ne koristi src registar

                        string sym_lit = current_line_tokens[1];
                        char first_char = current_line_tokens[1].at(0);

                        switch (first_char)
                        {
                        case '*':
                        {
                            sym_lit.erase(0,1);
                            addr = "04";
                            break;
                        }
                        
                        case '%':
                        {
                            addr = "05";
                            pcrel = true;
                            src_reg_number = "7";
                            sym_lit.erase(0,1);
                            break;
                        }

                        default:
                        {
                            addr = "00";
                            break;
                        }
                        }
                        if (regex_match(sym_lit, simple_matches, num_regex)) //literal je u pitanju
                        {
                            if(pcrel)
                            {
                                cout << "Wrong instruction format at line " << line_cnt << endl;
                                ::exit(8);
                            }

                            if (sym_lit.substr(0, 1) != "0")
                            {
                                data_payload = stoi(sym_lit);
                            }
                            else if (sym_lit.substr(0, 2) == "0x" || sym_lit.substr(0, 2) == "0X")
                            {
                                data_payload = stoi(sym_lit, 0, 16);
                            }
                            else if (sym_lit.substr(0, 1) == "0")
                            {
                                data_payload = stoi(sym_lit, 0, 8);
                            }

                        }
                        else
                        {
                            int i = check_string_table_index(sym_lit);

                            if (i == -1)
                            {
                                cout << "Symbol " << sym_lit << " at line " << line_cnt << " hasn't been defined!" << endl;
                                ::exit(13);
                            }
                            else
                            {

                                if (pcrel)
                                {
                                    if (symbol_table[i].section == current_section)
                                    {
                                        data_payload = symbol_table[i].value - location_counter - 5;
                                        //jer je data_payload jednak pomeraju, a on se i nakon linkovanja ne promeni ako su simbol i naredba bili u istoj sekciji
                                    }
                                    else
                                    {
                                        if(symbol_table[i].section=="ABS")
                                            data_payload = -2;//on je jednak addendu, da bi PC znao da u trenutku krpljenja mora da se vrati za 2 unazad jer on gleda u sledeću instrukciju
                                        else
                                            data_payload = symbol_table[i].value - 2;
                                        
                                        int offset = location_counter + 3;
                                        int addend = -2;
                                        if (symbol_table[i].section != "ABS")
                                            relocation_patch(PCREL, offset, i, addend);
                                        else
                                            relocation_patch(PCRELABS, offset, i, addend);
                                    }                                    
                                    
                                }
                                else
                                {
                                    
                                    if (symbol_table[i].info == 'E')//ovaj uslov je baš pipav
                                    { //RELOKACIJA IZGLEDA TREBA DA SE RADI I KADA SIMBOL NIJE DEFINISAN U ISTOJ SEKCIJI U KOJOJ JE OVA TRENUTNA NAREDBA!
                                        //OVO STO PISE GORE PROVERI ODMAH SAD! MOZDA ZAVISI I OD TOGA JE LI PCREL ADRESIRANJE U PITANJU
                                        data_payload = 0;
                                    }
                                    else
                                        data_payload = symbol_table[i].value;//kada je ABS sekcija uzecu ovo vrednost, gornji if uslov je dobar u tom slucaju
                                        //JEDINO OSTAJE: AKO JE SIMBOL LOKALNI, ALI IZ DRUGE SEKCIJE, DA LI SE NJEGOVA VREDNOST PISE U KOD? KOLIKO PEDJA KAZE, DA!
                                    if (symbol_table[i].section != "ABS")
                                    {
                                        int offset = location_counter + 3; //fali mi samo payload odnosno dva poslednja bajta
                                        int addend = 0;
                                        relocation_patch(REL, offset, i, addend);
                                    }
                                }//e u ovom slucaju, kada se u output pise vrednost simbola, treba praviti rel. zapis bez obzira na to da li su bili u istoj sekciji naredba i simbol

                            }
                        }
                        location_counter+=5;
                        payload = print_hex_number(data_payload); 
                        instruction_code = op_code + dest_reg_number + src_reg_number + addr + payload;
                    }
                    else if (rip)
                    {
                        int data_payload = 0;

                        if (regex_search(current_line_tokens[1], simple_matches, regex("r[0-7]")))
                        {
                            src_reg_number = current_line_tokens[1].substr(3, 1);
                        }
                        else
                        {
                            if (regex_search(current_line_tokens[1], simple_matches, regex("sp"))) 
                                src_reg_number = "6";
                            else if (regex_search(current_line_tokens[1], simple_matches, regex("pc")))//ovde nisam siguran da li pc uopste sme da se pojavi! Videcu pri debagovanju.
                                src_reg_number = "7";
                            else if (regex_search(current_line_tokens[1], simple_matches, regex("psw")))
                                src_reg_number = "8";//ovo je diskutabilno
                        }
                        addr = "03";

                        //string rip_help = current_line_tokens[3];
                        
                        //payload = rip_help.substr(4, rip_help.size());//posto je do kraja ostalo manje od rip_help.size(), f-ja ce stati na kraju i ostaje mi "]" koji moram osisati
                        payload = current_line_tokens[3];
                        payload.pop_back();
                        
                        //e sada, payload moze biti simbol ili literal
                        if (!regex_match(payload, simple_matches, num_regex)) //dakle, simbol je
                        {
                            int i = check_string_table_index(payload);

                            if (i == -1)
                            {
                                cout << "Symbol " <<  payload << " at line " << line_cnt << " hasn't been defined!" << endl;
                                ::exit(13);
                            }

                            if (symbol_table[i].info == 'E' /*|| (symbol_table[i].section != current_section && symbol_table[i].section != "ABS")*/)
                            {
                                data_payload = 0;//da li staviti string x-eva?
                            }
                            else
                                data_payload = symbol_table[i].value;

                            int offset = location_counter + 3; //fali mi samo payload odnosno dva poslednja bajta
                            int addend = 0;
                            relocation_patch(REL, offset, i, addend);
                        }
                        else
                        {
                            if (payload.substr(0, 1) != "0")
                            {
                                data_payload = stoi(payload);
                            }
                            else if (payload.substr(0, 2) == "0x" || payload.substr(0, 2) == "0X")
                            {
                                data_payload = stoi(payload, 0, 16);
                            }
                            else if (payload.substr(0, 1) == "0")
                            {
                                data_payload = stoi(payload, 0, 8);
                            }
                        }
                        location_counter+=5;
                        payload = print_hex_number(data_payload);
                        instruction_code = op_code + dest_reg_number + src_reg_number + addr + payload;
                    }

                }

                else //int, push, pop, not
                {
                    if (regex_match(current_line_tokens[1], simple_matches, regex("r[0-7]")))
                    {
                        dest_reg_number = current_line_tokens[1].substr(1, 1);
                    }
                    else
                    {
                        if (current_line_tokens[1] == "sp") //ne svidja mi se sto ovako radim
                            dest_reg_number = "6";
                        else if (current_line_tokens[1] == "pc")
                            dest_reg_number = "7";
                        else if (current_line_tokens[1] == "psw")
                            dest_reg_number = "8";
                    }

                    if (first_word == "int")
                    {
                        instruction_code = "10" + dest_reg_number + "F";
                        location_counter+=2;
                    }
                    if (first_word == "push") //push i pop su ldr i str, samo nabazdareni
                    {
                        instruction_code = "B0" + dest_reg_number + "612";
                        location_counter+=3;
                    }
                    if (first_word == "pop")
                    {
                        instruction_code = "A0" + dest_reg_number + "642";
                        location_counter+=3;
                    }
                    if (first_word == "not")
                    {
                        instruction_code = "80" + dest_reg_number + dest_reg_number;
                        location_counter+=2;
                    }

                }
            }

            if (regex_match(first_word, simple_matches, two_op_command))
            {
                if ((!rip && current_line_tokens.size() != 3)
                ||   (rip && current_line_tokens.size() != 5))
                {
                    cout << "Wrong instruction format at line " << line_cnt << endl;
                    ::exit(8);
                }

                if (first_word == "ldr" || first_word == "str")
                {
                    if (first_word == "ldr")
                    {
                        op_code = "A0";
                    }
                    else
                    {
                        op_code = "B0";
                    }
                    if(current_line_tokens[1].back()==',')
                        current_line_tokens[1].pop_back();
                    
                    if(current_line_tokens[1] == "sp")
                        dest_reg_number = "6";
                    else if(current_line_tokens[1] == "pc")
                        dest_reg_number = "7";
                    else if (current_line_tokens[1] == "psw")
                        dest_reg_number = "8";//ovo nije dobro, tu je samo da zakrpi!
                    else
                        dest_reg_number = current_line_tokens[1].substr(1, 1);

                    if (regex_match(current_line_tokens[2], simple_matches, data_operand_register_simple))
                    {
                        if (current_line_tokens[2].at(0) == '[')
                        {
                            src_reg_number = current_line_tokens[2].substr(2, 1);
                            addr = "02";
                        }
                        else
                        {
                            src_reg_number = current_line_tokens[2].substr(1, 1);
                            addr = "01";
                        }
                        location_counter+=3;
                        instruction_code = op_code + dest_reg_number + src_reg_number + addr;
                    }

                    else if (regex_match(current_line_tokens[2], simple_matches, data_operand))
                    {
                        bool pcrel = false;
                        int data_payload = 0;
                        src_reg_number = "F";//recimo da ću staviti F tamo gde se ne koristi src registar

                        string sym_lit = current_line_tokens[2];
                        char first_char = current_line_tokens[2].at(0);

                        switch (first_char)
                        {
                        case '$':
                        {
                            sym_lit.erase(0,1);
                            addr = "00";
                            break;
                        }
                        
                        case '%':
                        {
                            addr = "03";
                            src_reg_number = "7";
                            pcrel = true;
                            sym_lit.erase(0,1);
                            break;
                        }

                        default:
                        {
                            addr = "04";
                            break;
                        }
                        }
                        if (regex_match(sym_lit, simple_matches, num_regex)) //literal je u pitanju
                        {
                            if(pcrel)
                            {
                                cout << "Wrong instruction format at line " << line_cnt << endl;
                                ::exit(8);
                            }

                            if (sym_lit.substr(0, 1) != "0")
                            {
                                data_payload = stoi(sym_lit);
                            }
                            else if (sym_lit.substr(0, 2) == "0x" || sym_lit.substr(0, 2) == "0X")
                            {
                                data_payload = stoi(sym_lit, 0, 16);
                            }
                            else if (sym_lit.substr(0, 1) == "0")
                            {
                                data_payload = stoi(sym_lit, 0, 8);
                            }
                        }
                        else
                        {
                            int i = check_string_table_index(sym_lit);

                            if (i == -1)
                            {
                                cout << "Symbol " << sym_lit << " at line " << line_cnt << " hasn't been defined!" << endl;
                                ::exit(13);
                            }
                            else
                            {
                                if (pcrel)
                                {
                                    if (symbol_table[i].section == current_section)
                                    {
                                        data_payload = symbol_table[i].value - location_counter - 5;
                                    }
                                    else
                                    {
                                        if(symbol_table[i].section=="ABS")
                                            data_payload = -2;//on je jednak addendu, da bi PC znao da u trenutku krpljenja mora da se vrati za 2 unazad jer on gleda u sledeću instrukciju
                                        else
                                            data_payload = symbol_table[i].value - 2;
                                        
                                        int offset = location_counter + 3;
                                        int addend = -2;
                                        if (symbol_table[i].section != "ABS")
                                            relocation_patch(PCREL, offset, i, addend);
                                        else
                                            relocation_patch(PCRELABS, offset, i, addend);
                                    }
                                    
                                }
                                else
                                {

                                    if (symbol_table[i].info == 'E' /*|| (symbol_table[i].section != current_section && symbol_table[i].section != "ABS")*/)
                                    {
                                        data_payload = 0;
                                    }
                                    else
                                        data_payload = symbol_table[i].value;

                                   if (symbol_table[i].section != "ABS")
                                    {
                                        int offset = location_counter + 3; //fali mi samo payload odnosno dva poslednja bajta
                                        int addend = 0;
                                        relocation_patch(REL, offset, i, addend);
                                    }
                                } //e u ovom slucaju, kada se u output pise vrednost simbola, treba praviti rel. zapis bez obzira na to da li su bili u istoj sekciji naredba i simbol
                            }

                        }//sta je data_payload kad je pcrel adresiranje? E ovde se izgleda javlja i relokacija, hmm...
                        location_counter+=5;
                        payload = print_hex_number(data_payload); 
                        instruction_code = op_code + dest_reg_number + src_reg_number + addr + payload;
                    }

                    else if (rip)
                    {
                        int data_payload = 0;
                        src_reg_number = current_line_tokens[2].substr(2, 1);
                        addr = "03";

                        payload = current_line_tokens[4];
                        payload.pop_back();
                       
                        if (!regex_match(payload, simple_matches, num_regex)) //dakle, simbol je
                        {
                            int i = check_string_table_index(payload);

                            if(i == -1)
                            {
                                cout << "Symbol " << payload << " at line " << line_cnt << " hasn't been defined!" << endl;
                                ::exit(13); 
                            }
                            if (symbol_table[i].info == 'E' /*|| (symbol_table[i].section != current_section && symbol_table[i].section != "ABS")*/)
                            {
                                data_payload = 0;
                            }
                            else
                                data_payload = symbol_table[i].value;

                            if (symbol_table[i].section != "ABS")
                            {
                                int offset = location_counter + 3;
                                int addend = 0;
                                relocation_patch(REL, offset, i, addend);
                            }
                        }
                        else
                        {
                            if (payload.substr(0, 1) != "0")
                            {
                                data_payload = stoi(payload);
                            }
                            else if (payload.substr(0, 2) == "0x" || payload.substr(0, 2) == "0X")
                            {
                                data_payload = stoi(payload, 0, 16);
                            }
                            else if (payload.substr(0, 1) == "0")
                            {
                                data_payload = stoi(payload, 0, 8);
                            }
                        }
                        location_counter+=5;
                        payload = print_hex_number(data_payload);
                        instruction_code = op_code + dest_reg_number + src_reg_number + addr + payload;
                    }
                    
                }
                else
                { //oba operanda su cisti registri
                    if(current_line_tokens[1] == "sp")
                        dest_reg_number = "6";
                    else if(current_line_tokens[1] == "pc")
                        dest_reg_number = "7";
                    else if (current_line_tokens[1] == "psw")
                        dest_reg_number = "8";//ovo nije dobro, tu je samo da zakrpi!
                    else
                        dest_reg_number = current_line_tokens[1].substr(1, 1);

                    if(current_line_tokens[2] == "sp")
                        src_reg_number = "6";
                    else if(current_line_tokens[2] == "pc")
                        src_reg_number = "7";
                    else if (current_line_tokens[2] == "psw")
                        src_reg_number = "8";//ovo nije dobro, tu je samo da zakrpi!
                    else
                        src_reg_number = current_line_tokens[2].substr(1, 1);

                    if (first_word == "xchg")
                        op_code = "60";
                    else if (first_word == "add")
                        op_code = "70";
                    else if (first_word == "sub")
                        op_code = "71";
                    else if (first_word == "mul")
                        op_code = "72";
                    else if (first_word == "div")
                        op_code = "73";
                    else if (first_word == "cmp")
                        op_code = "74";
                    else if (first_word == "and")
                        op_code = "81";
                    else if (first_word == "or")
                        op_code = "82";
                    else if (first_word == "xor")
                        op_code = "83";
                    else if (first_word == "test")
                        op_code = "84";
                    else if (first_word == "shl")
                        op_code = "90";
                    else if (first_word == "shr")
                        op_code = "91";

                    location_counter+=2;
                    instruction_code = op_code + dest_reg_number + src_reg_number;
                }
            }
 
        }
        cout << "Linija broj "<< line_cnt << " ima kod:" << instruction_code << endl;
        cout << "Linija broj "<< line_cnt+1 << " ima locaton_counter:" << location_counter << endl;//test ispis!
        if(instruction_code!="")
            out << instruction_code << endl;
        line_cnt++;
    }
}

void Assembler::print_symbol_table(vector<symbol_table_element> sym_tab, ofstream &out)
{

    vector<symbol_table_element>::iterator it;
    vector<string_table_element>::iterator stringit;

    out<<endl;
    out<<"-------------------------SYMBOL TABLE---------------------------------"<<endl;
    out << setw(20) << left << "Label" << setw(20) << left
        << "Offset" << setw(20)
        << "Size" << setw(20)
        << "Info" << setw(20)
        // << "Defined" << setw(20)
        << "Other" << setw(20)
        << "Section" << endl;

    out << "----------------------------------------------------------------------------------------------------"<<endl;

    for (it = sym_tab.begin(); it != sym_tab.end(); it++)
    {
        string label = "";
        for(string_table_element ste: string_table)
        {
            if(ste.index == it->name)
            {
                label = ste.value;
                break;
            }
        }

        out << setw(20) << left << label << setw(20) << left << it->value << setw(20) << left << it->size
        << setw(20) << left << it->info << setw(20) << left << it->other << setw(20) << left << it->section << endl;

        
    }
    out << "----------------------------------------------------------------------------------------------------"<<endl;
}

void Assembler::print_relocation_table(vector<rel_table_element> rel_tab, ofstream &out)
{

    vector<rel_table_element>::iterator it;
    vector<string_table_element>::iterator stringit;

    out<<endl;
    out<<"-------------------------RELOCATION TABLE---------------------------------"<<endl;
    out << setw(20) << left << "Name" << setw(20) << left
        << "Section" << setw(20) << left
        << "Offset" << setw(20)
        << "Type" << setw(20)
        //<< "Info" << setw(20)
        // << "Defined" << setw(20)
        << "Index" << setw(20)
        << "Addend" << endl;

    out << "----------------------------------------------------------------------------------------------------"<<endl;

    for (it = rel_tab.begin(); it != rel_tab.end(); it++)
    {
        string label = "";
        for(string_table_element ste: string_table)
        {
            if(ste.index == it->index)
            {
                label = ste.value;
                break;
            }
        }

        out << setw(20) << left << label << setw(20) << left << it->section<< setw(20) << left << it->offset
        << setw(20) << left << it->relType << setw(20) << left << it->index << setw(20) << left << it->addend << endl;

       
    }
     out << "----------------------------------------------------------------------------------------------------"<<endl;
}

Assembler::Assembler()
{
    location_counter = 0;
    curr_line = "";
};
Assembler::~Assembler(){

};


int main(int argc, char const *argv[])
{

    Assembler *a = new Assembler();
    ifstream myfile;
    ofstream out = ofstream();

    if(argc>2)
    {
        string option = argv[2];
        out.open(argv[3]);
    }
    

    cout << argc << endl;
    int i=0;
    while(argv[i])
    {
        cout<<argv[i]<<endl;
        i++;
    }


    myfile.open(argv[1]);
    if (!myfile)
    {
        cerr << "Unable to open file " << argv[1];
        ::exit(4);
    }

    a->first_pass(myfile);

    myfile.open(argv[1]);
    if (!myfile)
    {
        cerr << "Unable to open file!";
        ::exit(4);
    }
    a->second_pass(myfile, out);

    a->print_symbol_table(a->symbol_table, out);
    a->print_relocation_table(a->relocation_table, out);

    cout<< "Symbol table contains following symbols:"<<endl;
    for (string_table_element ste: a->string_table)
        cout << ste.value << endl;

    myfile.close();
    return 0;
}