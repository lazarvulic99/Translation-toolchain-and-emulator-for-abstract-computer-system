#include <iostream>
#include <string>
#include <fstream>
#include <regex>
#include <iomanip>
#include <sstream>

#include "../inc/assembler.hpp"
#include "../inc/regexes.hpp"
//#include "/home/ss/Desktop/resenje/inc/assembler.hpp"
//#include "/home/ss/Desktop/resenje/inc/regexes.hpp"

int Assembler::sections_ID_autoIncrement = 0;
int Assembler::symbols_ID_autoIncrement = 0;
int Assembler::lineNo = 0;
string Assembler::pendingSectionName = "";
int Assembler::pendingSectionNo = -1;
int Assembler::assembling_finished = 0;
int Assembler::locationCounter = 0;

void tokenize(string const &str, const char delim, vector<string> &out)
{
    stringstream ss(str);
    string s;
    while (getline(ss, s, delim)) {
        out.push_back(s);
    }
}

string Assembler::simbolHexOblik(string hexVrednost){
  string povratnaVrednost = "";
  if(hexVrednost.size() == 1){
    povratnaVrednost = "000" + hexVrednost;
  }else if(hexVrednost.size() == 2){
    povratnaVrednost = "00" + hexVrednost;
  }else if(hexVrednost.size() == 3){
    povratnaVrednost = "0" + hexVrednost;
  }else{
    povratnaVrednost = hexVrednost;
  }
  return povratnaVrednost;
}

string Assembler::odluciBrojRegistra(string operandRegistar){
  //cout << "Odlucujem o broju registra: " << operandRegistar << endl;
  if(!operandRegistar.compare("psw")){
    return "8";
  }else if(!operandRegistar.compare("pc")){
    return "7";
  }else if(!operandRegistar.compare("sp")){
    return "6";
  }else{
    string broj = operandRegistar.substr(1, -1);
    return broj;
  }
}

Assembler::Assembler(string inputFileName, string outputFileName){
  input_file_name = inputFileName;
  output_file_name = outputFileName;
}

int Assembler::checkFileExtensions(){
  if(regex_match(input_file_name, regex_input_file_extension)){
      //cout << "[ Prosao regex za ulazni fajl ]" << endl;
  }else{
    cout << "*** Ulazni fajl nema ekstenziju .s ***" << endl;
    return - 1;
  }
  if(regex_match(output_file_name, regex_ouput_file_extension)){
    //cout << "[ Prosao regex za izlazni fajl ]" << endl;    
  }else{
    cout << "*** Izlazni fajl nema ekstenziju .o ***" << endl;
    return - 1;
  }
  return 0;
}

bool Assembler::checkForLabelInLine(string line){
  smatch imeLabele;
  // !!!OVAJ DEO DODAT ZBOG RAZMAKA NA KRAJA
  /*int pos = line.find_last_not_of(" ");
  cout << "Pozicija: " << pos << endl;
  line = line.substr(0, pos + 1);*/
  // !!!OVAJ DEO DODAT zbog razmaka na kraju
  bool found = regex_search(line, imeLabele, regex_for_label);
  if(found == true){
    //cout << "*** Pronasao labelu ***" << endl;
    int found = 0;
    string imeLabele = line.substr(0, line.size() - 1);
    for(auto & element : tableSections){
      if(!element.sectionName.compare(imeLabele)){
        found = 1;
        break;
      }
    }
    if(found == 0){
      //cout << "OK" << endl;
      if(pendingSectionNo != -1){
        //cout << "Labela u sekciji!" << endl;
        int foundSymbol = 0;
        int br = 0;
        for(auto & elementS : tableSymbols){
          if(!elementS.symbolName.compare(imeLabele)){
            foundSymbol = 1;
            break;
          }
          br = br + 1;
        }
        if(!foundSymbol){
          Entry_Symbol noviSimbol;
          noviSimbol.symbolName = imeLabele;
          noviSimbol.sectionName = pendingSectionName;
          noviSimbol.symbolID = symbols_ID_autoIncrement++;
          noviSimbol.symbolSize = -1;
          noviSimbol.symbolValue = locationCounter;
          noviSimbol.globalSymbol = false;
          noviSimbol.definedSymbol = true;
          noviSimbol.isSection = false;
          tableSymbols.push_back(noviSimbol);
          return true;
        }else{
          //cout << "Vec je dodat taj simbol od ranije." << endl;
          if(!tableSymbols[br].sectionName.compare("UNDEFINED")){
            cout << "Simbol je uvezen i ne moze da se definise u nasoj sekciji." << endl;
            exit(-1);
          }
          if(tableSymbols[br].definedSymbol){
            cout << "Simbol je vec definisan." << endl;
            exit(-1);
          }
          tableSymbols[br].definedSymbol = true;
          tableSymbols[br].sectionName = pendingSectionName;
          tableSymbols[br].symbolValue = locationCounter;
          return true;
        }
      }else{
        cout << "Labela ne moze da ne bude u definisanoj sekciji." << endl;
        exit(-1);
      }
    }else{
      cout << "Labela ne moze da se zove isto kao neka od sekcija ispred nje." << endl;
      exit(-1);
    }
  }else{
    //cout << "EVO ME OVDE U PROVERI" << endl;
    string firstChar = line.substr(0, 1);
    if(!firstChar.compare(".")){
      return false;
    }
    int position = line.find_first_of(" ");
    if (position != -1){
      string izaLabele = line.substr(position + 1, -1);
      string imeLabele = line.substr(0, position);
      if(imeLabele.at(position - 1) == ':'){
        //cout << "Labela je." << endl;
      }else{
        return false;
      }
      //cout << "Provera labela i direktiva/instrukcija u istoj liniji." << endl;
      izaLabele = line_removeBlankCharactersAtBeginning(izaLabele);
      izaLabele = line_removeAdditionalSpacesOrTabulator(izaLabele);
      izaLabele = line_removePunctuationSpace(izaLabele);
      bool retVal = checkForLabelInLine(imeLabele);
      retVal = checkForDirectiveInLine(izaLabele);
      // Dodaj if, ako je retVal true znaci da je u pitanju direktiva, ako je false, proveri da li je instrukcija
      if(!retVal){
        retVal = checkForInstructionInLine(izaLabele);
      }
      return retVal;
    }
  }
  return found;
}

bool Assembler::checkIfGlobalDirective(string line){
  smatch globalDir;
  bool found = regex_search(line, globalDir, regex_for_global);
  if(found == true){
    //cout << "*** Pronasao global ***" << endl;
    string simboli = line.substr(8, -1);
    const char delimiter = ',';
    vector<string> simbol;
    tokenize(simboli, delimiter, simbol);    
    for (auto & s: simbol) {
      s = line_removeBlankCharactersAtBeginning(s);
      int pronasaoSekcijuSaImenom = 0;
      Entry_Section foundSection;
      for (auto & element : tableSections) {
        if(!element.sectionName.compare(s)){
          pronasaoSekcijuSaImenom = 1;
          foundSection = element;
          break;
        }
      }
      if(pronasaoSekcijuSaImenom){
        cout << "*** Ime sekcije ne mozete proglasiti globalnim. ***" << endl;
      }else{
        int pronasaoSimbolSaImenom = 0;
        for (auto & element : tableSymbols) {
          if(!element.symbolName.compare(s)){
            pronasaoSimbolSaImenom = 1;
            if(!element.sectionName.compare("UNDEFINED")){ // Ovo proveriti da li ulazi!!!
              cout << "*** Simbol sme ili da se izvozi, ili da se uvozi. ***" << endl;
              break;
            }
            element.globalSymbol = true;
            break;
          }
        }
        if(pronasaoSimbolSaImenom == 0){
          Entry_Symbol noviSimbol;
          noviSimbol.symbolID = Assembler::symbols_ID_autoIncrement++;
          noviSimbol.symbolName = s;
          noviSimbol.sectionName = "NEPOZNATO"; // Ovo proveriti sta kako!!!
          noviSimbol.globalSymbol = true;
          noviSimbol.symbolSize = -1;
          noviSimbol.definedSymbol = false; // Ovo proveriti!!!
          noviSimbol.symbolValue = 0;
          noviSimbol.isSection = false;
          tableSymbols.push_back(noviSimbol);
        }else{
          //cout << "*** Simbol sa takvim imenom je vec definisan. ***" << endl;
        }
      }
    }
  }
  return found;
}

bool Assembler::checkIfExternDirective(string line){
  smatch externDir;
  bool found = regex_search(line, externDir, regex_for_extern);
  if(found == true){
    //cout << "*** Pronasao extern ***" << endl;
    string simboli = line.substr(8, -1);
    const char delimiter = ',';
    vector<string> simbol;
    tokenize(simboli, delimiter, simbol);
    for (auto & s: simbol) {
      s = line_removeBlankCharactersAtBeginning(s);
      int pronasaoSekcijuSaImenom = 0;
      Entry_Section foundSection;
      for (auto & element : tableSections) {
        if(!element.sectionName.compare(s)){
          pronasaoSekcijuSaImenom = 1;
          foundSection = element;
          break;
        }
      }
      if(pronasaoSekcijuSaImenom){
        cout << "*** Simbol koji predstavlja sekciju se ne sme izvoziti! ***" << endl;
        exit(-1);
      }else{
        int pronasaoSimbolSaImenom = 0;
        Entry_Symbol  foundSymbol;
        int br = -1;
        for (auto & element : tableSymbols) {
          br++;
          if(!element.symbolName.compare(s)){
            pronasaoSimbolSaImenom = 1;
            foundSymbol = element;
            break;
          }
        }
        if(pronasaoSimbolSaImenom == 0){
          Entry_Symbol noviSimbol;
          noviSimbol.symbolName = s;
          noviSimbol.symbolID = Assembler::symbols_ID_autoIncrement++;
          noviSimbol.globalSymbol = true;
          noviSimbol.definedSymbol = false;
          noviSimbol.symbolSize = -1;
          noviSimbol.symbolValue = 0;
          noviSimbol.sectionName = "UNDEFINED";
          noviSimbol.isSection = false;
          tableSymbols.push_back(noviSimbol);
        }else{
          // Tu odraditi jos neke provere, da li je simbol definisan ili ne.
          if (foundSymbol.globalSymbol && !foundSymbol.sectionName.compare("NEPOZNATO")){
            cout << "Sa extern se izvozi simbol, a ovaj simbol je definisan kao uvezeni." << endl;
            exit(-1);
          }
          if (foundSymbol.definedSymbol){
            cout << "Ne mozete da izvezete definisan simbol." << endl;
            exit(-1);
          }
          for (auto & element : tableSymbols)
          {
            if(!element.symbolName.compare(s)){
              element.sectionName = "UNDEFINED"; 
              element.globalSymbol = true;
              break;
            }
          }          
          /*foundSymbol.sectionName = "UNDEFINED";
          foundSymbol.globalSymbol = true;*/
        }
      }
    } 
  }else{
    //cout << "*** Nije extern ***" << endl;
  }
  return found;
}

void Assembler::skipDirectiveWork(string line){
  string bytesForSkipString = line.substr(6, -1);
  int bytesForSkip = stoi(bytesForSkipString, nullptr, 0); // 3. argument je po defaultu za osnovu 10, kada je 0, o osnovi se odlucuje na osnovu formata stringa.
  //cout << "BAJTA ZA SKIP: " << bytesForSkip << endl;
  if(pendingSectionNo != -1){
    if(bytesForSkip < 0){
      cout << "*** Literal iza .skip mora biti pozitivan. ***" << endl;
      exit(-1);
    }else{
      int pronasao = 0;
      for (auto & element : tableSections) {
        if(element.sectionID == pendingSectionNo){
          pronasao = 1;
          for (int i = 0; i < bytesForSkip*2; i++){
            element.sectionCode.push_back('0');
          }
          break;
        }
      } 
      tableSections[pendingSectionNo].sectionSize += bytesForSkip;
      locationCounter += bytesForSkip;
      //cout << locationCounter << endl;
    }
  }else{
    cout << "*** .skip direktiva mora da se nadje u sekciji: " << line << endl;
    exit(-1);
  }
}

bool Assembler::checkIfSkipDirective(string line){
  smatch skipDir;
  bool found = regex_search(line, skipDir, regex_for_skip_hex_literal);
  if(found == true){
    //cout << "*** Pronasao skip hex ***" << endl;
    skipDirectiveWork(line);
  }else{
    found = regex_search(line, skipDir, regex_for_skip_dec_positive_literal);
    if(found == true){
      //cout << "*** Pronasao skip pos dec ***" << endl;
      skipDirectiveWork(line);
    }else{
      found = regex_search(line, skipDir, regex_for_skip_dec_negative_literal);
      if(found == true){
        //cout << "*** Pronasao skip neg dec ***" << endl;
        skipDirectiveWork(line);
      }else{
        //cout << "*** Nije skip ***" << endl;
      }
    }
  }
  return found;
}

string Assembler::pretvoriUHexOblik(int broj){
  stringstream hexOblik;
  hexOblik << hex << broj;
  return hexOblik.str();
}

string Assembler::stringToUpper(string strToConvert)
{
    transform(strToConvert.begin(), strToConvert.end(), strToConvert.begin(), ::toupper);
    return strToConvert;
}

void Assembler::dodajRelokacijuZaSekciju(int offset, int symbol, int type, int tekucaSekcija, int addend){
  // type: 0 - symbol, 1 - pc rel addr
  Entry_Reallocation relokacioniZapis;
  relokacioniZapis.location = offset;
  relokacioniZapis.refSym = symbol;
  relokacioniZapis.type = type;
  relokacioniZapis.addend = addend;
  relokacioniZapis.realAddend = addend;
  relokacioniZapis.realSymbolNumber = symbol;
  for(auto & elem: tableSections){
    if(elem.sectionID == tekucaSekcija){
      elem.tableOfRealocations.push_back(relokacioniZapis);
      break;
    }
  }
}

void Assembler::wordDirectiveWork(string line){
  //cout << line << endl;
  if(pendingSectionNo == -1){
    cout << ".word direktiva se mora naci u sekciji." << endl;
    exit(-1);
  }else{
    //cout << line << " HEHE" << endl;
    if(regex_match(line, regex_word_1) || regex_match(line, regex_word_2)  || regex_match(line, regex_word_3)){
      string broj = line.substr(0,-1);
      //cout << broj << " broj" << endl;
      int bioHex = 0;
      if(regex_match(line, regex_word_1)){
        bioHex = 1;
      }
      int cifra = stoi(line, nullptr, 0);
      //cout << "CIFRA: " << cifra << endl;
      string hexCifra = pretvoriUHexOblik(cifra);
      //cout << hexCifra << endl;
      hexCifra = stringToUpper(hexCifra);
      //cout << "LINIJA WORD: " << broj << endl;
      if(regex_match(line, regex_word_3)){
        //cout << "BIO JE NEG DEC BROJ" << endl;
      }
      //cout << hexCifra << endl;
      locationCounter += 2;
      //cout << locationCounter << endl;
      int br = -1;
      for(auto & element : tableSections){
        br = br + 1;
        if (!element.sectionName.compare(pendingSectionName)){
          element.sectionSize = element.sectionSize + 2;
          if(bioHex){/*
            int sizeBroj = broj.size();
            if(sizeBroj == 3){
              element.sectionCode.push_back('0');
            }
            for (int i = 2; i < sizeBroj; i++)
            {
              element.sectionCode.push_back(broj[i]);
            }
            element.sectionCode.push_back('0');
            element.sectionCode.push_back('0');*/
            //cout << "BIO HEX: " << endl;
            if(hexCifra.size() < 4){
              hexCifra = simbolHexOblik(hexCifra);
            }
            //cout << hexCifra << endl;
            element.sectionCode.push_back(hexCifra[2]);
            element.sectionCode.push_back(hexCifra[3]);
            element.sectionCode.push_back(hexCifra[0]);
            element.sectionCode.push_back(hexCifra[1]);
          }else{
            /*int hexSize = hexCifra.size();
            if(hexSize == 1){
              element.sectionCode.push_back('0');
              element.sectionCode.push_back(hexCifra[0]);
              element.sectionCode.push_back('0');
              element.sectionCode.push_back('0');
            }else{
              for (int i = 0; i < hexCifra.size(); i++)
              {
                element.sectionCode.push_back(hexCifra[i]);
              }
              element.sectionCode.push_back('0');
              element.sectionCode.push_back('0');
            }*/
            //cout << "NIJE HEX: " << hexCifra << endl;
            if(regex_match(line, regex_word_3)){
              //cout << "EVO SADA I TU: " << hexCifra << endl;
              hexCifra = hexCifra.substr(hexCifra.size() - 4, 4);
              //cout << hexCifra << endl;
            }else{
              hexCifra = simbolHexOblik(hexCifra);
            }
            //cout << hexCifra << endl;
            element.sectionCode.push_back(hexCifra[2]);
            element.sectionCode.push_back(hexCifra[3]);
            element.sectionCode.push_back(hexCifra[0]);
            element.sectionCode.push_back(hexCifra[1]);
          }
          break;
        }
      }
      vector<char> kod = tableSections[br].sectionCode;
      /*for (int i = 0; i < kod.size(); i++)
      {
        cout << kod[i] << "";
      }
      cout << endl;*/
    }else{
      if(regex_match(line, regex_word_4)){
        int pronasao = 0;
        int br = -1;
        int brojSimbola = -1;
        for(auto & element : tableSymbols){
          br = br + 1;
          if (!element.symbolName.compare(line)){
            pronasao = 1;
            brojSimbola = element.symbolID;
            break;
          }
        }
        if(pronasao == 1){/*
          Entry_Symbol_Appearance symbolAppearance;
          symbolAppearance.sectionName = pendingSectionName;
          symbolAppearance.sectionId = pendingSectionNo;
          symbolAppearance.offsetInSection = locationCounter;
          symbolAppearance.type = 0;
          tableSymbols[br].symbolReferencing.push_back(symbolAppearance);
          for (auto & elemSec : tableSections){
            if(elemSec.sectionID == pendingSectionNo){
              for(int i = 0; i < 4; i++){
                elemSec.sectionCode.push_back('0');
              }
              elemSec.sectionSize = elemSec.sectionSize + 2;
              locationCounter = locationCounter + 2;
            }
          }*/
          //cout << "Evo ovde vec postoji simbol." << endl;
          int vrednostSimbola = tableSymbols[br].symbolValue;
          string hexVrednostSimbola = pretvoriUHexOblik(vrednostSimbola);
          hexVrednostSimbola = stringToUpper(hexVrednostSimbola);
          //cout << "Vrednost simbola: " << hexVrednostSimbola << endl;
          if(hexVrednostSimbola.size() == 1){
            hexVrednostSimbola = "000" + hexVrednostSimbola;
          }else if(hexVrednostSimbola.size() == 2){
            hexVrednostSimbola = "00" + hexVrednostSimbola;
          }else if(hexVrednostSimbola.size() == 3){
            hexVrednostSimbola = "0" + hexVrednostSimbola;
          }
          for(auto & elemSec : tableSections){
            if(elemSec.sectionID == pendingSectionNo){
              // Za lokalne simbole ce se upisati njihov value
              // Za globalne simbole ce se u section.code upisati 00 00
              // Cak i za nedifinisan simbol, ubacice 00 00 u section.code jer sam njihov value postavljao na 0 po defaultu.
              elemSec.sectionCode.push_back(hexVrednostSimbola[2]);
              elemSec.sectionCode.push_back(hexVrednostSimbola[3]);
              elemSec.sectionCode.push_back(hexVrednostSimbola[0]);
              elemSec.sectionCode.push_back(hexVrednostSimbola[1]);
              elemSec.sectionSize = elemSec.sectionSize + 2;
              break;              
            }
          }
          Entry_Symbol_Appearance symbolAppearance;
          symbolAppearance.sectionName = pendingSectionName;
          symbolAppearance.sectionId = pendingSectionNo;
          symbolAppearance.offsetInSection = locationCounter;
          symbolAppearance.type = 0;
          symbolAppearance.isData = 0;
          tableSymbols[brojSimbola].symbolReferencing.push_back(symbolAppearance);
          dodajRelokacijuZaSekciju(locationCounter, brojSimbola, 0, pendingSectionNo, 0);
          locationCounter = locationCounter + 2;
        }else{
          Entry_Symbol noviSimbol;
          noviSimbol.symbolName = line;
          noviSimbol.symbolID = symbols_ID_autoIncrement++;
          noviSimbol.symbolValue = 0;
          noviSimbol.symbolSize = -1;
          noviSimbol.globalSymbol = false;
          noviSimbol.definedSymbol = false;
          noviSimbol.isSection = false;
          noviSimbol.sectionName = "NEPOZNATO"; // Ovo proveri
          tableSymbols.push_back(noviSimbol);
          brojSimbola = noviSimbol.symbolID;
          dodajRelokacijuZaSekciju(locationCounter, brojSimbola, 0, pendingSectionNo, 0);
          // Tu ce biti neki dodatan posao
          Entry_Symbol_Appearance symbolAppearance;
          symbolAppearance.sectionName = pendingSectionName;
          symbolAppearance.sectionId = pendingSectionNo;
          symbolAppearance.offsetInSection = locationCounter;
          symbolAppearance.type = 0;
          symbolAppearance.isData = 0;
          for (auto & elem1 : tableSymbols)
          {
            if(!elem1.symbolName.compare(noviSimbol.symbolName)){
              //cout << "Evo me xD" << endl;
              elem1.symbolReferencing.push_back(symbolAppearance);
              break;
            }
          }
          for (auto & elem2 : tableSections){
            if(elem2.sectionID == pendingSectionNo){
              //elem2.sectionCode.push_back('0'); // ovo ce se u metodi za backpatching prepraviti ukoliko je vrednost simbola iza .word bila razlicita od 0
              for(int i = 0; i < 4; i++){
                elem2.sectionCode.push_back('0');
              }
              elem2.sectionSize = elem2.sectionSize + 2;
              locationCounter = locationCounter + 2;
              //cout << "Evo me i tu sam bio." << endl;
            }
          }
        }
      }else {
        cout << "Ne radi se ni o cemu!" << endl;
        exit(-1);
      }
    }
  }
}

string Assembler::odradiPosaoOkoSimbola(string operand, int offset, int type){
  int pronasao = 0;
  int br = -1;
  int brojSimbola = -1;
  for(auto & elemSim : tableSymbols){
    br++;
    if(!elemSim.symbolName.compare(operand)){
      pronasao = 1;
      brojSimbola = elemSim.symbolID;
      break;
    }
  }
  if(pronasao == 1){
    //cout << "Evo ovde vec postoji simbol." << endl;
    int vrednostSimbola = tableSymbols[br].symbolValue;
    string hexVrednostSimbola = pretvoriUHexOblik(vrednostSimbola);
    hexVrednostSimbola = stringToUpper(hexVrednostSimbola);
    //cout << "Vrednost simbola: " << hexVrednostSimbola << endl;
    if(hexVrednostSimbola.size() == 1){
      hexVrednostSimbola = "000" + hexVrednostSimbola;
    }else if(hexVrednostSimbola.size() == 2){
      hexVrednostSimbola = "00" + hexVrednostSimbola;
    }else if(hexVrednostSimbola.size() == 3){
      hexVrednostSimbola = "0" + hexVrednostSimbola;
    }
    Entry_Symbol_Appearance flink;
    flink.sectionId = pendingSectionNo;
    flink.sectionName = pendingSectionName;
    flink.type = 0;
    flink.offsetInSection = offset;
    flink.isData = 1;
    tableSymbols[brojSimbola].symbolReferencing.push_back(flink);
    dodajRelokacijuZaSekciju(offset, brojSimbola, type, pendingSectionNo, 0);
    return hexVrednostSimbola;
  }else{
    Entry_Symbol noviSimbol;
    noviSimbol.symbolName = operand;
    noviSimbol.symbolID = symbols_ID_autoIncrement++;
    noviSimbol.symbolValue = 0;
    noviSimbol.symbolSize = -1;
    noviSimbol.globalSymbol = false;
    noviSimbol.definedSymbol = false;
    noviSimbol.isSection = false;
    noviSimbol.sectionName = "NEPOZNATO"; // Ovo proveri
    tableSymbols.push_back(noviSimbol);
    brojSimbola = noviSimbol.symbolID;
    dodajRelokacijuZaSekciju(offset, brojSimbola, type, pendingSectionNo, 0);
    // Tu ce biti neki dodatan posao
    Entry_Symbol_Appearance symbolAppearance;
    symbolAppearance.sectionName = pendingSectionName;
    symbolAppearance.sectionId = pendingSectionNo;
    symbolAppearance.offsetInSection = locationCounter + 3;
    symbolAppearance.type = 0;
    symbolAppearance.isData = 1;
    for (auto & elem1 : tableSymbols)
    {
      if(!elem1.symbolName.compare(noviSimbol.symbolName)){
        //cout << "Evo me xD" << endl;
        elem1.symbolReferencing.push_back(symbolAppearance);
        break;
      }
    }
    string retString = "0000";
    return retString;
  }
}

bool Assembler::checkIfWordDirective(string line){
  //cout << line << endl;
  if(line.at(0) != '.'){
    //cout << "Nije direktiva uopste." << ": " << line << endl;
    return false;
  }
  smatch wordDir;
  //cout<<line<<endl;
  bool found = false;
  string tip = line.substr(0, 5);
  //cout << tip << endl;
  if(!tip.compare(".word")){
    string argumenti = line.substr(6, -1);
    //cout << argumenti << endl;
    found = true;
    //cout << "Tu sam" << endl;
    if(regex_match(argumenti, regex_for_word_single)){
      //cout << "*** Pronasao word single ***" << endl;
      wordDirectiveWork(argumenti);
    }else{
      //cout << argumenti << endl;
      const char delimiter = ',';
      vector<string> out;
      tokenize(argumenti, delimiter, out);
      for (auto &s: out) {
        s = line_removeBlankCharactersAtBeginning(s);
        //cout << s << std::endl;
        if(regex_match(s, regex_word_1) || regex_match(s, regex_word_2) ||regex_match(s, regex_word_3) ||regex_match(s, regex_word_4)){
          //cout << "*** Pronasao WORD!!! single u listi***" << endl;
          wordDirectiveWork(s);
        }else{
          cout << "*** GRESKA ZA WORD!!! single u listi***" << line << endl;
          exit(-1);
        }
      }
    }
  }else{
    cout << "Nije poznata direktiva !!!" << endl;
    exit(-1);
  }
  return found;
}

bool Assembler::checkIfEndDirective(string line){
  smatch endDir;
  bool found = regex_search(line, endDir, regex_for_end);
  if(found == true){
    //cout << "*** Pronasao end ***" << endl;
    Assembler::assembling_finished = 1;
  }
  return found;
}

bool Assembler::checkIfSectionDirective(string line){
  //cout << "Line: " << line << endl;
  smatch sectionDir;
  bool found = regex_search(line, sectionDir, regex_for_section);
  if(found == true){
    //cout << "*** Pronasao section ***" << endl;
    string sectionName = line.substr(9, -1);
    int pronasao = 0;
    Entry_Section foundSection;
    for (auto & element : tableSections) {
      if(!element.sectionName.compare(sectionName)){
        pronasao = 1;
        foundSection = element;
        break;
      }
    } 
    if(pronasao == 0){
      Entry_Symbol foundSymbol;
      int pronasaoGresku = 0;
      for (auto & element: tableSymbols) {
        if(!element.symbolName.compare(sectionName)){
          pronasaoGresku = 1;
          break;
        }
      }
      if (pronasaoGresku){
        cout << "Ime sekcije vec postoji kao ime simbola: " << line << endl;
        exit(-1);
      }
      locationCounter = 0;
      //cout << "Nova sekcija" << endl;
      Entry_Section novaSekcija;
      //novaSekcija.sectionID = Assembler::symbols_ID_autoIncrement++;
      novaSekcija.sectionID = Assembler::sections_ID_autoIncrement++;
      novaSekcija.sectionName = sectionName;
      novaSekcija.sectionSize = 0;
      novaSekcija.beginningAddress = 0;
      tableSections.push_back(novaSekcija);
      Entry_Symbol noviSimlbol;
      noviSimlbol.definedSymbol = true;
      noviSimlbol.sectionName = sectionName;
      noviSimlbol.symbolSize = -1;
      noviSimlbol.symbolValue = 0;
      noviSimlbol.symbolName = sectionName;
      noviSimlbol.globalSymbol = false;
      noviSimlbol.isSection = true;
      noviSimlbol.symbolID = Assembler::symbols_ID_autoIncrement++;
      tableSymbols.push_back(noviSimlbol);
      pendingSectionNo = novaSekcija.sectionID;
      pendingSectionName = novaSekcija.sectionName;
    }else{
      //cout << "Postojeca sekcija" << endl;
      pendingSectionName = sectionName;
      //cout << pendingSectionName << endl;
      pendingSectionNo = foundSection.sectionID;
      //cout << pendingSectionNo << endl;
      locationCounter = foundSection.sectionSize;
      //cout << "Location counter je: " << foundSection.sectionSize << endl;
      //cout << locationCounter << endl;
    }
  }
  return found;
}

bool Assembler::checkForDirectiveInLine(string modified_line){
  if(checkIfGlobalDirective(modified_line)){
    return true;
  }else if(checkIfExternDirective(modified_line)){
    return true;
  }else if(checkIfSectionDirective(modified_line)){
    return true;
  }else if(checkIfSkipDirective(modified_line)){
    return true;
  }else if(checkIfEndDirective(modified_line)){
    return true;
  }else if(checkIfWordDirective(modified_line)){
    return true;
  }
  return false;
}

int Assembler::pretvoriIzHexUDec(string hex)
{
	int len = hex.size();
	int base = 1;
	int dec_val = 0;
	for (int i = len - 1; i >= 0; i--) {
		if (hex[i] >= '0' && hex[i] <= '9') {
			dec_val += (int(hex[i]) - 48) * base;
			base = base * 16;
		}
		else if (hex[i] >= 'A' && hex[i] <= 'F') {
			dec_val += (int(hex[i]) - 55) * base;
			base = base * 16;
		}
	}
	return dec_val;
}

int Assembler::dodajSimbol(string name, int tip, int offset){
  int postoji = 0;
  int brSimbola = -1;
  for(auto & elem: tableSymbols){
    if(!elem.symbolName.compare(name)){
      postoji = 1;
      brSimbola = elem.symbolID;
      break;
    }
  }
  if(postoji == 1){
    Entry_Symbol_Appearance flink;
    flink.sectionId = pendingSectionNo;
    flink.sectionName = pendingSectionName;
    flink.type = tip;
    flink.offsetInSection = offset;
    flink.isData = 1;
    tableSymbols[brSimbola].symbolReferencing.push_back(flink);
    return 0;
  }else{
    Entry_Symbol_Appearance flink;
    flink.sectionId = pendingSectionNo;
    flink.sectionName = pendingSectionName;
    flink.type = tip;
    flink.offsetInSection = offset;
    flink.isData = 1;
    Entry_Symbol noviSimbol;
    noviSimbol.definedSymbol = false;
    noviSimbol.globalSymbol = false;
    noviSimbol.isSection = false;
    noviSimbol.sectionName = pendingSectionName;
    noviSimbol.symbolID = symbols_ID_autoIncrement++;
    noviSimbol.symbolName = name;
    noviSimbol.symbolSize = -1;
    noviSimbol.symbolValue = 0;
    noviSimbol.symbolReferencing.push_back(flink);
    tableSymbols.push_back(noviSimbol);
    return 1;
  }
}

int Assembler::dohvatiAdresuSimbola(string simbolName){
  int val = 0;
  for(auto & elem: tableSymbols){
    if(!elem.symbolName.compare(simbolName)){
      val = elem.symbolValue;
      break;
    }
  }
  return val;
}

bool Assembler::checkForInstructionInLine(string modified_line){
  // !!! PROVERI DA LI SE PORED CODE, PUSHuje JOS NESTO, KAO NPR OFFSET
  if(pendingSectionNo == -1){
    cout << "Instrukcija se mora naci u sekciji. Sekcija nije zapoceta!" << endl;
    exit(-1);
  }else{
    smatch sm;
    if(regex_search(modified_line, sm, regex_for_instruction_with_no_operands)){
      //cout << "Pronasao instrukciju bez operanada: " << modified_line << endl;
      string code;
      string instructionName = sm.str(1);
      if(!instructionName.compare("halt")){
        code = "00";
      }else if(!instructionName.compare("iret")){
        code = "20";
      }else if(!instructionName.compare("ret")){
        code = "40";
      }
      //cout <<"Ime instrukcije: " << instructionName << " code: " << code << endl;
      for (auto & element: tableSections)
      {
        if(element.sectionID == pendingSectionNo){
          element.sectionSize = element.sectionSize + 1;
          for (int i = 0; i < code.size(); i++){
            element.sectionCode.push_back(code[i]);
          }
          break;
        }
      }
      locationCounter++;
      return true;
    }else if(regex_search(modified_line, sm, regex_for_instruction_with_one_operand)){
      //cout << "Pronasao instrukciju sa jednim operandom: " << modified_line << endl;
      string code;
      string instructionName = sm.str(1);
      string registerNumber = sm.str(2);
      if(!registerNumber.compare("psw")){
        registerNumber = "8";
      }else if(!registerNumber.compare("pc")){
        registerNumber = "7";
      }else if(!registerNumber.compare("sp")){
        registerNumber = "6";
      }else{
        registerNumber = registerNumber.substr(1, -1);
      }
      int sizeForIncrement = 0;
      if(!instructionName.compare("push")){
        code = "B0";
        code += registerNumber;
        code += "6"; // sp = r6
        code += "12"; // 1 - umanjuje se za 2 pre formiranja adrese operanda, 2 - regind
        sizeForIncrement = 3;
      }else if(!instructionName.compare("pop")){
        code = "A0";
        code += registerNumber;
        code += "6"; // sp = r6
        code += "42"; // 2 - uvecava se za 2 nakon formiranja adrese operanda, 2 - regind
        sizeForIncrement = 3;
      }else if(!instructionName.compare("int")){
        code = "10";
        code += registerNumber;
        code += "F";
        sizeForIncrement = 2;
      }else if(!instructionName.compare("not")){
        code = "8O";
        code += registerNumber;
        code += "F"; // drugi(source) registar se ne koristi, vec samo destination registar. Realno je mogao bilo koji broj 0-F
        sizeForIncrement = 2;
      }
      //cout <<"Ime instrukcije: " << instructionName << " code: " << code << endl;
      for (auto & element: tableSections)
      {
        if(element.sectionID == pendingSectionNo){
          element.sectionSize = element.sectionSize + sizeForIncrement;
          for (int i = 0; i < code.size(); i++){
            element.sectionCode.push_back(code[i]);
          }
          break;
        }
      }
      locationCounter += sizeForIncrement;
      return true;
    }else if(regex_search(modified_line, sm, regex_for_one_operand_jump_instruction)){
      //cout << "Pronasao instrukciju skoka sa jednim operandom: " << modified_line << endl;
      string code;
      string instructionName = sm.str(1);
      string operand = sm.str(2);
      //cout << instructionName << endl;
      //cout << operand << endl;
      int sizeForIncrement = 0;

      if(!instructionName.compare("jmp")){
        code = "50";
      }else if(!instructionName.compare("jeq")){
        code = "51";
      }else if(!instructionName.compare("jne")){
        code = "52";
      }else if(!instructionName.compare("jgt")){
        code = "53";
      }else if(!instructionName.compare("call")){
        code = "30";
      }

      code += "F";

      smatch om;
      if(regex_search(operand, om, regex_for_pc_rel_jump_instructions)){
        //cout << "PC REL JUMP" << endl;
        //cout << locationCounter << endl;
        code += "7"; // r7 = pc
        code += "05"; // nema azuriranja, regdir sa pomerajem
        string operandName = operand.substr(1, -1);
        //cout << operandName << endl;
        int ret = dodajSimbol(operandName, 1, locationCounter + 3);
        if(ret == 0){
          // Simbol vec postoji u tabeli simbola
          int br = -1;
          for(auto & elemSim : tableSymbols){
            if(!elemSim.symbolName.compare(operandName)){
              br = elemSim.symbolID;
              break;
            }
          }
          Entry_Reallocation realokacija;
          realokacija.location = locationCounter + 3;
          realokacija.refSym = br;
          realokacija.type = 1;
          realokacija.realSymbolNumber = br;
          realokacija.addend = 0;
          realokacija.realAddend = 0;
          int vrednost;
          if(tableSymbols[br].globalSymbol || !tableSymbols[br].definedSymbol){
            //cout << "Tu 1" << endl;
            vrednost = -2;
            string vrednostString = pretvoriUHexOblik(vrednost);
            //cout << vrednostString << endl;
            vrednostString = vrednostString.substr(vrednostString.size() - 4, 4);
            vrednostString = stringToUpper(vrednostString);
            code += vrednostString;
            //cout << vrednostString << endl;
            tableSections[pendingSectionNo].tableOfRealocations.push_back(realokacija);
          }else{
            //cout << "Tu 2" << endl;
            if(!tableSymbols[br].sectionName.compare(pendingSectionName)){
              //cout << "Tu 3" << endl;
              vrednost = tableSymbols[br].symbolValue - 2 - locationCounter - 3;
              //cout << vrednost << endl;
              string vrednostString = pretvoriUHexOblik(vrednost);
              //cout << vrednostString << endl;
              vrednostString = vrednostString.substr(vrednostString.size() - 4, 4);
              //cout << vrednostString << endl;
              vrednostString = stringToUpper(vrednostString);
              code += vrednostString;
              //cout << vrednostString << endl;
            }else{
              //cout << "Tu 4" << endl;
              vrednost = tableSymbols[br].symbolValue - 2;
              string vrednostString = pretvoriUHexOblik(vrednost);
              //vrednostString = simbolHexOblik(vrednostString);
              string noviString = simbolHexOblik(vrednostString);
              //cout << noviString << endl;
              noviString = noviString.substr(noviString.size() - 4, 4);
              noviString = stringToUpper(noviString);
              code += noviString;
              //cout << noviString << endl;
              tableSections[pendingSectionNo].tableOfRealocations.push_back(realokacija);
            }
          }
        }else{
          //cout << "Simbol ne postoji u tabeli simbola!" << endl;
          code += "0000";
          int br = -1;
          for(auto & elemSim : tableSymbols){
            if(!elemSim.symbolName.compare(operandName)){
              br = elemSim.symbolID;
              break;
            }
          }
          Entry_Reallocation realokacija;
          realokacija.location = locationCounter + 3;
          realokacija.refSym = br;
          realokacija.type = 1;
          realokacija.addend = 0;
          realokacija.realAddend = 0;
          realokacija.realSymbolNumber = br;
          tableSections[pendingSectionNo].tableOfRealocations.push_back(realokacija);
        }
        sizeForIncrement = 5;
        /*int ret = dodajSimbol(operandName, 1, locationCounter + 3); // !!! Proveri ovo da li je locationCounter + 3, ili samo 3
        if(ret == 0){
          // Simbol vec postoji u tabeli simbola
          int value = dohvatiAdresuSimbola(operandName);
          int razlika = value - (locationCounter + 5); // 5 ili 3???
          string razlikaHex = pretvoriUHexOblik(razlika);
          razlikaHex = razlikaHex.substr(razlikaHex.size()-4, 4);
          razlikaHex = stringToUpper(razlikaHex);
          code += razlikaHex.substr(2, 2);
          code += razlikaHex.substr(0, 2);
        }else{
          code += "0000";
        }
        sizeForIncrement = 5;*/
      }else if(regex_search(operand, om, regex_for_regdir_jump_instructions)){
        //cout << "REGDIR JUMP" << endl;
        string operandOnly = operand.substr(1, -1);
        string registerNumber;
        if(!operandOnly.compare("psw")){
          registerNumber = "8";
        }else if(!operandOnly.compare("pc")){
          registerNumber = "7";
        }else if(!operandOnly.compare("sp")){
          registerNumber = "6";
        }else{
          registerNumber = operandOnly.substr(1, -1);
        }
        //cout << registerNumber << endl;
        code += registerNumber;
        code += "0"; // nema azuriranja
        code += "1"; // regDir adresiranje
        sizeForIncrement = 3;
      }else if(regex_search(operand, om, regex_for_regind_jump_instructions)){
        //cout << "REGIND JUMP" << endl;
        //cout << operand << endl;
        string samoOperand = operand.substr(1, -1);
        string registerNumber;
        if(!samoOperand.compare("[psw]")){
          registerNumber = "8";
        }else if(!samoOperand.compare("[pc]")){
          registerNumber = "7";
        }else if(!samoOperand.compare("[sp]")){
          registerNumber = "6";
        }else{
          registerNumber = samoOperand.substr(2, 1);
        }
        //cout << registerNumber << endl;
        code += registerNumber;
        code += "0"; // nema azuriranja
        code += "2"; // regInd adresiranje
        sizeForIncrement = 3;
      }else if(regex_search(operand, om, regex_for_absolute_jump_instructions)){
        //cout << "ABS JUMP" << endl;
        code += "F"; // RegDescr3210, source registar, nebitan je, moglo je da stoji bilo sta, pa i 0
        code += "00"; // 0-nema azuriranja vrednosti, 0 - neposredno adresiranje
        //cout << operand << endl;
        if(regex_match(operand, regex_word_1) || regex_match(operand, regex_word_2)){
          if(regex_match(operand, regex_word_1)){
            string hexBroj = operand.substr(2, -1);
            hexBroj = simbolHexOblik(hexBroj);
            code += hexBroj[0];
            code += hexBroj[1];
            code += hexBroj[2];
            code += hexBroj[3];
            //cout << hexBroj << endl;
          }else{
            int decBroj = stoi(operand, nullptr, 0);
            string decBrojHex = pretvoriUHexOblik(decBroj);
            decBrojHex = stringToUpper(decBrojHex);
            decBrojHex = simbolHexOblik(decBrojHex);
            code += decBrojHex[0];
            code += decBrojHex[1];
            code += decBrojHex[2];
            code += decBrojHex[3];
            //cout << "DEC BROJ " << decBrojHex << endl;
          }
        }else if(regex_match(operand, regex_word_4)){
          //cout << operand << endl;
          string retString = odradiPosaoOkoSimbola(operand, locationCounter + 3, 2);
          //cout << "VRACENA VREDNOST: " << retString << endl;
          code += retString[0];
          code += retString[1]; 
          code += retString[2]; 
          code += retString[3];  
        }
        sizeForIncrement = 5;
      }else if(regex_search(operand, om, regex_for_memdir_jump_instructions)){
        //cout << "MEMDIR JUMP" << endl;
        string samoOperand = operand.substr(1, -1);
        code += "F"; //RegDescr3210 tj. source registar je nebitan, pa se moze staviti bilo sta, od F do cak 0
        code += "04"; // 0 - nema azuriranja vrednosti registra, 4 - MEM(operand je u mem. na adresi ukazanoj payload instrukcije)
        //cout << samoOperand << endl;
        /*if(samoOperand.size() == 3){
          if(samoOperand.at(0) == 'r'){
            cout << samoOperand.size() << endl;
            if(samoOperand.at(1) >= '0' && samoOperand.at(1) <= '9' && samoOperand.at(2) >= '0' && samoOperand.at(2) <= '9'){
              cout << "GRESKA! Registar moze biti samo od [0-8]! " << modified_line << endl;
              exit(-1); 
            }
          }
        }*/
        if(regex_match(samoOperand, regex_word_1)){
          string hexBroj = samoOperand.substr(2, -1);
          hexBroj = stringToUpper(hexBroj);
          hexBroj = simbolHexOblik(hexBroj);
          code += hexBroj[0];
          code += hexBroj[1];
          code += hexBroj[2];
          code += hexBroj[3];
          //cout << hexBroj << endl;
        }else if(regex_match(samoOperand, regex_word_2)){
          int decBroj = stoi(samoOperand, nullptr, 0);
          string decBrojHex = pretvoriUHexOblik(decBroj);
          decBrojHex = stringToUpper(decBrojHex);
          decBrojHex = simbolHexOblik(decBrojHex);
          code += decBrojHex[0];
          code += decBrojHex[1];
          code += decBrojHex[2];
          code += decBrojHex[3];
          //cout << "DEC BROJ " << decBrojHex << endl;
        }else if(regex_match(samoOperand, regex_word_4)){
          string retString = odradiPosaoOkoSimbola(samoOperand, locationCounter + 3, 2);
          //cout << "VRACENA VREDNOST: " << retString << endl;
          code += retString[0];
          code += retString[1]; 
          code += retString[2]; 
          code += retString[3];  
        }
        sizeForIncrement = 5;
      }else if(regex_search(operand, om, regex_for_regind_with_disp_jump_instructions)){
        //cout << "REGIND WITH DISP JUMP" << endl;
        string operandRegistar = om.str(1);
        string pomeraj = om.str(2);
        string registarBroj = odluciBrojRegistra(operandRegistar);
        code += registarBroj;
        code += "03"; // 0 - nema azuriranja vrednosti registra, 3 - regind sa 16b oznacenim pomerajem
        //cout << "Operand registar: " + operandRegistar << endl;
        //cout << "Pomeraj: " + pomeraj << endl;
        //cout << "Registar broj: " + registarBroj << endl;
        if(regex_match(pomeraj, regex_word_4)){
          string retString = odradiPosaoOkoSimbola(pomeraj, locationCounter + 3, 2);
          //cout << "VRACENA VREDNOST: " << retString << endl;
          code += retString[0];
          code += retString[1]; 
          code += retString[2]; 
          code += retString[3]; 
        }else{
          //cout << "Radi se o literalu kao pomeraju" << endl;
          if(regex_match(pomeraj, regex_word_1)){
            string hexBroj = pomeraj.substr(2, -1);
            hexBroj = stringToUpper(hexBroj);
            hexBroj = simbolHexOblik(hexBroj);
            code += hexBroj[0];
            code += hexBroj[1];
            code += hexBroj[2];
            code += hexBroj[3];
            //cout << hexBroj << endl;
          }else if(regex_match(pomeraj, regex_word_2)){
            int decBroj = stoi(pomeraj, nullptr, 0);
            string decBrojHex = pretvoriUHexOblik(decBroj);
            decBrojHex = stringToUpper(decBrojHex);
            decBrojHex = simbolHexOblik(decBrojHex);
            code += decBrojHex[0];
            code += decBrojHex[1];
            code += decBrojHex[2];
            code += decBrojHex[3];
            //cout << "DEC BROJ " << decBrojHex << endl;
          }else if(regex_match(pomeraj, regex_word_3)){
            int decBroj = stoi(pomeraj, nullptr, 0);
            string decBrojHex = pretvoriUHexOblik(decBroj);
            decBrojHex = decBrojHex.substr(decBrojHex.size()-4, 4);
            decBrojHex = stringToUpper(decBrojHex);
            code += decBrojHex[0];
            code += decBrojHex[1];
            code += decBrojHex[2];
            code += decBrojHex[3];
            //cout << decBrojHex << endl;
          }
        }
        sizeForIncrement = 5;
      }else{
        cout << "Greska! Proverite registre ili nacin adresiranja:  " << modified_line << endl;
        exit(-1);
      }
      for (auto & element: tableSections)
      {
        if(element.sectionID == pendingSectionNo){
          element.sectionSize = element.sectionSize + sizeForIncrement;
          for (int i = 0; i < code.size(); i++){
            element.sectionCode.push_back(code[i]);
          }
          break;
        }
      }
      locationCounter += sizeForIncrement;
      return true;
    }else if(regex_search(modified_line, sm, regex_for_instruction_with_two_operands_ldr_str)){
      //cout << "Pronasao ldr/str instrukciju sa dva operanda: " << modified_line << endl;
      string code;
      string instructionName = sm.str(1);
      string registerNumber1 = sm.str(2);
      string operand = sm.str(3);
      operand = line_removeBlankCharactersAtBeginning(operand);
      int sizeForIncrement = 0;

      if(!registerNumber1.compare("sp")){
        registerNumber1 = '6';
      }else if (!registerNumber1.compare("pc")){
        registerNumber1 = '7';
      }else if (!registerNumber1.compare("psw")){
        registerNumber1 = '8';
      }else{
        registerNumber1 = registerNumber1.substr(1, -1);
      }

      if(!instructionName.compare("ldr")){
        code = "A0";
      }else if(!instructionName.compare("str")){
        code = "B0";
      }

      smatch om;
      if(regex_search(operand, om, regex_for_absolute_addr_ldr_str)){
        //cout << "Absolute adresiranje za ldr/str" << endl;
        string literal = om.str(1);
        code += registerNumber1;
        code += "F"; //RegDescr3210, source registar nije bitan, moze da stoji bilo sta, pa i 0
        code += "00"; // 0 - nema azuriranja vrednosti registra, 0 - neposredno adresiranje
        if(regex_match(literal, regex_word_1)){
          string hexBroj = literal.substr(2, -1);
          hexBroj = stringToUpper(hexBroj);
          hexBroj = simbolHexOblik(hexBroj);
          code += hexBroj[0];
          code += hexBroj[1];
          code += hexBroj[2];
          code += hexBroj[3];
          //cout << hexBroj << endl;
        }else if(regex_match(literal, regex_word_2)){
          int decBroj = stoi(literal, nullptr, 0);
          string decBrojHex = pretvoriUHexOblik(decBroj);
          decBrojHex = stringToUpper(decBrojHex);
          decBrojHex = simbolHexOblik(decBrojHex);
          code += decBrojHex[0];
          code += decBrojHex[1];
          code += decBrojHex[2];
          code += decBrojHex[3];
          //cout << "POS DEC BROJ " << decBrojHex << endl;
        }else if(regex_match(literal, regex_word_3)){
          int decBroj = stoi(literal, nullptr, 0);
          string decBrojHex = pretvoriUHexOblik(decBroj);
          decBrojHex = decBrojHex.substr(decBrojHex.size()-4, 4);
          decBrojHex = stringToUpper(decBrojHex);
          code += decBrojHex[0];
          code += decBrojHex[1];
          code += decBrojHex[2];
          code += decBrojHex[3];
          //cout << "NEG DEC BROJ " << decBrojHex << endl;
        }else if(regex_match(literal, regex_word_4)){
          string retString = odradiPosaoOkoSimbola(literal, locationCounter + 3, 2);
          //cout << "VRACENA VREDNOST: " << retString << endl;
          code += retString[0]; 
          code += retString[1]; 
          code += retString[2];
          code += retString[3]; 
        }
        sizeForIncrement = 5;
      }else if(regex_search(operand, om, regex_for_regdir_addr_ldr_str)){
        //cout << "REGDIR adresiranje za ldr/str" << endl;
        sizeForIncrement = 3;
        string operandRegister;
        if(!operand.compare("psw")){
          operandRegister = "8";
        }else if(!operand.compare("sp")){
          operandRegister = "6";
        }else if(!operand.compare("pc")){
          operandRegister = "7";
        }else {
          operandRegister = operand.substr(1, -1);
        }
        code += registerNumber1;
        code += operandRegister;
        code += "0"; // Up3..Up0 su 0b0000, nema azuriranja vrednosti odredjenog registra
        code += "1"; // U pitanju je regdir adresiranje Am3..Am0 su 0b0001
      }else if(regex_search(operand, om, regex_for_regind_addr_ldr_str)){
        //cout << "REGIND adresiranje za ldr/str" << endl;
        sizeForIncrement = 3;
        string operandRegister;
        //cout << "OPERAND: " << operand << endl;
        if(!operand.compare("[psw]")){
          operandRegister = "8";
        }else if(!operand.compare("[sp]")){
          operandRegister = "6";
        }else if(!operand.compare("[pc]")){
          operandRegister = "7";
        }else {
          operandRegister = operand.substr(2, 1);
        }
        code += registerNumber1;
        code += operandRegister;
        code += "0"; // Up3..Up0 su 0b0000, nema azuriranja vrednosti odredjenog registra
        code += "2"; // U pitanju je regind adresiranje Am3..Am0 su 0b0010
      }else if(regex_search(operand, om, regex_for_pc_rel_addr_ldr_str)){
        //cout << "PCREL adresiranje za ldr, str" << endl;
        operand = operand.substr(1, -1);
        //cout << operand << endl;
        code += registerNumber1;
        code += "7"; // r7=pc, radi se o pc rel adresiranju
        code += "03"; // 0 - nema azuriranja vrednosti registra, 3 - regind sa 16b oznacenim pomerajem
        int ret = dodajSimbol(operand, 1, locationCounter + 3);
        if(ret == 0){
          // Simbol vec postoji u tabeli simbola
          int br = -1;
          for(auto & elemSim : tableSymbols){
            if(!elemSim.symbolName.compare(operand)){
              br = elemSim.symbolID;
              break;
            }
          }
          Entry_Reallocation realokacija;
          realokacija.location = locationCounter + 3;
          realokacija.refSym = br;
          realokacija.type = 1;
          realokacija.addend = 0;
          realokacija.realAddend = 0;
          realokacija.realSymbolNumber = br;
          int vrednost;
          if(tableSymbols[br].globalSymbol || !tableSymbols[br].definedSymbol){
            //cout << "Tu 1" << endl;
            vrednost = -2;
            string vrednostString = pretvoriUHexOblik(vrednost);
            //cout << vrednostString << endl;
            vrednostString = vrednostString.substr(vrednostString.size() - 4, 4);
            vrednostString = stringToUpper(vrednostString);
            code += vrednostString;
            //cout << vrednostString << endl;
            tableSections[pendingSectionNo].tableOfRealocations.push_back(realokacija);
          }else{
            //cout << "Tu 2" << endl;
            if(!tableSymbols[br].sectionName.compare(pendingSectionName)){
              //cout << "Tu 3" << endl;
              vrednost = tableSymbols[br].symbolValue - 2 - locationCounter - 3;
              //cout << vrednost << endl;
              string vrednostString = pretvoriUHexOblik(vrednost);
              //cout << vrednostString << endl;
              vrednostString = vrednostString.substr(vrednostString.size() - 4, 4);
              //cout << vrednostString << endl;
              vrednostString = stringToUpper(vrednostString);
              code += vrednostString;
              //cout << vrednostString << endl;
            }else{
              //cout << "Tu 4" << endl;
              vrednost = tableSymbols[br].symbolValue - 2;
              string vrednostString = pretvoriUHexOblik(vrednost);
              //vrednostString = simbolHexOblik(vrednostString);
              string noviString = simbolHexOblik(vrednostString);
              //cout << noviString << endl;
              noviString = noviString.substr(noviString.size() - 4, 4);
              noviString = stringToUpper(noviString);
              code += noviString;
              //cout << noviString << endl;
              tableSections[pendingSectionNo].tableOfRealocations.push_back(realokacija);
            }
          }
        }else{
          // Simbol ne postoji u tabeli simbola
          code += "0000";
          int br = -1;
          for(auto & elemSim : tableSymbols){
            if(!elemSim.symbolName.compare(operand)){
              br = elemSim.symbolID;
              break;
            }
          }
          Entry_Reallocation realokacija;
          realokacija.location = locationCounter + 3;
          realokacija.refSym = br;
          realokacija.type = 1;
          realokacija.addend = 0;
          realokacija.realAddend = 0;
          realokacija.realSymbolNumber = br;
          tableSections[pendingSectionNo].tableOfRealocations.push_back(realokacija);
        }
        sizeForIncrement = 5;
      }else if(regex_search(operand, om, regex_for_regind_with_disp_addr_ldr_str)){
        //cout << "REGIND WITH DISP adresiranje za ldr, str" << endl;
        string registar = om.str(1);
        registar = odluciBrojRegistra(registar);
        string pomerajLiteralOrSimbol = om.str(2);
        code += registerNumber1;
        code += registar;
        code += "03"; // 0 - nema azuriranja vrednosti registra, 3 - regind sa 16b oznacenim pomerajem
        if(regex_match(pomerajLiteralOrSimbol, regex_word_1)){
          string hexBroj = pomerajLiteralOrSimbol.substr(2, -1);
          hexBroj = stringToUpper(hexBroj);
          hexBroj = simbolHexOblik(hexBroj);
          code += hexBroj[0];
          code += hexBroj[1];
          code += hexBroj[2];
          code += hexBroj[3];
          //cout << hexBroj << endl;
        }else if(regex_match(pomerajLiteralOrSimbol, regex_word_2)){
          int decBroj = stoi(pomerajLiteralOrSimbol, nullptr, 0);
          string decBrojHex = pretvoriUHexOblik(decBroj);
          decBrojHex = stringToUpper(decBrojHex);
          decBrojHex = simbolHexOblik(decBrojHex);
          code += decBrojHex[0];
          code += decBrojHex[1];
          code += decBrojHex[2];
          code += decBrojHex[3];
          //cout << "POS DEC BROJ " << decBrojHex << endl;
        }else if(regex_match(pomerajLiteralOrSimbol, regex_word_3)){
          int decBroj = stoi(pomerajLiteralOrSimbol, nullptr, 0);
          string decBrojHex = pretvoriUHexOblik(decBroj);
          decBrojHex = decBrojHex.substr(decBrojHex.size()-4, 4);
          decBrojHex = stringToUpper(decBrojHex);
          code += decBrojHex[0];
          code += decBrojHex[1];
          code += decBrojHex[2];
          code += decBrojHex[3];
          //cout << "NEG DEC BROJ " << decBrojHex << endl;
        }else if(regex_match(pomerajLiteralOrSimbol, regex_word_4)){
          string retString = odradiPosaoOkoSimbola(pomerajLiteralOrSimbol, locationCounter + 3, 2);
          //cout << "VRACENA VREDNOST: " << retString << endl;
          code += retString[0];
          code += retString[1]; 
          code += retString[2]; 
          code += retString[3]; 
        }
        sizeForIncrement = 5;
      }else if(regex_search(operand, om, regex_for_memdir_addr_ldr_str) || regex_search(operand, om, regex_word_4)){
        //cout << "MEMDIR adresiranje za ldr, str" << endl;
        //cout << operand << endl;
        /*if(operand.size() == 3){
          if(operand.at(0) == 'r'){
            cout << operand.size() << endl;
            if(operand.at(1) >= '0' && operand.at(1) <= '9' && operand.at(2) >= '0' && operand.at(2) <= '9'){
              cout << "GRESKA! Registar moze biti samo od [0-8]! " << modified_line << endl;
              exit(-1); 
            }
          }
        }*/
        code += registerNumber1;
        code += "F"; // RegDescr3210, source register nije bitno, moze da stoji bilo sta od 0-F
        code += "04"; // 0 - nema azuriranja vrednosti registra, 4 - mem dir adresiranje, operand je u mem. na adresi ukazanoj payload instrukcije
        if(regex_match(operand, regex_word_1)){
          string hexBroj = operand.substr(2, -1);
          hexBroj = stringToUpper(hexBroj);
          hexBroj = simbolHexOblik(hexBroj);
          code += hexBroj[0];
          code += hexBroj[1];
          code += hexBroj[2];
          code += hexBroj[3];
          //cout << hexBroj << endl;
        }else if(regex_match(operand, regex_word_2)){
          int decBroj = stoi(operand, nullptr, 0);
          string decBrojHex = pretvoriUHexOblik(decBroj);
          decBrojHex = stringToUpper(decBrojHex);
          decBrojHex = simbolHexOblik(decBrojHex);
          code += decBrojHex[0];
          code += decBrojHex[1];
          code += decBrojHex[2];
          code += decBrojHex[3];
          //cout << "POS DEC BROJ " << decBrojHex << endl;
        }else if(regex_match(operand, regex_word_4)){
          string retString = odradiPosaoOkoSimbola(operand, locationCounter + 3, 2);
          //cout << "VRACENA VREDNOST: " << retString << endl;
          code += retString[0];
          code += retString[1]; 
          code += retString[2]; 
          code += retString[3]; 
        }
        sizeForIncrement = 5;
      }else {
        //cout << operand << endl;
        cout << "Greska! Proverite registre ili nacin adresiranja: " << modified_line << endl;
        exit(-1);
      }
      
      //cout <<"Ime instrukcije: " << instructionName << " code: " << code << endl;
      for (auto & element: tableSections)
      {
        if(element.sectionID == pendingSectionNo){
          element.sectionSize = element.sectionSize + sizeForIncrement;
          for (int i = 0; i < code.size(); i++){
            element.sectionCode.push_back(code[i]);
          }
          break;
        }
      }
      locationCounter += sizeForIncrement;
      return true;
    }else if(regex_match(modified_line, sm, regex_for_other_instructions_with_two_operands)){
      //cout << "Pronasao neku od ostalih instrukcija sa dva operanda: " << modified_line << endl;
      string code;
      string instructionName = sm.str(1);
      string registerNumber1 = sm.str(2);
      string registerNumber2 = sm.str(3);
      //cout << registerNumber1 << " - " << registerNumber2 << endl;
      int sizeForIncrement = 0;
      if(!registerNumber1.compare("sp")){
        registerNumber1 = '6';
      }else if (!registerNumber1.compare("pc")){
        registerNumber1 = '7';
      }else if (!registerNumber1.compare("psw")){
        registerNumber1 = '8';
      }else{
        registerNumber1 = registerNumber1.substr(1, -1);
      }

      if(!registerNumber2.compare("sp")){
        registerNumber2 = '6';
      }else if (!registerNumber2.compare("pc")){
        registerNumber2 = '7';
      }else if (!registerNumber2.compare("psw")){
        registerNumber2 = '8';
      }else{
        registerNumber2 = registerNumber2.substr(1, -1);
      }

      if(!instructionName.compare("xchg")){
        code = "60";
        code += registerNumber1;
        code += registerNumber2;
        sizeForIncrement = 2;
      }else if(!instructionName.compare("add")){
        code = "70";
        code += registerNumber1;
        code += registerNumber2;
        sizeForIncrement = 2;
      }else if(!instructionName.compare("sub")){
        code = "71";
        code += registerNumber1;
        code += registerNumber2;
        sizeForIncrement = 2;
      }else if(!instructionName.compare("mul")){
        code = "72";
        code += registerNumber1;
        code += registerNumber2;
        sizeForIncrement = 2;
      }else if(!instructionName.compare("div")){
        code = "73";
        code += registerNumber1;
        code += registerNumber2;
        sizeForIncrement = 2;
      }else if(!instructionName.compare("cmp")){
        code = "74";
        code += registerNumber1;
        code += registerNumber2;
        sizeForIncrement = 2;
      }else if(!instructionName.compare("and")){
        code = "81";
        code += registerNumber1;
        code += registerNumber2;
        sizeForIncrement = 2;
      }else if(!instructionName.compare("or")){
        code = "82";
        code += registerNumber1;
        code += registerNumber2;
        sizeForIncrement = 2;
      }else if(!instructionName.compare("xor")){
        code = "83";
        code += registerNumber1;
        code += registerNumber2;
        sizeForIncrement = 2;
      }else if(!instructionName.compare("test")){
        code = "84";
        code += registerNumber1;
        code += registerNumber2;
        sizeForIncrement = 2;
      }else if(!instructionName.compare("shl")){
        code = "90";
        code += registerNumber1;
        code += registerNumber2;
        sizeForIncrement = 2;
      }else if(!instructionName.compare("shr")){
        code = "91";
        code += registerNumber1;
        code += registerNumber2;
        sizeForIncrement = 2;
      }
      //cout <<"Ime instrukcije: " << instructionName << " code: " << code << endl;
      for (auto & element: tableSections)
      {
        if(element.sectionID == pendingSectionNo){
          element.sectionSize = element.sectionSize + sizeForIncrement;
          for (int i = 0; i < code.size(); i++){
            element.sectionCode.push_back(code[i]);
          }
          break;
        }
      }
      locationCounter += sizeForIncrement;
      return true;
    }else{
      cout << "Nije prepoznata instrukcija! " << modified_line << endl;
      exit(-1);
    }
  }
  return false;
}

void Assembler::initializeAssembler(){
  Entry_Section nedefinisanaSekcija;
  nedefinisanaSekcija.sectionID = sections_ID_autoIncrement++;
  nedefinisanaSekcija.sectionName = "UNDEFINED";
  nedefinisanaSekcija.sectionSize = 0;
  nedefinisanaSekcija.beginningAddress = 0;
  tableSections.push_back(nedefinisanaSekcija);

  Entry_Symbol nedefinisaniSimbol;
  nedefinisaniSimbol.symbolName = "UNDEFINED";
  nedefinisaniSimbol.symbolSize = 0;
  nedefinisaniSimbol.symbolID = symbols_ID_autoIncrement++;
  nedefinisaniSimbol.definedSymbol = true;
  nedefinisaniSimbol.globalSymbol = false;
  nedefinisaniSimbol.sectionName = "UNDEFINED";
  nedefinisaniSimbol.symbolValue = 0;
  nedefinisaniSimbol.isSection = true;
  tableSymbols.push_back(nedefinisaniSimbol);
}

string Assembler::line_removeComments(string line){
  int position = line.find_first_of('#');
  string newString = "";
  if(position != 0){
    newString = line.substr(0, position-1);
  }
  return newString;
}

string Assembler::line_removeBlankCharactersAtBeginning(string line){
  int position = line.find_first_not_of(' ');
  string newString = line;
  if(position != 0){
    newString = line.erase(0, position);
  }
  return newString;
}

string Assembler::line_removeAdditionalSpacesOrTabulator(string line){
  string newLine = regex_replace(line, regex_additionalSpace, " ");
  return newLine;
}

string Assembler::line_removePunctuationSpace(string line){
  string newLine = regex_replace(line, regex_punctuationSpace, ", ");
  return newLine;
}

void Assembler::isThereUndefinedSymbol(){
  //cout << "________________________________" << endl;
  if(tableSymbols.size() > 1){
    for(auto & element : tableSymbols){
      if((element.symbolName.compare("UNDEFINED") && !element.definedSymbol && !element.globalSymbol) || (
        element.symbolName.compare("UNDEFINED") && !element.definedSymbol && !element.sectionName.compare("NEPOZNATO")
      )){
        cout << "GASIM! Simbol nije definisan: " + element.symbolName << endl;
        exit(-1);
      }
    }
  }else{
    cout << "Niko nije ubacen u tabelu simbola" << endl;
  }
}

void Assembler::prepraviTabeluRelokacionihZapisa(){
  for(auto & elemSek : tableSections){
    for(auto & elemRel : elemSek.tableOfRealocations){
      int brSimb = elemRel.realSymbolNumber;
      //cout << brSimb << endl;
      bool globalanLokalan = tableSymbols[brSimb].globalSymbol;
      if(!globalanLokalan){
        //cout << "Azuriranje na lokalan" << endl;
        string brSekcString = tableSymbols[brSimb].sectionName;
        //cout << brSekcString << endl;
        int brojSekcije = -1;
        for(auto & elemSimbola : tableSymbols){
          if(!elemSimbola.symbolName.compare(brSekcString)){
            brojSekcije = elemSimbola.symbolID;
            //cout << brojSekcije << endl;
            break;
          }
        }
        /*int brojSekcije = -1;
        for(auto & elemSekcijeNovi : tableSections){
          if(!elemSekcijeNovi.sectionName.compare(brSekcString)){
            cout << brojSekcije << endl;
            brojSekcije = elemSekcijeNovi.sectionID;
            break;
          }
        }*/
        elemRel.refSym = brojSekcije;
      }
    }
  }
}

void Assembler::izbrisiNepotrebneRelokacije(){
  for(auto & elemSek : tableSections){
    for(vector<Entry_Reallocation>::iterator it = elemSek.tableOfRealocations.begin() ; it != elemSek.tableOfRealocations.end() ;){
      int brSimbola = (*it).realSymbolNumber;
      string sekcijaSimbola = tableSymbols[brSimbola].sectionName;
      string sekcijaRelokacije = elemSek.sectionName;
      int tipRelokacije = (*it).type;
      if(tipRelokacije == 1 && (!sekcijaSimbola.compare(sekcijaRelokacije))){
        it = elemSek.tableOfRealocations.erase(it);
      }else{
        ++it;
      }
    }
  }
}

void Assembler::backpatch(){
  /*for(auto & nesto : tableSymbols){
    for(auto & nesto2 : nesto.symbolReferencing){
      cout << nesto2.offsetInSection << ":" << nesto2.sectionId << ":" << nesto2.sectionName << ":" << nesto2.type << ":" << nesto2.isData << endl;
    }
  }*/
  //cout << "Usao u backpatch metodu!" << endl;
  prepraviTabeluRelokacionihZapisa();
  for(auto & elem : tableSymbols){
    for(auto & elem1 : elem.symbolReferencing){
      int pozicija = elem1.offsetInSection;
      string sekcijaIme = elem1.sectionName;
      int sekcijaBroj = elem1.sectionId;
      //cout << "Pozicija" << endl;
      //cout << pozicija << endl;
      int vrednostSimbola = elem.symbolValue;
      int tipPrepravke = elem1.type; // 0 za simbol, 1 za pc relativno adresiranje kod instrukcija
      if(elem.definedSymbol && (elem.sectionName.compare("UNDEFINED") && elem.sectionName.compare("NEPOZNATO"))){
        //cout << "Treba prepravka" << endl;
        string vrednostHex = pretvoriUHexOblik(vrednostSimbola);
        vrednostHex = stringToUpper(vrednostHex);
        vrednostHex = simbolHexOblik(vrednostHex);
        //cout << vrednostHex << endl;
        if(tipPrepravke == 0){
          //cout << "PREPRAVKA ZA SIMBOL" << endl;
          if(elem1.isData == 1){
            //cout << "OVDE 1" << endl;
            if(elem.globalSymbol){
              vrednostHex = "0000";
            }
            tableSections[sekcijaBroj].sectionCode[pozicija*2] = vrednostHex[0];
            tableSections[sekcijaBroj].sectionCode[pozicija*2 + 1] = vrednostHex[1];
            tableSections[sekcijaBroj].sectionCode[pozicija*2 + 2] = vrednostHex[2];
            tableSections[sekcijaBroj].sectionCode[pozicija*2 + 3] =  vrednostHex[3];
          }else{
            if(elem.globalSymbol){
              vrednostHex = "0000";
            }
            //cout << "OVDE 2" << endl;
            tableSections[sekcijaBroj].sectionCode[pozicija*2] = vrednostHex[2];
            tableSections[sekcijaBroj].sectionCode[pozicija*2 + 1] = vrednostHex[3];
            tableSections[sekcijaBroj].sectionCode[pozicija*2 + 2] = vrednostHex[0];
            tableSections[sekcijaBroj].sectionCode[pozicija*2 + 3] =  vrednostHex[1];
          }
        }else{
          //cout << "PC relativna prepravka" << endl;
          //cout << elem.symbolName << " " << elem.symbolValue << endl;
          int vrednost = elem.symbolValue - pozicija - 2;
          //cout << "IZRACUNATA VREDNOST: " << vrednost << endl;
          //cout << elem1.sectionName << endl;
          string hexVrednost = pretvoriUHexOblik(vrednost);
          hexVrednost = simbolHexOblik(hexVrednost);
          //cout << hexVrednost << endl;
          hexVrednost = stringToUpper(hexVrednost);
          string simbolSekcijaDefinisan = elem.sectionName;
          //cout << simbolSekcijaDefinisan << endl;
          if(simbolSekcijaDefinisan.compare(elem1.sectionName)){
            //cout << "Tu" << endl;
            vrednost = elem.symbolValue - 2;
            hexVrednost = pretvoriUHexOblik(vrednost);
            hexVrednost = simbolHexOblik(hexVrednost);
            hexVrednost = stringToUpper(hexVrednost);
            //cout << hexVrednost << endl;
          }
          // Sa ovim zakomentarisanim a bez donjeg if-a na liniji 1830, ispis mi je kao njemu
          /*if(elem.globalSymbol){ // OVAKO JE BILO PA JE NA ONOM JEDNOM TESTU MI ISPISIVALO FF FE za globalan koji je def. u sekc.
            cout << "GLOBALAN JE" << endl; // A to je kad je globalan iz neke druge sekcije
            vrednost = -2; // KAD JE globalan, definisan u nasoj sekciji, ide se addend + value - offset
            hexVrednost = pretvoriUHexOblik(vrednost);
            hexVrednost = stringToUpper(hexVrednost);
            cout << hexVrednost << endl;
          }*/
          if(elem.globalSymbol && simbolSekcijaDefinisan.compare(elem1.sectionName)){
            //cout << "GLOBALAN JE" << endl;
            for(auto & elemRel: tableSections[sekcijaBroj].tableOfRealocations){
              if(elemRel.location == pozicija){
                elemRel.realAddend = -2;
              }
            }
            vrednost = -2;
            hexVrednost = pretvoriUHexOblik(vrednost);
            hexVrednost = stringToUpper(hexVrednost);
            //cout << hexVrednost << endl;
          }else{
            for(auto & elemRel: tableSections[sekcijaBroj].tableOfRealocations){
              if(elemRel.location == pozicija){
                elemRel.realAddend = elem.symbolValue -2;
              }
            }
          }
          if(vrednost < 0){
            hexVrednost = hexVrednost.substr(hexVrednost.size() - 4, 4);
          }
          tableSections[sekcijaBroj].sectionCode[pozicija*2] = hexVrednost[0];
          tableSections[sekcijaBroj].sectionCode[pozicija*2 + 1] = hexVrednost[1];
          tableSections[sekcijaBroj].sectionCode[pozicija*2 + 2] = hexVrednost[2];
          tableSections[sekcijaBroj].sectionCode[pozicija*2 + 3] =  hexVrednost[3];
          //cout << hexVrednost << endl;
        }
      }else{
        //cout << "Ne moze da se zakljuci nista za prepravku" << endl;
        // Tu vrv treba da se doda relokacioni zapis za linker
        if(elem.globalSymbol && !elem.definedSymbol && elem1.type){
          // extern simbol
          for(auto & elemRel: tableSections[sekcijaBroj].tableOfRealocations){
            if(elemRel.location == pozicija){
              elemRel.realAddend = -2;
            }
          }
          int hexVrednostInt = -2;
          string hexVrednost = pretvoriUHexOblik(hexVrednostInt);
          hexVrednost = stringToUpper(hexVrednost);
          hexVrednost = hexVrednost.substr(hexVrednost.size() - 4, 4);
          tableSections[sekcijaBroj].sectionCode[pozicija*2] = hexVrednost[0];
          tableSections[sekcijaBroj].sectionCode[pozicija*2 + 1] = hexVrednost[1];
          tableSections[sekcijaBroj].sectionCode[pozicija*2 + 2] = hexVrednost[2];
          tableSections[sekcijaBroj].sectionCode[pozicija*2 + 3] =  hexVrednost[3];
          //cout << hexVrednost << endl;
        }
      }
    }
  }
  izbrisiNepotrebneRelokacije();
}

void Assembler::ispisiTabeluSekcija(fstream& izlazniFajl){
  izlazniFajl << "---TABELA SEKCIJA---" << endl;
  izlazniFajl << "--------------------" << endl;
  izlazniFajl << "No.     " << "     ||     " << "     Name     " << "     ||     " << "   Size     " << "     ||    " << "     Beginning address    " << endl;
  izlazniFajl << "____________________________________________________________________________________________________" << endl;
  for(auto & element : tableSections){
    izlazniFajl << setw(5) << " " << element.sectionID << setw(25 - element.sectionName.size()) << " " <<element.sectionName << setw(20) << " " << element.sectionSize << setw(26 - to_string(element.sectionSize).size()) << " " << element.beginningAddress << endl;
  }
}

void Assembler::ispisiTabeluSimbola(fstream& izlazniFajl){
  //izlazniFajl << endl;
  //izlazniFajl << endl;
  izlazniFajl << "---TABELA SIMBOLA---" << endl;
  izlazniFajl << "  SectionName  |  SymbolID   |   SymbolName   |   TypeOfSymbol   |   DefinedSymbol  |  SymbolValue  |  SymbolSize  | SekcijaJeDaNe" << endl;
  izlazniFajl << "___________________________________________________________________________________________________________________________________" << endl;
  for (auto & element: tableSymbols){
    string tip = "";
    string def = "";
    string sekc = "";
    if(element.globalSymbol){
      tip = "Global";
    }else{
      tip = "Local";
    }
    if(element.definedSymbol){
      def = "DEF";
    }else{
      def = "UND";
    }
    if(element.isSection){
      sekc = "DA";
    }else{
      sekc = "NE";
    }
    int stvVel = -1;
    if(element.isSection){
      for (size_t i = 0; i < tableSections.size(); i++)
      {
        if(!tableSections[i].sectionName.compare(element.sectionName)){
          stvVel = tableSections[i].sectionSize;
          break;
        }
      }
      
    }
    izlazniFajl << setw(5) << " " << element.sectionName << setw(15-element.sectionName.size()) << " " << element.symbolID << setw(24-element.symbolName.size() - to_string(element.symbolID).size()) << " " << element.symbolName << setw(17-tip.size()) << " " << tip << setw(16 - def.size()) << " " << def;
    izlazniFajl << setw(14) << " " << element.symbolValue << setw(18 - to_string(element.symbolValue).size()) << " " << stvVel << setw(11 - to_string(stvVel).size()) << " " << sekc << endl;
  } 
}

void Assembler::ispisiKodSekcije(fstream& izlazniFajl){
  izlazniFajl << endl;
  izlazniFajl << "KOD SEKCIJA:" << endl;
  izlazniFajl << "_______________________________________" << endl;
  for(auto & elem: tableSections){
    if(elem.sectionName.compare("UNDEFINED")){
      izlazniFajl << "section: " << elem.sectionName << endl;
      int br = 0;
      for(int i = 0; i < elem.sectionCode.size(); i++){
        br++;
        if(i == 0){
          izlazniFajl << "0: " << setw(9) << " ";
        }
        if((i % 2 == 0) && i != 0){
          izlazniFajl << elem.sectionCode[i];
        }else{
          izlazniFajl << elem.sectionCode[i];
        }
        if(br < 16 && br % 2 == 0){
          izlazniFajl << " ";
        }
        if(br == 16 && (i + 1) != elem.sectionCode.size()){
          br = 0;
          izlazniFajl << endl;
          int broj = i + 1;
          string hexBroj = pretvoriUHexOblik(broj / 2);
          izlazniFajl << hexBroj << ": " << setw(10 - hexBroj.size()) << " ";
        }
      }
      izlazniFajl << endl;
      izlazniFajl << endl;
    }
  }
}

void Assembler::ispisiRelokacioneZapise(fstream& outputFile){
  outputFile << endl;
  outputFile << "RELOKACIONI ZAPISI" << endl;
  outputFile << "_________________________________";
  for (auto & elemSec: tableSections)
  {
    if(elemSec.tableOfRealocations.size() > 0){
      outputFile << endl << "section: " << elemSec.sectionName << endl;
      outputFile << "Referisani simbol --- ||--- Location ---|| Tip relok.(0-simbol word, 1-pc rel addr, 2-simbol instr)||-- Addend --||-- Real addend --" << endl;
    }
    for(auto & elemRelloc: elemSec.tableOfRealocations){
      string locationHex = pretvoriUHexOblik(elemRelloc.location);
      outputFile << setw(10) << " " << elemRelloc.refSym << setw(18 - to_string(elemRelloc.refSym).size()) << " " << elemRelloc.location << " == 0x" << locationHex << setw(37 - locationHex.size() - to_string(elemRelloc.location).size()) << " " << elemRelloc.type << setw(35) << " " << elemRelloc.addend << setw(15) << " " << elemRelloc.realAddend << endl;
    }
  }
  
}

void Assembler::generateOutputFile(){
  //cout << "---GENERISANJE IZLAZNOG FAJLA---" << endl;
  //cout << output_file_name << endl;
  int pozicija = output_file_name.find_first_of("/");
  //cout << pozicija << endl;
  string folder = "";
  string novo_ime = "asembler_" + output_file_name.substr(pozicija + 1, -1); 
  //exit(-1);
  //cout << "________________________________" << endl;
  fstream outputFile;
  //outputFile.open(folder + "/" + novo_ime, ios_base::out | ios_base::trunc);
  if(pozicija != -1){
    outputFile.open(folder + "/" + novo_ime, ios_base::out | ios_base::trunc);
  }else{
    //cout << "evo me ovde" << endl;
    outputFile.open(novo_ime, ios_base::out | ios_base::trunc);
  }
  if(outputFile.is_open()){
    cout << "Uspesno napravljen izlazni fajl: " << output_file_name << endl;
    //ispisiTabeluSekcija(outputFile);
    ispisiTabeluSimbola(outputFile);
    ispisiKodSekcije(outputFile);
    ispisiRelokacioneZapise(outputFile);
    outputFile << endl;
    outputFile << endl;
    outputFile.close(); // ZATVARANJE IZLAZNOG FAJLA!!! PROVERI DA LI TREBA UOPSTE
  }else{
    cout << "Greska pri pravljenju izlaznog fajla: " << output_file_name << endl;
  }
}

void Assembler::generateLinkerInputFile(){
  fstream outputFileLinker;
  outputFileLinker.open(output_file_name, ios_base::out | ios_base::trunc);
  if(outputFileLinker.is_open()){
    //cout << "Uspesno napravljen izlazni fajl za linker." << endl;
    outputFileLinker << "OUTPUT FOR LINKER" << endl;
    outputFileLinker << endl;
    //outputFileLinker << endl << "-ZA LINKER TABLE SYMBOLS-" << endl;
    /*if(tableSymbols.size() > 0){
      outputFileLinker << "pocetakTabSimb" << endl;
    }*/
    for(auto & elemSimbol : tableSymbols){
      int stvVel = -1;
      if(elemSimbol.isSection){
        for (size_t i = 0; i < tableSections.size(); i++)
        {
          if(!tableSections[i].sectionName.compare(elemSimbol.sectionName)){
            stvVel = tableSections[i].sectionSize;
            break;
          }
        }
      }
      outputFileLinker << "TS-" << elemSimbol.symbolID << ":" << elemSimbol.symbolName << ":" << elemSimbol.symbolValue << ":" << elemSimbol.sectionName << ":";
      outputFileLinker << elemSimbol.globalSymbol << ":" << elemSimbol.definedSymbol << ":" << stvVel << ":" << elemSimbol.isSection << endl;
    }
    outputFileLinker << "_" << endl;
    //outputFileLinker << "krajTabSimb" << endl;
    //outputFileLinker << endl << "-ZA LINKER SECTIONS CODE-" << endl;
    for(auto & elem: tableSections){
      //outputFileLinker << "pocetakSek" << endl;
      outputFileLinker << endl;
      outputFileLinker << "section: " << elem.sectionID << ":" << elem.sectionName << ":" << elem.sectionSize << ":" << elem.beginningAddress << endl;
      int br = 0;
      if(elem.sectionCode.size() == 0){
        outputFileLinker << "Nema sadrzaj";
        //outputFileLinker << "krajSek";
      }
      for(int i = 0; i < elem.sectionCode.size(); i++){
        br++;
        if(i == 0){
          outputFileLinker << "0: ";
        }
        if((i % 2 == 0) && i != 0){
          outputFileLinker << elem.sectionCode[i];
        }else{
          outputFileLinker << elem.sectionCode[i];
        }
        if(br < 16 && br % 2 == 0){
          outputFileLinker << " ";
        }
        if(br == 16 && (i + 1) != elem.sectionCode.size()){
          br = 0;
          outputFileLinker << endl;
          int broj = i + 1;
          string hexBroj = pretvoriUHexOblik(broj / 2);
          outputFileLinker << hexBroj << ": ";
        }
      }
      if(elem.sectionCode.size() > 0){
        //outputFileLinker << endl << "krajSek";
      }
      //outputFileLinker << endl;
      outputFileLinker << endl;
    }
    outputFileLinker << "+" << endl;
    //outputFileLinker << "-ZA LINKER REALLOCATIONS-" << endl;
    outputFileLinker << endl;
    for (auto & elemSec: tableSections)
    {
      /*if(elemSec.tableOfRealocations.size() > 0){
        outputFileLinker << "ReallocSection: " << elemSec.sectionName << endl;
        //outputFileLinker << "pocetakRelokacija" << endl;
      }*/
      for(auto & elemRelloc: elemSec.tableOfRealocations){
        string locationHex = pretvoriUHexOblik(elemRelloc.location);
        outputFileLinker << "RS:" << elemSec.sectionName << "-" << elemRelloc.refSym << ":" << elemRelloc.location << ":0x" << locationHex << ":" << elemRelloc.type << ":" << elemRelloc.addend << ":" << elemRelloc.realAddend << endl;
      }
      /*if(elemSec.tableOfRealocations.size() > 0){
        outputFileLinker << "krajRelokacija" << endl << endl;
      }*/
    }
    outputFileLinker.close(); // ZATVARANJE IZLAZNOG FAJLA!!! PROVERI DA LI TREBA UOPSTE
  }else{
    cout << "Greska pri pravljenju izlaznog fajla za linker." << endl;
  }
}

//Stara verzija
int Assembler::makeAssemblyLines(){
  ifstream inputFile;
  inputFile.open(input_file_name);
  if(inputFile.is_open()){
    string currentLine;
    while(getline(inputFile, currentLine)){
      lineNo++;
      currentLine = line_removeBlankCharactersAtBeginning(currentLine);
      currentLine = line_removeComments(currentLine);
      currentLine = line_removeAdditionalSpacesOrTabulator(currentLine);
      currentLine = line_removePunctuationSpace(currentLine);
      if(!regex_match(currentLine, regex_empty_line)){
        int pos = currentLine.find_last_not_of(" ");
        //cout << "Pozicija: " << pos << endl;
        currentLine = currentLine.substr(0, pos + 1);
        modified_lines.push_back(currentLine);
      }      
    }
    inputFile.close(); // ZATVARANJE ULAZNOG FAJLA!!! PROVERI DA LI TREBA UOPSTE
    return 0;
  }else{
    cout << "*** Error with opening input file. ***" << endl;
    return -1;
  }
}

// Nova verzija
/*int Assembler::makeAssemblyLines(){
  ifstream inputFile;
  inputFile.open(input_file_name);
  if(inputFile.is_open()){
    string currentLine;
    while(getline(inputFile, currentLine)){
      lineNo++;
      currentLine = line_removeBlankCharactersAtBeginning(currentLine);
      currentLine = line_removeComments(currentLine);
      currentLine = line_removeAdditionalSpacesOrTabulator(currentLine);
      currentLine = line_removePunctuationSpace(currentLine);
      if(!regex_match(currentLine, regex_empty_line)){
        int pos = currentLine.find_last_not_of(" ");
        //cout << "Pozicija: " << pos << endl;
        currentLine = currentLine.substr(0, pos + 1);
        //modified_lines.push_back(currentLine);
        int ret = parseModifiedLines(currentLine);
        if(ret < 0){
          return -1;
        }
        if(ret == 3){
          break;
        }
      }      
    }
    if(Assembler::assembling_finished == 0){
      cout << "Na kraju se mora naci: .end direktiva!" << endl;
      exit(-1);
    }
    Assembler::backpatch();
    Assembler::isThereUndefinedSymbol();
    inputFile.close(); // ZATVARANJE ULAZNOG FAJLA!!! PROVERI DA LI TREBA UOPSTE
    return 0;
  }else{
    cout << "*** Error with opening input file. ***" << endl;
    return -1;
  }
}*/

// Stara verzija
int Assembler::parseModifiedLines(){
  for (size_t i = 0; i < modified_lines.size(); i++)
  {
    //cout << "Linija: " << modified_lines[i] << endl;
    bool label_found = checkForLabelInLine(modified_lines[i]);
    bool directive_found;
    bool instruction_found;
    if(label_found){
      //cout << "Bila je labela" << endl;
      continue;
    }
    if(label_found == false){
      directive_found = checkForDirectiveInLine(modified_lines[i]);
      if(directive_found){
        //cout << "Bila je direktiva" << endl;
        if(Assembler::assembling_finished){
          //cout << "Kraj: " << modified_lines[i] << endl;
          break; 
        }
        continue;
      }else{
        instruction_found = checkForInstructionInLine(modified_lines[i]);
        if(instruction_found == false){
          cout << "U liniji mora biti labela/direktiva/instrukcija." << endl;
          exit(-1);
        }
      }
    }
  }
  if(Assembler::assembling_finished == 0){
    cout << "Na kraju se mora naci: .end direktiva!" << endl;
    exit(-1);
  }
  Assembler::backpatch();
  Assembler::isThereUndefinedSymbol();
  return 0;
}

// Nova verzija
/*int Assembler::parseModifiedLines(string line){
  bool label_found = checkForLabelInLine(line);
  bool directive_found;
  bool instruction_found;
  if(label_found){
    //cout << "Bila je labela" << endl;
    //continue;
    return 2;
  }
  if(label_found == false){
    directive_found = checkForDirectiveInLine(line);
    if(directive_found){
      //cout << "Bila je direktiva" << endl;
      if(Assembler::assembling_finished){
        //cout << "Kraj: " << modified_lines[i] << endl;
        //break;
        return 3; 
      }
      //continue;
      return 2;
    }else{
      instruction_found = checkForInstructionInLine(line);
      if(instruction_found == false){
        cout << "U liniji mora biti labela/direktiva/instrukcija." << endl;
        exit(-1);
      }
    }
  }
  return 0;
}*/

// Stara Verzija
int Assembler::assembly(){
  int ret = makeAssemblyLines();
  if(ret < 0){
    return -1;
  }
  ret = parseModifiedLines();
  if(ret < 0){
    return -1;
  }
  generateOutputFile();
  generateLinkerInputFile();
  return 0;
}

//Nova verzija
/*int Assembler::assembly(){
  int ret = makeAssemblyLines();
  if(ret < 0){
    return -1;
  }
  generateOutputFile();
  generateLinkerInputFile();
  return 0;
}*/