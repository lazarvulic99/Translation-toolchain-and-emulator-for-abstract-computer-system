#ifndef LINKER_HPP
#define LINKER_HPP

#include <string>
#include <iostream>
#include <vector>
#include <fstream>

using namespace std;

struct Entry_Symbol{
  string fajl;
  int symId;
  string symName;
  string symValue; // To je bukvalno ofset, tj. adresa do simbola
  string sectionName;
  int symSize;
  int isSection;
  string symDefined;
  string symGlobal;
};

struct Entry_Reallocation{
  string fajl;
  string sekcija;
  int refSym;
  int location;
  string locationHex;
  int type; // 0 - word, 1 - pc rel, 2 - ostale instr.
  int addend;
  int realAddend;
};

struct Entry_Section{
  string fajl;
  string name;
  //int id;
  int beginningAddress;
  int size;
};

struct SectionCode{
  string fajl;
  string sectionName;
  string offset;
  string code;
};

class Linker{
private:
  string imeIzlaza;
  vector<string> ulazniFajlovi;

  vector<Entry_Symbol> tableSymbols;
  vector<Entry_Reallocation> tableReallocations;
  vector<Entry_Section> tableSections;
  vector<SectionCode> kodSekcija;

  ofstream outputFile;
public:
  Linker(string nazivIzlaza, vector<string> naziviUlaznihFajlova);
  string kodKonacni;
  void obrada();
  void zatvoriFajlove();
  int prolazakKrozUlazneFajlove();
  int obradiSekcije();
  int dodajSekcijeNaOsnovuSimbola();
  int azurirajPocetneAdreseSekcija();
  int prepraviTabeluSimbola();
  int prepraviRelokacije();
  int pretvoriIzHexUDec(string hex);
  string pretvoriUHexOblik(int broj);
  string simbolHexOblik(string hexVrednost);
  string stringToUpper(string strToConvert);
  int pomocnaMetodaZaNegativneBrojeve(string hexBroj);
  void potraziSekcijuIstogImenaIzDrugihFajlova(string sectionName, int startingAddress, string imeFajla);
  void dodajSimbol(string imeFajla, string symbol_id, string name, string value, string sectionName, string globalYesNo, string definedYesNo, string realSize, string isSectionYesNo);
  void dodajRelokaciju(string imeFajla, string sekcija_i_refSym, string location, string locationHex, string type, string addend, string realAddend);
  void ispisZaEmulator();
  void ispisZaEmulatorNovo();
  string dodajDodatneSpejsove(string s);
  int fjaZaPretvaranjeIzHexUDec(string s);
  string spejsoviZaZadnjuLiniju(string s);
};

#endif
