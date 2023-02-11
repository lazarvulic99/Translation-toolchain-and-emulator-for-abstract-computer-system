#ifndef ASSEMBLER_HPP
#define ASSEMBLER_HPP

#include <string>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

struct Entry_Reallocation{
  int location;
  int refSym;
  int type; // 0 za razresavanje simbola word, 1 za razresavanje kod pc rel adresiranja, 2 za razresavanje simbola instr.
  int addend; // za fiksni deo instrukcije u polju koje treba da se prepravi
  int realSymbolNumber;
  int realAddend;
};

struct Entry_Symbol_Appearance{
  int offsetInSection; // pomeraj od pocetka sekcije u kojoj se javlja
  int sectionId; // id sekcije u kojoj se javio
  string sectionName; // ime sekcije u kojoj se javio
  int type; // tip pojavljivanja, 0 za simbol, 1 za pc rel adresiranje
  int isData; // 0 za word, 1 za instrukcije
};

// defines one entry for sections table
struct Entry_Section{
  string sectionName;
  int sectionID;
  int sectionSize;
  int beginningAddress; // ovo po defaultu stavljam na 0, pa ce linker kad ih bude razmestao to da azurira
  vector<char> sectionCode;
  vector<Entry_Reallocation> tableOfRealocations;
};

// defines one entry for symbols table
// can not use extern for variable, so I'll use externSymbol. Every bool field renamed to sthSymbol
struct Entry_Symbol{
  string symbolName;
  string sectionName;
  int symbolID;
  int symbolSize;
  int symbolValue;
  bool globalSymbol;
  bool definedSymbol;
  bool isSection;
  vector<Entry_Symbol_Appearance> symbolReferencing; // to je forward link u sustini sa predavanja
  //bool externSymbol;
};

class Assembler{
private:
  string input_file_name;
  string output_file_name;

  vector<string> modified_lines;

  static int assembling_finished;

  static int locationCounter;
  static string pendingSectionName;
  static int pendingSectionNo;

  static int sections_ID_autoIncrement;
  static int symbols_ID_autoIncrement;

  string line_removeComments(string line);
  string line_removeBlankCharactersAtBeginning(string line);
  string line_removeAdditionalSpacesOrTabulator(string line);
  string line_removePunctuationSpace(string line);

  bool checkForLabelInLine(string modified_line);

  bool checkForDirectiveInLine(string modified_line);
  bool checkForInstructionInLine(string modified_line);

  void skipDirectiveWork(string line);
  void wordDirectiveWork(string line);

  bool checkIfGlobalDirective(string line);
  bool checkIfExternDirective(string line);
  bool checkIfSectionDirective(string line);
  bool checkIfWordDirective(string line);
  bool checkIfSkipDirective(string line);
  bool checkIfEndDirective(string line);

  void isThereUndefinedSymbol();
  void dodajRelokacijuZaSekciju(int offset, int symbol, int type, int tekucaSekcija, int addend);
  void backpatch();

  // Stara verzija 
  int parseModifiedLines();

  // Nova verzija
  //int parseModifiedLines(string line);

  string pretvoriUHexOblik(int broj);
  int pretvoriIzHexUDec(string hex);
  string stringToUpper(string strToConvert);
  string simbolHexOblik(string hexVrednost);
public:
  static int lineNo;
  vector<Entry_Section> tableSections;
  vector<Entry_Symbol> tableSymbols;

  int dodajSimbol(string simbolName, int tip, int offsetInSection);
  int dohvatiAdresuSimbola(string simbolName);
  string odluciBrojRegistra(string operandRegistar);
  string odradiPosaoOkoSimbola(string operand, int offset, int type);
  void prepraviTabeluRelokacionihZapisa();
  void izbrisiNepotrebneRelokacije();

  Assembler(string inputFileName, string outputFileName);
  void ispisiTabeluSekcija(fstream& izlazniFajl); // Mora da stoji & za referencu, inace prosledjeni izl. fajl ne bi mogao da se menja
  void ispisiTabeluSimbola(fstream& izlazniFajl); // Iz istog razloga je prosledjenja referenca na izl. fajl
  void ispisiKodSekcije(fstream& izlazniFajl); // Iz istog razloga je prosledjena referenca na izl. fajl
  void ispisiRelokacioneZapise(fstream& outputFile); // Iz istog razloga je prosledjena referenca na izl. fajl
  int checkFileExtensions();
  void initializeAssembler();
  int makeAssemblyLines();
  int assembly();
  void generateOutputFile();
  void generateLinkerInputFile();
};

#endif