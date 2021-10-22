#include <string>
#include <iostream>
#include <vector>
#include <regex>

using namespace std;

typedef struct {
    int num;
    string symbol;

} rel_text_element;

typedef struct {
    uint32_t offset;
    uint32_t info;

} rel_data_element;

typedef struct {
    uint32_t name;//index
    uint32_t value;//adresa na kojoj se nasao simbol, odnosno location_counter
    uint32_t size;
    char info;//Ovde cu staviti podatak o tome da li je simbol definisan kao local, extern ili global,
    //ili 'U' ako je nedefinisan trenutno, odnosno definisan pomocu .word
    char other;
    string section;//"ABS" - apsolutni, "UND" - nedefinisan; sve ostalo znaci da je korektno; mozda ga ipak nekad izmeniti u string?
    //"ABS" - kada je definisan pomocu .equ, "UND" - kada sam prvi put naleteo na njega u .global ili .extern

} symbol_table_element;

typedef struct {
    uint32_t index;//index iz tabele simbola
    string value;
    uint32_t info;

} string_table_element;

typedef struct {
    uint32_t symbol_table_index;
    int value;//u formi stringa ili broja?
    //uint32_t info;

} literal_pool_element;

typedef struct {
    string section;//trenutna sekcija, ona u kojoj se nalazi naredba koju krpiš
    int offset;
    string relType;
    int index;//index simbola u tabeli simbola, da znam koji treba gadjati; ovo je zapravo jako bitan podatak
    //pedja je stavljao redni broj sekcije u kojoj se nalazi simbol kojim se krpi; mislim da sam ovo pogrešio, to je r.b. simbola, što već imam
    int addend;

} rel_table_element;

class Assembler {

    private:
    uint32_t location_counter;
    uint32_t sym_table_index=0;
    string curr_line;
    bool label_previous_line = false;
    short label_cnt = 0; //broji labele u jednom redu
    smatch white_matches, lab_matches, dir_matches, section_matches, end_match, defining_matches, simple_matches;
    //verovatno sam mogao da definisem samo jedan ovakav objekat i da ga svuda koristim
    //zapravo mi trebaju minimum dva zbog onog sto sam uradio kad naidjem na .section
    string cut_line;//ova bi trebalo da mi je visak
    int line_cnt = 1;
    string current_section = "";
    bool new_section_begins = false;

    const string REL = "R_386_16";
    const string PCREL = "R_386_PC16";
    const string PCRELABS = "R_386_PC16_ABS";//koja je razlika?

    public:
    Assembler();
    ~Assembler();


    vector<rel_text_element> rel_text_table;//sta sam ja definisao ovde, pokazivac, referencu ili nesro trece? Izgleda da ne mora pokazivac, ovo se otrpilike ponasa kao ime instance strukture, pristup elementu ide identifikator.polje.
    vector<rel_data_element> rel_data_table;
    vector<symbol_table_element> symbol_table;
    vector<string_table_element> string_table;
    vector<string> word_defined_symbols;
    vector<string> current_line_tokens;
    vector<literal_pool_element> literal_pool;
    vector<rel_table_element> relocation_table;

    void first_pass(ifstream& doc);
    void second_pass(ifstream& doc, ofstream &out);
    
    void cut_ws_before(string* s);
    void cut_ws_after(string* s);
    void cut_r_after(string *s);
    bool directive_processing(string first_word, string temp_line);
    string cut_comment(string s);
    bool check_string_table(string s);
    void tokenize(string s);
    void location_counter_increase(string s);
    //void second_pass_processing(string first_word);
    int check_string_table_index(string s);
    int get_st_value(int i);
    string print_hex_number(int num);
    void relocation_patch(string rel_type, int offset, int index, int addend);
    void print_symbol_table(vector<symbol_table_element> sym_tab, ofstream &out);
    void print_relocation_table(vector<rel_table_element> rel_tab, ofstream &out);
    
    
};