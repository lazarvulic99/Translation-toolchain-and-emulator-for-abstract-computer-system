#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <math.h>

#include "../inc/linker.hpp"
//#include "/home/ss/Desktop/resenje/inc/linker.hpp"

Linker::Linker(string nazivIzlaza, vector<string> naziviUlaznihFajlova){
  //cout << "U konstruktoru sam!" << endl;
  imeIzlaza = nazivIzlaza;
  ulazniFajlovi = naziviUlaznihFajlova;
}

void Linker::zatvoriFajlove(){
  outputFile.close();
  /*for(auto & kod : kodSekcija){
    cout << kod.fajl << " " << kod.sectionName << " " << kod.offset << " - " << kod.code << endl;
  }*/
}

void tokenize(string const &str, const char delim, vector<string> &out)
{
    stringstream ss(str);
    string s;
    while (getline(ss, s, delim)) {
        out.push_back(s);
    }
}

int Linker::pretvoriIzHexUDec(string hex)
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

string Linker::simbolHexOblik(string hexVrednost){
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

string Linker::stringToUpper(string strToConvert)
{
    transform(strToConvert.begin(), strToConvert.end(), strToConvert.begin(), ::toupper);
    return strToConvert;
}

string Linker::pretvoriUHexOblik(int broj){
  stringstream hexOblik;
  hexOblik << hex << broj;
  return hexOblik.str();
}

int Linker::pomocnaMetodaZaNegativneBrojeve(string hexBroj){
  //cout << "Stari broj u hex obliku: " << hexBroj << endl;
  string broj = "";
  int praviBroj;
  if(hexBroj[0] > '8'){
    broj = "FFFF" + hexBroj;
    praviBroj = strtoul(broj.c_str(), nullptr, 16);
  }else{
    praviBroj = pretvoriIzHexUDec(hexBroj);
  }
  //cout << "Pravi broj: " << praviBroj << endl;
  return praviBroj;
}

void Linker::dodajSimbol(string imeFajla, string symbol_id, string name, string value, string sectionName, string globalYesNo, string definedYesNo, string realSize, string isSectionYesNo){
  //cout << "Dodajem simbol" << endl;
  Entry_Symbol newSymbol;
  newSymbol.fajl = imeFajla;
  int idSimbola = stoi(symbol_id, nullptr, 0);
  newSymbol.symId = idSimbola;
  newSymbol.symName = name;
  newSymbol.symValue = value;
  newSymbol.sectionName = sectionName;
  newSymbol.symDefined = definedYesNo;
  newSymbol.symGlobal = globalYesNo;
  int velicina = stoi(realSize, nullptr, 0);
  int sekcijaDaNe = stoi(isSectionYesNo, nullptr, 0);
  newSymbol.isSection = sekcijaDaNe;
  newSymbol.symSize = velicina;
  tableSymbols.push_back(newSymbol);
}

void Linker::dodajRelokaciju(string imeFajla, string sekcija_i_refSym, string location, string locationHex, string type, string addend, string realAddend){
  //cout << "Dodajem relokaciju" << endl;
  Entry_Reallocation newRealloc;
  newRealloc.fajl = imeFajla;
  vector<string> tokeniArgument;
  const char delimiter = '-';
  tokenize(sekcija_i_refSym, delimiter, tokeniArgument);
  newRealloc.sekcija = tokeniArgument[0];
  int refSymId = stoi(tokeniArgument[1], nullptr, 0);
  newRealloc.refSym = refSymId;
  int lokacija = stoi(location, nullptr, 0);
  newRealloc.location = lokacija;
  newRealloc.locationHex = locationHex;
  int tip = stoi(type, nullptr, 0);
  newRealloc.type = tip;
  int adend = stoi(addend, nullptr, 0);
  int praviAdend = stoi(realAddend, nullptr, 0);
  newRealloc.addend = adend;
  newRealloc.realAddend = praviAdend;
  tableReallocations.push_back(newRealloc);
}

int Linker::prolazakKrozUlazneFajlove(){
  //cout << "Prolazim kroz ulazne fajlove!" << endl;
  for(int i = 0; i < ulazniFajlovi.size(); i++){
    ifstream ulazniFajl;
    ulazniFajl.open(ulazniFajlovi[i]);
    if(!ulazniFajl.is_open()){
      cout << "Greska prilikom otvaranja ulaznog fajla: " << ulazniFajlovi[i] << endl;
      return -1;
    }else{
      //cout << "Uspesno otvorio fajl: " << ulazniFajlovi[i] << endl;
      string currentLine;
      while(getline(ulazniFajl, currentLine)){
        if(!currentLine.compare("OUTPUT FOR LINKER")){
          //cout << "OUTPUT FOR LINKER!" << endl;
          continue;
        }else if(!currentLine.compare("")){
          //cout << "EMPTY LINE!" << endl;
          continue;
        }else if(!currentLine.compare("_")){
          break;
        }else{
          string ulazLinije = currentLine.substr(0, 4);
          if(!ulazLinije.compare("TS-0")){
            continue;
          }
          //cout << "Evo me: " << currentLine << endl;
          vector<string> tokeni;
          const char delimiter = ':';
          tokenize(currentLine, delimiter, tokeni);
          string podatak0 = tokeni[0];
          string symbol_id = podatak0.substr(3, -1);
          string name = tokeni[1];
          string value = tokeni[2];
          string sectionName = tokeni[3];
          string globalYesNo = tokeni[4];
          string definedYesNo = tokeni[5];
          string realSize = tokeni[6];
          string isSectionYesNo = tokeni[7];
          if(!globalYesNo.compare("0") && !isSectionYesNo.compare("0")){
            //cout << "Simbol je lokalan(a nije sekcija), ne dodaje se u tabelu simbola!" << endl;
          }else{
            dodajSimbol(ulazniFajlovi[i], symbol_id, name, value, sectionName, globalYesNo, definedYesNo, realSize, isSectionYesNo);
          }
        }
      }
      for(int j = 0; j < tableSymbols.size(); j++){
        if(!tableSymbols[j].isSection){
          for(int k = 0; k < tableSymbols.size(); k++){
            if(tableSymbols[j].fajl.compare(tableSymbols[k].fajl)){
              if(!tableSymbols[j].symName.compare(tableSymbols[k].symName)){
                if((tableSymbols[j].sectionName.compare("UNDEFINED") && tableSymbols[j].sectionName.compare("NEPOZNATO") && tableSymbols[j].sectionName.compare("UNKNOWN")) && (tableSymbols[k].sectionName.compare("UNDEFINED") && tableSymbols[k].sectionName.compare("NEPOZNATO") && tableSymbols[k].sectionName.compare("UNKNOWN"))){
                  cout << "GRESKA! Visestruka definicija simbola: " + tableSymbols[j].symName << endl;
                  return -1;
                }
              }
            }
          }
        }
      }
      //cout << "PROSAO PROVERU ZA VISESTRUKE SIMBOLE" << endl;
      string imeSekcije;
      while(getline(ulazniFajl, currentLine)){
        if(!currentLine.compare("")){
          //cout << "PRAZNA LINIJA!!!" << endl;
          continue;
        }else if(!currentLine.compare("Nema sadrzaj")){
          //cout << "BEZ SADRZAJA!!!" << endl;
          continue;
        }else if(!currentLine.compare("+")){
          //cout << "KRAJ ZA SEKCIJE!!!" << endl;
          break;
        }else{
          //cout << currentLine << endl;
          string uvodLinije = currentLine.substr(0, 9);
          if(!uvodLinije.compare("section: ")){
            //cout << "Uvod u sekciju" << endl;
            string ostatakLinije = currentLine.substr(9, -1);
            //cout << "Ostatak linije: " << ostatakLinije << endl;
            int pos = ostatakLinije.find_first_of(":");
            imeSekcije = ostatakLinije.substr(pos + 1, -1);
            //cout << "Ime sekcije: " << imeSekcije << endl;
            int novaPoz = imeSekcije.find_first_of(":");
            imeSekcije = imeSekcije.substr(0, novaPoz);
            //cout << imeSekcije << endl;
          }else{
            //kod sekcije
            //cout << imeSekcije << endl;
            //cout << currentLine << endl;
            int positionOfDveTacke = currentLine.find_first_of(":");
            string offset = currentLine.substr(0, positionOfDveTacke);
            string code = currentLine.substr(positionOfDveTacke + 2, -1);
            string modifiedCode = "";
            for(int m = 0; m < code.size(); m++){
              if(code.at(m) != ' '){
                modifiedCode += code.at(m);
              }
            }
            SectionCode kodSekcije;
            kodSekcije.code = modifiedCode;
            kodSekcije.fajl = ulazniFajlovi[i];
            kodSekcije.offset = offset;
            kodSekcije.sectionName = imeSekcije;
            kodSekcija.push_back(kodSekcije);
          }
        }
      }

      /*for(int l = 0; l < kodSekcija.size(); l++){
        cout << kodSekcija[l].fajl << " " << kodSekcija[l].sectionName << " " << kodSekcija[l].offset << " " << kodSekcija[l].code << endl;
      }*/

      //cout << "DODAO CODE SEKCIJE" << endl;
      while(getline(ulazniFajl, currentLine)){
        //cout << currentLine << endl;
        if(!currentLine.compare("")){
          continue;
        }else{
          string deo = currentLine.substr(0, 3);
          if(!deo.compare("RS:")){
            //cout << "Nasao rekolaciju" << endl;
            vector<string> tokeniRekolacije;
            const char delimiter = ':';
            tokenize(currentLine, delimiter, tokeniRekolacije);
            string podatak0 = tokeniRekolacije[0];
            string sekcija_i_refSym = tokeniRekolacije[1];
            string location = tokeniRekolacije[2];
            string locationHex = tokeniRekolacije[3];
            string type = tokeniRekolacije[4];
            string addend = tokeniRekolacije[5];
            string realAddend = tokeniRekolacije[6];
            dodajRelokaciju(ulazniFajlovi[i], sekcija_i_refSym, location, locationHex, type, addend, realAddend);
          }else{
            continue;
          }
        }
      }
      /*cout << "TABELA RELOKACIJA" << endl;
      for(int i = 0; i < tableReallocations.size(); i++){
        cout << tableReallocations[i].fajl << ":" << tableReallocations[i].sekcija << ":" << tableReallocations[i].refSym << ":";
        cout << tableReallocations[i].location << ":" << tableReallocations[i].locationHex << ":" << tableReallocations[i].type << ":";
        cout << tableReallocations[i].addend << ":" << tableReallocations[i].realAddend << endl;
      }*/
    }
    ulazniFajl.close();
  }
  return 0;
}

void Linker::potraziSekcijuIstogImenaIzDrugihFajlova(string sectionName, int startingAddress, string imeFajla){
  //cout << "Pretrazujem za ostale" << endl;
  for(auto & elem : tableSymbols){
    if(!elem.symName.compare(sectionName) && elem.fajl.compare(imeFajla)){
      Entry_Section novaSek;
      novaSek.fajl = elem.fajl;
      novaSek.name = elem.symName;
      //novaSek.id = elem.symId;
      novaSek.beginningAddress = startingAddress;
      novaSek.size = elem.symSize;
      tableSections.push_back(novaSek);
    }
  }
}

int Linker::dodajSekcijeNaOsnovuSimbola(){
  //cout << endl << "Dodajem sekcije na osnovu simbola." << endl;
  int retVal = 0;
  for(auto & elemSim : tableSymbols){
    if(elemSim.isSection){
      //cout << "Jeste sekcija: " << elemSim.symName << endl;
      int nasaoUbacenu = 0;
      for(auto & elemSekc : tableSections){
        if(!elemSekc.name.compare(elemSim.symName)){
          //cout << "Sekcija je vec jednom ubacena." << endl;
          nasaoUbacenu = 1;
          break;
        }
      }
      if(nasaoUbacenu){
        //cout << "Vec ubacena" << endl;
      }else{
        //cout << "Nova" << endl;
        Entry_Section novaSekcija;
        novaSekcija.fajl = elemSim.fajl;
        novaSekcija.name = elemSim.symName;
        novaSekcija.beginningAddress = 0;
        //novaSekcija.id = elemSim.symId;
        novaSekcija.size = elemSim.symSize;
        tableSections.push_back(novaSekcija);
        potraziSekcijuIstogImenaIzDrugihFajlova(elemSim.symName, novaSekcija.beginningAddress + novaSekcija.size, elemSim.fajl);
      }
    }
  }
  /*for(auto & eSek : tableSections){
    cout << eSek.fajl << " " << eSek.name << " " << eSek.beginningAddress << " " << eSek.size << endl;
  }*/
  return retVal;
}

int Linker::azurirajPocetneAdreseSekcija(){
  //cout << endl;
  //cout << "Azuriram pocetne adrese sekcija" << endl;
  int disp = 0;
  for(auto & sekc : tableSections){
    sekc.beginningAddress = disp;
    disp += sekc.size;
  }
  /*for(auto & eSek : tableSections){
    cout << eSek.fajl << " " << eSek.name << " " << eSek.beginningAddress << " " << eSek.size << endl;
  }*/
  return 0;
}

int Linker::obradiSekcije(){
  int retVal = dodajSekcijeNaOsnovuSimbola();
  if(retVal < 0){
    exit(-1);
  }
  retVal = azurirajPocetneAdreseSekcija();
  if(retVal < 0){
    exit(-1);
  }
  return 0;
}

int Linker::prepraviTabeluSimbola(){ // Ovo proveri dodatno!!!
  //cout << endl << "Prepravljam tabelu simbola." << endl;
  /*for(auto & simb : tableSymbols){
    cout << simb.fajl << " " << simb.symId << " " << simb.symName << " " << simb.symValue << " ";
    cout << simb.symSize << " " << simb.symGlobal << " " << simb.symDefined << " " << simb.sectionName << " " << simb.isSection;
    cout << endl;
  }*/
  for(auto & elemSimbol : tableSymbols){
    if(elemSimbol.isSection){
      //cout << "simbol je sekcija" << endl;
      for(auto & elemSekcija : tableSections){
        if(!elemSimbol.fajl.compare(elemSekcija.fajl) && !elemSimbol.symName.compare(elemSekcija.name)){
          int disp = elemSekcija.beginningAddress;
          elemSimbol.symValue = to_string(disp);
          break;
        }
      }
    }else{
      if(!(!elemSimbol.symGlobal.compare("1") && !elemSimbol.symDefined.compare("0"))){
        //cout << "Nije eksterni simbol" << endl;
        if(elemSimbol.sectionName.compare("UNDEFINED") && elemSimbol.sectionName.compare("NEPOZNATO") && elemSimbol.sectionName.compare("UNKNOWN")){
          //cout << "Tu sam" << endl;
          string simbolSekcija;
          for(auto & elemSimbol1 : tableSymbols){
            if(!elemSimbol1.fajl.compare(elemSimbol.fajl) && elemSimbol1.symId == elemSimbol.symId){
              //cout << "Evo tu" << endl;
              simbolSekcija = elemSimbol1.sectionName;
              //cout << simbolSekcija << endl;
              break;
            }
          }
          for(auto & sekc : tableSections){
            if(!sekc.fajl.compare(elemSimbol.fajl) && !sekc.name.compare(simbolSekcija)){
              //cout << "Majke mi sam ovde" << endl;
              //cout << stoi(elemSimbol.symValue, nullptr, 0) << "   " << sekc.beginningAddress << endl; 
              int disp = stoi(elemSimbol.symValue, nullptr, 0) + sekc.beginningAddress;
              //cout << disp << endl;
              elemSimbol.symValue = to_string(disp);
            }
          }
        }
      }
    }
  }
  //cout << endl;
  /*for(auto & simb : tableSymbols){
    cout << simb.fajl << " " << simb.symId << " " << simb.symName << " " << simb.symValue << " ";
    cout << simb.symSize << " " << simb.symGlobal << " " << simb.symDefined << " " << simb.sectionName << " " << simb.isSection;
    cout << endl;
  }*/
  return 0;
}

int Linker::fjaZaPretvaranjeIzHexUDec(string s){
  //cout << "Dosao do fje za pretvaranje: " << s << endl;
  string noviStr = s;
  noviStr = stringToUpper(noviStr);
  int broj = stoul(noviStr, nullptr, 16);
  //cout << "OPA: " << broj << endl;
  //cout << "HEHE: " << noviStr << endl;
  return broj;
}

/*int Linker::prepraviRelokacije(){
  string kodZaIspis = "";
  for(auto & eSek : tableSections){
    cout << eSek.fajl << " " << eSek.name << " " << eSek.beginningAddress << " " << eSek.size << endl;
    for(auto & kodSek : kodSekcija){
      if(!eSek.fajl.compare(kodSek.fajl) && !eSek.name.compare(kodSek.sectionName)){
        kodZaIspis += kodSek.code;
      }
    }
  }
  for(int i = 0; i < kodZaIspis.size(); i++){
    if(i != 0 && i % 16 == 0){
      cout << endl;
    }
    cout << kodZaIspis[i];
  }
  for(auto & elemRel : tableReallocations){
    bool nasaoSimbol = false;
    string vrednostSimbolaString;
    int vrednostSimbola;
    string sekcijaUkojojJeRelokacija = elemRel.sekcija;
    string imeSekcije = "";
    string simbol;
    for(auto & elemSimb : tableSymbols){
      if(elemRel.refSym == elemSimb.symId){
        if(!elemRel.fajl.compare(elemSimb.fajl)){
          cout << "Tu sam za relok" << endl;
          simbol = elemSimb.symName;
          if(!elemSimb.sectionName.compare("UNDEFINED") || !elemSimb.sectionName.compare("NEPOZNATO") || !elemSimb.sectionName.compare("UNKNOWN")){
            for(auto & elemSimb2 : tableSymbols){
              // ako se pronadje simbol istog imena, iz razl. fajla, kome sekcija nije UNDEFINED, tj. definisan je
              // onda smo za eksterni simbol koji smo uvezli, stvarno i nasli njegovu definiciju pa se nece bacati greska
              // Naravno, mora da bude definicija u drugom fajlu, jer ne moze u istom fajlu simbol i da se uvozi, i da se
              // definise kao novi simbol. To bi bila visestruka definicija simbola.
              if(elemSimb2.fajl.compare(elemSimb.fajl) && !elemSimb2.symName.compare(elemSimb.symName)){
                if(elemSimb2.sectionName.compare("UNDEFINED") && elemSimb2.sectionName.compare("NEPOZNATO") && elemSimb2.sectionName.compare("UNKNOWN")){
                  cout << "Nasao" << endl;
                  nasaoSimbol = true;
                  vrednostSimbolaString =  elemSimb2.symValue;
                }
              }
            }
          }else{
            if(!elemSimb.symDefined.compare("1")){
              cout << "Tu 1" << endl;
              nasaoSimbol = true;
              vrednostSimbolaString = elemSimb.symValue;
            }
          }
        }
      }
      //cout << "Sekcija u kojoj je relokacija: " << sekcijaUkojojJeRelokacija << endl;
      if(!elemSimb.sectionName.compare(sekcijaUkojojJeRelokacija)){
        //cout << "Evo me ovde brate" << endl;
        if(!elemSimb.fajl.compare(elemRel.fajl)){
          //cout << "A evo me i ovde sam usao" << endl;
          imeSekcije = elemSimb.sectionName;
        }
      }
    }
    if(nasaoSimbol){
      cout << "Nasao ga:" << elemRel.fajl << " " << elemRel.sekcija << " " << elemRel.location << endl;
      for(auto & elemSec: tableSections){
        if(!elemSec.fajl.compare(elemRel.fajl) && !elemSec.name.compare(elemRel.sekcija)){
          cout << "Nasao sekciju!" << endl;
          int pozicijaZaPrepravku = elemRel.location + elemSec.beginningAddress;
          cout << "Pozicija za prepravku: " + pozicijaZaPrepravku << endl;
        }
      }
    }else{
      cout << "***" << endl;
      cout << "Nije pronadjena definicija simbola: " << simbol << " ni u jednom ulaznom fajlu!" << endl;
      cout << "***" << endl;
      return -1;
    }
  }
  return 0;
}*/

int Linker::prepraviRelokacije(){
  string kodZaIspis = "";
  for(auto & eSek : tableSections){
    //cout << eSek.fajl << " " << eSek.name << " " << eSek.beginningAddress << " " << eSek.size << endl;
    for(auto & kodSek : kodSekcija){
      if(!eSek.fajl.compare(kodSek.fajl) && !eSek.name.compare(kodSek.sectionName)){
        kodZaIspis += kodSek.code;
      }
    }
  }
  /*for(int i = 0; i < kodZaIspis.size(); i++){
    if(i != 0 && i % 16 == 0){
      cout << endl;
    }
    cout << kodZaIspis[i];
  }*/
  for(auto & elemRel : tableReallocations){
    bool nasaoSimbol = false;
    string vrednostSimbolaString;
    int vrednostSimbola;
    string sekcijaUkojojJeRelokacija = elemRel.sekcija;
    string imeSekcije = "";
    string simbol;
    for(auto & elemSimb : tableSymbols){
      if(elemRel.refSym == elemSimb.symId){
        if(!elemRel.fajl.compare(elemSimb.fajl)){
          //cout << endl << "Tu sam za relok" << endl;
          simbol = elemSimb.symName;
          if(!elemSimb.sectionName.compare("UNDEFINED") || !elemSimb.sectionName.compare("NEPOZNATO") || !elemSimb.sectionName.compare("UNKNOWN")){
            for(auto & elemSimb2 : tableSymbols){
              // ako se pronadje simbol istog imena, iz razl. fajla, kome sekcija nije UNDEFINED, tj. definisan je
              // onda smo za eksterni simbol koji smo uvezli, stvarno i nasli njegovu definiciju pa se nece bacati greska
              // Naravno, mora da bude definicija u drugom fajlu, jer ne moze u istom fajlu simbol i da se uvozi, i da se
              // definise kao novi simbol. To bi bila visestruka definicija simbola.
              if(elemSimb2.fajl.compare(elemSimb.fajl) && !elemSimb2.symName.compare(elemSimb.symName)){
                if(elemSimb2.sectionName.compare("UNDEFINED") && elemSimb2.sectionName.compare("NEPOZNATO") && elemSimb2.sectionName.compare("UNKNOWN")){
                  //cout << "Nasao" << endl;
                  nasaoSimbol = true;
                  vrednostSimbolaString =  elemSimb2.symValue;
                }
              }
            }
          }else{
            if(!elemSimb.symDefined.compare("1")){
              //cout << "Tu 1" << endl;
              nasaoSimbol = true;
              vrednostSimbolaString = elemSimb.symValue;
            }
          }
        }
      }
      //cout << "Sekcija u kojoj je relokacija: " << sekcijaUkojojJeRelokacija << endl;
      if(!elemSimb.sectionName.compare(sekcijaUkojojJeRelokacija)){
        //cout << "Evo me ovde brate" << endl;
        if(!elemSimb.fajl.compare(elemRel.fajl)){
          //cout << "A evo me i ovde sam usao" << endl;
          imeSekcije = elemSimb.sectionName;
        }
      }
    }
    if(nasaoSimbol){
      //cout << "Nasao ga:" << elemRel.fajl << " " << elemRel.sekcija << " " << elemRel.location << endl;
      int pozicijaRelokacije = elemRel.location;
      int pozicijaSekcije;
      for(auto & elemSekc : tableSections){
        if(!elemRel.fajl.compare(elemSekc.fajl) && !elemRel.sekcija.compare(elemSekc.name)){
          //cout << "Pozicija sekcije: " << elemSekc.beginningAddress << endl;
          pozicijaSekcije = elemSekc.beginningAddress;
          break;
        }
      }
      int mestoZaPrepravku = pozicijaRelokacije + pozicijaSekcije;
      //cout << "Mesto za prepravku: " << mestoZaPrepravku << endl;
      string deoPrePrepravke = kodZaIspis.substr(0, mestoZaPrepravku*2);
      string deoZaPrepravku = kodZaIspis.substr(mestoZaPrepravku*2, 4);
      string deoPoslePrepravke = kodZaIspis.substr(mestoZaPrepravku*2 + 4, -1);
      //cout << "Deo pre: " << deoPrePrepravke << endl;
      //cout << "Deo za prepravku: " << deoZaPrepravku << endl;
      //cout << "Deo posle prepravke: " << deoPoslePrepravke << endl;
      if(elemRel.type == 0){
        //cout << "Prepravka WORD" << endl;
        deoZaPrepravku = deoZaPrepravku.substr(2, 2) + deoZaPrepravku.substr(0, 2); // ovo zakomentarisi, odkomentarisi liniju
        // 580 i linije 648-711
        int staraVrednost = pretvoriIzHexUDec(deoZaPrepravku);
        //cout << staraVrednost << endl;
        //deoZaPrepravku = deoZaPrepravku.substr(2, 2) + deoZaPrepravku.substr(0, 2);
        //cout << "Dodaje se: " << stoi(vrednostSimbolaString, nullptr, 0) << endl;
        int novaVrednost = staraVrednost + stoi(vrednostSimbolaString, nullptr, 0);
        string novaVrednostString = pretvoriUHexOblik(novaVrednost);
        novaVrednostString = simbolHexOblik(novaVrednostString);
        novaVrednostString = stringToUpper(novaVrednostString);
        novaVrednostString = novaVrednostString.substr(2, 2) + novaVrednostString.substr(0, 2);
        //cout << "Nova: " << novaVrednostString << endl;
        string noviKod = deoPrePrepravke + novaVrednostString + deoPoslePrepravke;
        //cout << "Novi kod: " << noviKod << endl;
        kodZaIspis = noviKod;
      }else if(elemRel.type == 1){
        //cout << "Prepravka PCREL" << endl;
        int valueToWrite = 0;
        for(auto & elemSimbNovi : tableSymbols){
          if(!elemRel.fajl.compare(elemSimbNovi.fajl)){
            if(!elemSimbNovi.symName.compare(imeSekcije)){
              //cout << "Evo me usao za PC REL PREPRAVKU!" << endl;
              string valSimbola = elemSimbNovi.symValue; // a bukvalno mislim na adresu sekcije u kojoj ce da se nadje pc rel adresiranje
              // pa cu onda da dodam jos i offset do tog mesta koriscenja pc rel adresiranja
              int vrednostSimbola = stoi(valSimbola, nullptr, 0);
              int pomerajDoRelokacije = elemRel.location;
              valueToWrite = vrednostSimbola + pomerajDoRelokacije;
              //cout << vrednostSimbola << endl;
              //cout << valueToWrite << endl;
              break;
            }
          }
        }
        int staraVrednost = pomocnaMetodaZaNegativneBrojeve(deoZaPrepravku);
        int dodaj = stoi(vrednostSimbolaString, nullptr, 0) - valueToWrite;
        int upisiVrednost = staraVrednost + dodaj;
        string vrednostZaUpisString = pretvoriUHexOblik(upisiVrednost);
        vrednostZaUpisString = simbolHexOblik(vrednostZaUpisString);
        vrednostZaUpisString = stringToUpper(vrednostZaUpisString);
        if(vrednostZaUpisString.size() == 8){
          vrednostZaUpisString = vrednostZaUpisString.substr(vrednostZaUpisString.size() - 4, 4);
        }
        //cout << "Vrednost za upis: " << upisiVrednost << endl;
        //cout << vrednostZaUpisString << endl;
        string noviKod = deoPrePrepravke + vrednostZaUpisString + deoPoslePrepravke;
        //cout << "Nov izgled linije: " + noviKod << endl;
        kodZaIspis = noviKod;
      }else if(elemRel.type == 2){
        //cout << "Prepravka OSTALO" << endl;
        int staraVrednost = pretvoriIzHexUDec(deoZaPrepravku);
        //cout << staraVrednost << endl;
        //cout << "Dodaje se: " << stoi(vrednostSimbolaString, nullptr, 0) << endl;
        int novaVrednost = staraVrednost + stoi(vrednostSimbolaString, nullptr, 0);
        string novaVrednostString = pretvoriUHexOblik(novaVrednost);
        novaVrednostString = simbolHexOblik(novaVrednostString);
        novaVrednostString = stringToUpper(novaVrednostString);
        //cout << "Nova: " << novaVrednostString << endl;
        string noviKod = deoPrePrepravke + novaVrednostString + deoPoslePrepravke;
        //cout << "Nov izgled linije: " + noviKod << endl;
        kodZaIspis = noviKod;
      }
    }else{
      cout << "***" << endl;
      cout << "Nije pronadjena definicija simbola: " << simbol << " ni u jednom ulaznom fajlu!" << endl;
      cout << "***" << endl;
      return -1;
    }
  }
  /*for(auto & elmSim : tableSymbols){
    cout << elmSim.symName << " " << elmSim.symValue << endl;
  }*/

  /*for(auto & elemRel: tableReallocations){
    string deoPre = "";
    string deoZa = "";
    string deoPosle = "";
    if(elemRel.type == 0){
      for(auto & elemSekc: tableSections){
        if(!elemRel.fajl.compare(elemSekc.fajl) && !elemRel.sekcija.compare(elemSekc.name)){
          int pozicijaPrepravke = elemRel.location*2 + elemSekc.beginningAddress*2;
          //cout << "---" << endl;
          deoPre = kodZaIspis.substr(0, pozicijaPrepravke);
          deoZa = kodZaIspis.substr(pozicijaPrepravke, 4);
          deoPosle = kodZaIspis.substr(pozicijaPrepravke + 4, -1);
          string vrednost = deoZa.substr(2, 2) + deoZa.substr(0, 2);
          //cout << "Vr: " << vrednost << endl;
          int vrednostInt = pretvoriIzHexUDec(vrednost);
          string vrednostIntStr = to_string(vrednostInt);
          //cout << "Vr: int " << vrednostInt << endl;
          bool postoji = false;
          for(auto & elemSimbl: tableSymbols){
            if(!elemSimbl.symValue.compare(vrednostIntStr)){
              //cout << "Pronasao ga" << endl;
              postoji = true;
              break;
            }
          }
          if(!postoji){
            cout << "Deo za prepravku: " << deoZa << ", pozicija: " << pozicijaPrepravke << endl;
            string deo1 = deoZa.substr(0, 2);
            string deo2 = deoZa.substr(2, 2);
            if(!deo1.compare("00")){
              deoZa = deo2 + deo1;
              kodZaIspis = deoPre + deoZa + deoPosle;
            }else if(!deo2.compare("00")){
              deoZa = deo1 + deo2;
              kodZaIspis = deoPre + deoZa + deoPosle;
            }else{
              int broj1 = pretvoriIzHexUDec(deo1);
              int broj2 = pretvoriIzHexUDec(deo2);
              int zbir = broj1 + broj2;
              string noviDeo = pretvoriUHexOblik(zbir);
              noviDeo = stringToUpper(noviDeo);
              if(noviDeo.size() == 2){
                noviDeo += "00";
              }else if(noviDeo.size() == 1){
                noviDeo += "000";
              }else if(noviDeo.size() > 4){
                noviDeo = noviDeo.substr(noviDeo.size() - 4, 4);
              }else if(noviDeo.size() == 3){
                noviDeo = "0" + noviDeo;
                noviDeo = noviDeo.substr(2, 2) + noviDeo.substr(0, 2);
              }else if(noviDeo.size() == 4){
                noviDeo = noviDeo.substr(2, 2) + noviDeo.substr(0, 2);
              }
              cout << broj1 << endl;
              cout << broj2 << endl;
              cout << zbir << endl;
              cout << noviDeo << endl;
              kodZaIspis = deoPre + noviDeo + deoPosle;
            }
          }
        }
      }
    }
  }*/
  //cout << kodZaIspis << endl;
  kodKonacni = kodZaIspis;
  return 0;
}

/*
//Ovako mi brljalo nesto, ne znam sto
int Linker::prepraviRelokacije(){
  for(auto & elemRel : tableReallocations){
    bool nasaoSimbol = false;
    string vrednostSimbolaString;
    int vrednostSimbola;
    string sekcijaUkojojJeRelokacija = elemRel.sekcija;
    string imeSekcije = "";
    string simbol;
    for(auto & elemSimb : tableSymbols){
      if(elemRel.refSym == elemSimb.symId){
        if(!elemRel.fajl.compare(elemSimb.fajl)){
          cout << "Tu sam za relok" << endl;
          simbol = elemSimb.symName;
          if(!elemSimb.sectionName.compare("UNDEFINED") || !elemSimb.sectionName.compare("NEPOZNATO") || !elemSimb.sectionName.compare("UNKNOWN")){
            for(auto & elemSimb2 : tableSymbols){
              // ako se pronadje simbol istog imena, iz razl. fajla, kome sekcija nije UNDEFINED, tj. definisan je
              // onda smo za eksterni simbol koji smo uvezli, stvarno i nasli njegovu definiciju pa se nece bacati greska
              // Naravno, mora da bude definicija u drugom fajlu, jer ne moze u istom fajlu simbol i da se uvozi, i da se
              // definise kao novi simbol. To bi bila visestruka definicija simbola.
              if(elemSimb2.fajl.compare(elemSimb.fajl) && !elemSimb2.symName.compare(elemSimb.symName)){
                if(elemSimb2.sectionName.compare("UNDEFINED") && elemSimb2.sectionName.compare("NEPOZNATO") && elemSimb2.sectionName.compare("UNKNOWN")){
                  cout << "Nasao" << endl;
                  nasaoSimbol = true;
                  vrednostSimbolaString =  elemSimb2.symValue;
                }
              }
            }
          }else{
            if(!elemSimb.symDefined.compare("1")){
              cout << "Tu 1" << endl;
              nasaoSimbol = true;
              vrednostSimbolaString = elemSimb.symValue;
            }
          }
        }
      }
      //cout << "Sekcija u kojoj je relokacija: " << sekcijaUkojojJeRelokacija << endl;
      if(!elemSimb.sectionName.compare(sekcijaUkojojJeRelokacija)){
        //cout << "Evo me ovde brate" << endl;
        if(!elemSimb.fajl.compare(elemRel.fajl)){
          //cout << "A evo me i ovde sam usao" << endl;
          imeSekcije = elemSimb.sectionName;
        }
      }
    }
    if(nasaoSimbol){
      cout << "Nasao ga:" << elemRel.fajl << " " << elemRel.sekcija << " " << elemRel.location << endl;
      for(auto & kodSekc : kodSekcija){
        if(!elemRel.fajl.compare(kodSekc.fajl)){
          if(!elemRel.sekcija.compare(kodSekc.sectionName)){
            //cout << "Evo mene ovde!" << endl;
            string pomeraj = kodSekc.offset;
            cout << "POMERAJ JE: " << pomeraj << endl;
            int pomerajDec = fjaZaPretvaranjeIzHexUDec(pomeraj);
            //int pomerajDec = pretvoriIzHexUDec(pomeraj);
            cout << "Pomeraj je u dec: " << pomerajDec << endl;
            int pomerajRelokacije = elemRel.location;
            cout << "Pom relok: " << pomerajRelokacije << endl;
            if((pomerajRelokacije < pomerajDec + 8) && (pomerajRelokacije >= pomerajDec)){
              cout << "Pronasao deo koda koji treba da se promeni: " << kodSekc.code << endl;
              if(elemRel.type == 0){
                cout << "Prepravljam word direktivu!" << endl;
                cout << "Velicina linije " << kodSekc.code.size() << endl;
                string deoPre = kodSekc.code.substr(0, (pomerajRelokacije - pomerajDec)*2);
                cout << "Deo pre popravke:" << deoPre << endl;
                string deoZaPrepravku = kodSekc.code.substr((pomerajRelokacije - pomerajDec)*2, 4);
                deoZaPrepravku = deoZaPrepravku.substr(2, 2) + deoZaPrepravku.substr(0, 2);
                cout << "Deo za prepravku:" << deoZaPrepravku << endl;
                if(deoZaPrepravku.size() == 2){
                  cout << "Mora i naredna linija za WORD" << endl;
                  string imeFajla = kodSekc.fajl;
                  string nameSect = kodSekc.sectionName;
                  int pom = pomerajDec + 8;
                  string pomString = pretvoriUHexOblik(pom);
                  cout << "POMERAJ STRING OVDE: " << pomString << endl;
                  string drugaLinijaKoda = "";
                  for(auto & kodSekc1 : kodSekcija){
                    if(!kodSekc1.fajl.compare(imeFajla) && !kodSekc1.sectionName.compare(nameSect) && !kodSekc1.offset.compare(pomString)){
                      cout << "EVO USAO ODJEEEE" << endl;
                      drugaLinijaKoda = kodSekc1.code;
                      break;
                    }
                  }
                  deoZaPrepravku += drugaLinijaKoda.substr(0, 2);
                  cout << "Deo za prepravku:" << deoZaPrepravku << endl;
                  string deoPoslePrepravke = drugaLinijaKoda.substr(2, -1);
                  cout << "Deo posle prepravke:" << deoPoslePrepravke << endl;
                  int staraVrednost = pretvoriIzHexUDec(deoZaPrepravku);
                  //cout << staraVrednost << endl;
                  //cout << "Dodaje se: " << stoi(vrednostSimbolaString, nullptr, 0) << endl;
                  int novaVrednost = staraVrednost + stoi(vrednostSimbolaString, nullptr, 0);
                  string novaVrednostString = pretvoriUHexOblik(novaVrednost);
                  novaVrednostString = simbolHexOblik(novaVrednostString);
                  novaVrednostString = stringToUpper(novaVrednostString);
                  novaVrednostString = novaVrednostString.substr(2, 2) + novaVrednostString.substr(0, 2);
                  cout << "Nova: " << novaVrednostString << endl;
                  string deo1 = novaVrednostString.substr(0, 2);
                  string deo2 = novaVrednostString.substr(2, 2);
                  string kod1 = "";
                  string kod2 = "";
                  kod1 += deoPre + deo1;
                  kod2 += deo2 + deoPoslePrepravke;
                  kodSekc.code = kod1;
                  for(auto & kodSekc1 : kodSekcija){
                    if(!kodSekc1.fajl.compare(imeFajla) && !kodSekc1.sectionName.compare(nameSect) && !kodSekc1.offset.compare(pomString)){
                      cout << "EVO USAO ODJEEE da menjam" << endl;
                      kodSekc1.code = kod2;
                      break;
                    }
                  }
                  //exit(-1);
                }else{
                  cout << "WORD u jednom redu!" << endl;
                  string deoPoslePrepravke = kodSekc.code.substr((pomerajRelokacije - pomerajDec)*2 + 4, -1);
                  cout << "Deo posle prepravke:" << deoPoslePrepravke << endl;
                  //int staraVrednost = pretvoriIzHexUDec(deoZaPrepravku);
                  int staraVrednost = pretvoriIzHexUDec(deoZaPrepravku);
                  cout << staraVrednost << endl;
                  cout << "Dodaje se: " << stoi(vrednostSimbolaString, nullptr, 0) << endl;
                  int novaVrednost = staraVrednost + stoi(vrednostSimbolaString, nullptr, 0);
                  string novaVrednostString = pretvoriUHexOblik(novaVrednost);
                  novaVrednostString = simbolHexOblik(novaVrednostString);
                  novaVrednostString = stringToUpper(novaVrednostString);
                  novaVrednostString = novaVrednostString.substr(2, 2) + novaVrednostString.substr(0, 2);
                  cout << "Nova: " << novaVrednostString << endl;
                  string noviKod = deoPre + novaVrednostString + deoPoslePrepravke;
                  cout << "Nov izgled linije: " + noviKod << endl;
                  kodSekc.code = noviKod;
                  //exit(-1);
                }
              }else if(elemRel.type == 2){
                cout << "PREPRAVKA ZA OSTALE" << endl;
                cout << "Velicina linije " << kodSekc.code.size() << endl;
                string deoPre = kodSekc.code.substr(0, (pomerajRelokacije - pomerajDec)*2);
                cout << "Deo pre popravke:" << deoPre << endl;
                string deoZaPrepravku = kodSekc.code.substr((pomerajRelokacije - pomerajDec)*2, 4);
                cout << "Deo za prepravku:" << deoZaPrepravku << endl;
                if(deoZaPrepravku.size() == 2){
                  cout << "Mora i naredna linija OSTALI" << endl;
                  string imeFajla = kodSekc.fajl;
                  string nameSect = kodSekc.sectionName;
                  int pom = pomerajDec + 8;
                  string pomString = pretvoriUHexOblik(pom);
                  cout << "POMERAJ STRING OVDE: " << pomString << endl;
                  string drugaLinijaKoda = "";
                  for(auto & kodSekc1 : kodSekcija){
                    if(!kodSekc1.fajl.compare(imeFajla) && !kodSekc1.sectionName.compare(nameSect) && !kodSekc1.offset.compare(pomString)){
                      cout << "EVO USAO ODJEEEE" << endl;
                      drugaLinijaKoda = kodSekc1.code;
                      break;
                    }
                  }
                  deoZaPrepravku += drugaLinijaKoda.substr(0, 2);
                  cout << "Deo za prepravku:" << deoZaPrepravku << endl;
                  string deoPoslePrepravke = drugaLinijaKoda.substr(2, -1);
                  cout << "Deo posle prepravke:" << deoPoslePrepravke << endl;
                  int staraVrednost = pretvoriIzHexUDec(deoZaPrepravku);
                  //cout << staraVrednost << endl;
                  //cout << "Dodaje se: " << stoi(vrednostSimbolaString, nullptr, 0) << endl;
                  int novaVrednost = staraVrednost + stoi(vrednostSimbolaString, nullptr, 0);
                  string novaVrednostString = pretvoriUHexOblik(novaVrednost);
                  novaVrednostString = simbolHexOblik(novaVrednostString);
                  novaVrednostString = stringToUpper(novaVrednostString);
                  cout << "Nova: " << novaVrednostString << endl;
                  string deo1 = novaVrednostString.substr(0, 2);
                  string deo2 = novaVrednostString.substr(2, 2);
                  string kod1 = "";
                  kod1 += deoPre + deo1;
                  string kod2 = "";
                  kod2 = deo2 + deoPoslePrepravke;
                  kodSekc.code = kod1;
                  for(auto & kodSekc1 : kodSekcija){
                    if(!kodSekc1.fajl.compare(imeFajla) && !kodSekc1.sectionName.compare(nameSect) && !kodSekc1.offset.compare(pomString)){
                      cout << "EVO USAO ODJEEE da menjam" << endl;
                      kodSekc1.code = kod2;
                      break;
                    }
                  }
                  //exit(-1);
                }else{
                  cout << "U jednom redu OSTALI!" << endl;
                  string deoPoslePrepravke = kodSekc.code.substr((pomerajRelokacije - pomerajDec)*2 + 4, -1);
                  cout << "Deo posle prepravke:" << deoPoslePrepravke << endl;
                  //int staraVrednost = pretvoriIzHexUDec(deoZaPrepravku);
                  int staraVrednost = pretvoriIzHexUDec(deoZaPrepravku);
                  //cout << staraVrednost << endl;
                  //cout << "Dodaje se: " << stoi(vrednostSimbolaString, nullptr, 0) << endl;
                  int novaVrednost = staraVrednost + stoi(vrednostSimbolaString, nullptr, 0);
                  string novaVrednostString = pretvoriUHexOblik(novaVrednost);
                  novaVrednostString = simbolHexOblik(novaVrednostString);
                  novaVrednostString = stringToUpper(novaVrednostString);
                  cout << "Nova: " << novaVrednostString << endl;
                  string noviKod = deoPre + novaVrednostString + deoPoslePrepravke;
                  cout << "Nov izgled linije: " + noviKod << endl;
                  kodSekc.code = noviKod;
                }
              }else if(elemRel.type == 1){
                cout << "PC REL PREPRAVKA" << endl;
                cout << "Ime sekcije: " << imeSekcije << endl;
                int valueToWrite = 0;
                for(auto & elemSimbNovi : tableSymbols){
                  if(!elemRel.fajl.compare(elemSimbNovi.fajl)){
                    if(!elemSimbNovi.symName.compare(imeSekcije)){
                      cout << "Evo me usao za PC REL PREPRAVKU!" << endl;
                      string valSimbola = elemSimbNovi.symValue; // a bukvalno mislim na adresu sekcije u kojoj ce da se nadje pc rel adresiranje
                      // pa cu onda da dodam jos i offset do tog mesta koriscenja pc rel adresiranja
                      int vrednostSimbola = stoi(valSimbola, nullptr, 0);
                      int pomerajDoRelokacije = elemRel.location;
                      valueToWrite = vrednostSimbola + pomerajDoRelokacije;
                      cout << vrednostSimbola << endl;
                      cout << valueToWrite << endl;
                      break;
                    }
                  }
                }
                cout << "Velicina linije " << kodSekc.code.size() << endl;
                string deoPre = kodSekc.code.substr(0, (pomerajRelokacije - pomerajDec)*2);
                cout << "Deo pre popravke:" << deoPre << endl;
                string deoZaPrepravku = kodSekc.code.substr((pomerajRelokacije - pomerajDec)*2, 4);
                cout << "Deo za prepravku:" << deoZaPrepravku << endl;
                if(deoZaPrepravku.size() == 2){
                  cout << "Mora i naredna linija PC REL" << endl;
                  string imeFajla = kodSekc.fajl;
                  string nameSect = kodSekc.sectionName;
                  int pom = pomerajDec + 8;
                  string pomString = pretvoriUHexOblik(pom);
                  cout << "POMERAJ STRING OVDE: " << pomString << endl;
                  string drugaLinijaKoda = "";
                  for(auto & kodSekc1 : kodSekcija){
                    if(!kodSekc1.fajl.compare(imeFajla) && !kodSekc1.sectionName.compare(nameSect) && !kodSekc1.offset.compare(pomString)){
                      cout << "EVO USAO ODJEEEE" << endl;
                      drugaLinijaKoda = kodSekc1.code;
                      break;
                    }
                  }
                  deoZaPrepravku += drugaLinijaKoda.substr(0, 2);
                  cout << "Deo za prepravku:" << deoZaPrepravku << endl;
                  string deoPoslePrepravke = drugaLinijaKoda.substr(2, -1);
                  cout << "Deo posle prepravke:" << deoPoslePrepravke << endl;
                  //int staraVrednost = pretvoriIzHexUDec(deoZaPrepravku);
                  int staraVrednost = pomocnaMetodaZaNegativneBrojeve(deoZaPrepravku);
                  int dodaj = stoi(vrednostSimbolaString, nullptr, 0) - valueToWrite;
                  int upisiVrednost = staraVrednost + dodaj;
                  string vrednostZaUpisString = pretvoriUHexOblik(upisiVrednost);
                  vrednostZaUpisString = simbolHexOblik(vrednostZaUpisString);
                  vrednostZaUpisString = stringToUpper(vrednostZaUpisString);
                  if(vrednostZaUpisString.size() == 8){
                    vrednostZaUpisString = vrednostZaUpisString.substr(vrednostZaUpisString.size() - 4, 4);
                  }
                  cout << "Vrednost za upis: " << upisiVrednost << endl;
                  cout << vrednostZaUpisString << endl;
                  string deo1 = vrednostZaUpisString.substr(0, 2);
                  string deo2 = vrednostZaUpisString.substr(2, 2);
                  string kod1 = "";
                  string kod2 = "";
                  kod1 += deoPre + deo1;
                  kod2 += deo2 + deoPoslePrepravke;
                  kodSekc.code = kod1;
                  for(auto & kodSekc1 : kodSekcija){
                    if(!kodSekc1.fajl.compare(imeFajla) && !kodSekc1.sectionName.compare(nameSect) && !kodSekc1.offset.compare(pomString)){
                      cout << "EVO USAO ODJEEE da menjam" << endl;
                      kodSekc1.code = kod2;
                      break;
                    }
                  }
                  //exit(-1);
                }else{
                  cout << "U jednom redu PC REL" << endl;
                  string deoPoslePrepravke = kodSekc.code.substr((pomerajRelokacije - pomerajDec)*2 + 4, -1);
                  cout << "Deo posle prepravke:" << deoPoslePrepravke << endl;
                  //int staraVrednost = pretvoriIzHexUDec(deoZaPrepravku);
                  int staraVrednost = pomocnaMetodaZaNegativneBrojeve(deoZaPrepravku);
                  int dodaj = stoi(vrednostSimbolaString, nullptr, 0) - valueToWrite;
                  int upisiVrednost = staraVrednost + dodaj;
                  string vrednostZaUpisString = pretvoriUHexOblik(upisiVrednost);
                  vrednostZaUpisString = simbolHexOblik(vrednostZaUpisString);
                  vrednostZaUpisString = stringToUpper(vrednostZaUpisString);
                  if(vrednostZaUpisString.size() == 8){
                    vrednostZaUpisString = vrednostZaUpisString.substr(vrednostZaUpisString.size() - 4, 4);
                  }
                  cout << "Vrednost za upis: " << upisiVrednost << endl;
                  cout << vrednostZaUpisString << endl;
                  string noviKod = deoPre + vrednostZaUpisString + deoPoslePrepravke;
                  cout << "Nov izgled linije: " + noviKod << endl;
                  kodSekc.code = noviKod;
                }
              }
            }
          }
        }
      }
    }else{
      cout << "***" << endl;
      cout << "Nije pronadjena definicija simbola: " << simbol << " ni u jednom ulaznom fajlu!" << endl;
      cout << "***" << endl;
      return -1;
    }
  }
  return 0;
}
*/

string Linker::dodajDodatneSpejsove(string s){
  string deo1 = s.substr(0, 2);
  string deo2 = s.substr(2, 2);
  string deo3 = s.substr(4, 2);
  string deo4 = s.substr(6, 2);
  string deo5 = s.substr(8, 2);
  string deo6 = s.substr(10, 2);
  string deo7 = s.substr(12, 2);
  string deo8 = s.substr(14, 2);
  string noviString =  deo1 + " " + deo2 + " " + deo3 + " " + deo4 + " " + deo5 + " " + deo6 + " " + deo7 + " " + deo8;
  return noviString;
}

string Linker::spejsoviZaZadnjuLiniju(string s){
  //cout << "Evo me ovde: " << s << endl;
  //cout << "Velciina linije: " << s.size() << endl;
  string novi;
  if(s.size() == 2){
    novi = s;
  }else if(s.size() == 4){
    string deo1 = s.substr(0, 2);
    string deo2 = s.substr(2, 2);
    novi = deo1 + " " + deo2;
  }else if(s.size() == 6){
    string deo1 = s.substr(0, 2);
    string deo2 = s.substr(2, 2);
    string deo3 = s.substr(4, 2);
    novi = deo1 + " " + deo2 + " " + deo3;
  }else if(s.size() == 8){
    string deo1 = s.substr(0, 2);
    string deo2 = s.substr(2, 2);
    string deo3 = s.substr(4, 2);
    string deo4 = s.substr(6, 2);
    novi = deo1 + " " + deo2 + " " + deo3 + " " + deo4;
  }else if(s.size() == 10){
    string deo1 = s.substr(0, 2);
    string deo2 = s.substr(2, 2);
    string deo3 = s.substr(4, 2);
    string deo4 = s.substr(6, 2);
    string deo5 = s.substr(8, 2);
    novi = deo1 + " " + deo2 + " " + deo3 + " " + deo4 + " " + deo5;
  }else if(s.size() == 12){
    string deo1 = s.substr(0, 2);
    string deo2 = s.substr(2, 2);
    string deo3 = s.substr(4, 2);
    string deo4 = s.substr(6, 2);
    string deo5 = s.substr(8, 2);
    string deo6 = s.substr(10, 2);
    novi = deo1 + " " + deo2 + " " + deo3 + " " + deo4 + " " + deo5 + " " + deo6;
  }else if(s.size() == 14){
    string deo1 = s.substr(0, 2);
    string deo2 = s.substr(2, 2);
    string deo3 = s.substr(4, 2);
    string deo4 = s.substr(6, 2);
    string deo5 = s.substr(8, 2);
    string deo6 = s.substr(10, 2);
    string deo7 = s.substr(12, 2);
    novi = deo1 + " " + deo2 + " " + deo3 + " " + deo4 + " " + deo5 + " " + deo6 + " " + deo7;
  }else if(s.size() == 16){
    string deo1 = s.substr(0, 2);
    string deo2 = s.substr(2, 2);
    string deo3 = s.substr(4, 2);
    string deo4 = s.substr(6, 2);
    string deo5 = s.substr(8, 2);
    string deo6 = s.substr(10, 2);
    string deo7 = s.substr(12, 2);
    string deo8 = s.substr(14, 2);
    novi = deo1 + " " + deo2 + " " + deo3 + " " + deo4 + " " + deo5 + " " + deo6 + " " + deo7 + " " + deo8;
  }
  return novi;
}

// Koristio sam ranije, dok sam imao metodu koja je pogresno razresavala relokacije
/*void Linker::ispisZaEmulator(){
  cout << "-----" << endl;
  //outputFile << "Evo me" << endl;
  string kodZaIspis = "";
  for(auto & eSek : tableSections){
    cout << eSek.fajl << " " << eSek.name << " " << eSek.beginningAddress << " " << eSek.size << endl;
    for(auto & kodSek : kodSekcija){
      if(!eSek.fajl.compare(kodSek.fajl) && !eSek.name.compare(kodSek.sectionName)){
        kodZaIspis += kodSek.code;
      }
    }
  }
  int velDoSada = kodZaIspis.size();
  int novaVelicina = (velDoSada / 16 + 1) * 16;
  int razlika = novaVelicina - velDoSada;
  for(int br = 0; br < razlika; br++){
    kodZaIspis += "0"; // Ovo proveriti da li treba da se radi!!!
  }
  cout << "Velicina koda: " << velDoSada << endl;
  int brojacAdresa = 0;
  vector<string> zaIspis;
  for(int i = 0; i < velDoSada / 16; i++){
    string kodDeo = kodZaIspis.substr(i*16, 16);
    zaIspis.push_back(kodDeo);
  }
  int brojac = 0;
  for(vector<string>::iterator it = zaIspis.begin() ; it != zaIspis.end() ;){
    string kodDeo = *it;
    ++it;
    string adresaHex = pretvoriUHexOblik(brojac);
    adresaHex = simbolHexOblik(adresaHex);
    string kodSaRazmacima = dodajDodatneSpejsove(kodDeo);
    cout << adresaHex << ": " << kodSaRazmacima << endl;
    outputFile << adresaHex << ": " << kodSaRazmacima << endl;
    brojac += 8;
  }
  //cout << brojac * 2 << "---" << velDoSada << endl;
  string zavrsetak = kodZaIspis.substr(brojac*2, velDoSada - brojac*2);
  //cout << "Kraj: " << zavrsetak << endl;
  if(zavrsetak.size() > 0){
    int razlikaNova = brojac*2 - velDoSada;
    string adresaZadnjeLinije = pretvoriUHexOblik(brojac);
    adresaZadnjeLinije = simbolHexOblik(adresaZadnjeLinije);
    string zadnjaLinija = kodZaIspis.substr(brojac*2, razlikaNova);
    //zadnjaLinija = dodajDodatneSpejsove(zadnjaLinija);
    zadnjaLinija = spejsoviZaZadnjuLiniju(zadnjaLinija);
    //cout << "LOL: " << zadnjaLinija << endl;
    cout << adresaZadnjeLinije << ": " << zadnjaLinija << endl;
    outputFile << adresaZadnjeLinije << ": " << zadnjaLinija;
    //cout << zadnjaLinija << endl;
    //cout << "Brojac: " << brojac << endl;
  }
  //outputFile << kodZaIspis << endl;
}*/

void Linker::ispisZaEmulatorNovo(){
  /*cout << endl << "Kod je: " << endl;
  cout << kodKonacni << endl;*/
  int velDoSada = kodKonacni.size();
  int novaVelicina = (velDoSada / 16 + 1) * 16;
  int razlika = novaVelicina - velDoSada;
  /*for(int br = 0; br < razlika; br++){
    kodZaIspis += "0"; // Ovo proveriti da li treba da se radi!!!
  }*/
  //cout << "Velicina koda: " << velDoSada << endl;
  int brojacAdresa = 0;
  vector<string> zaIspis;
  for(int i = 0; i < velDoSada / 16; i++){
    string kodDeo = kodKonacni.substr(i*16, 16);
    zaIspis.push_back(kodDeo);
  }
  int brojac = 0;
  for(vector<string>::iterator it = zaIspis.begin() ; it != zaIspis.end() ;){
    string kodDeo = *it;
    ++it;
    string adresaHex = pretvoriUHexOblik(brojac);
    adresaHex = simbolHexOblik(adresaHex);
    string kodSaRazmacima = dodajDodatneSpejsove(kodDeo);
    //cout << adresaHex << ": " << kodSaRazmacima << endl;
    outputFile << adresaHex << ": " << kodSaRazmacima;
    brojac += 8;
    if(brojac*2 < velDoSada){
      outputFile << endl;
    }
  }
  //cout << brojac * 2 << "---" << velDoSada << endl;
  string zavrsetak = kodKonacni.substr(brojac*2, velDoSada - brojac*2);
  //cout << "Kraj: " << zavrsetak << endl;
  if(zavrsetak.size() > 0){
    int razlikaNova = brojac*2 - velDoSada;
    string adresaZadnjeLinije = pretvoriUHexOblik(brojac);
    adresaZadnjeLinije = simbolHexOblik(adresaZadnjeLinije);
    string zadnjaLinija = kodKonacni.substr(brojac*2, razlikaNova);
    //zadnjaLinija = dodajDodatneSpejsove(zadnjaLinija);
    zadnjaLinija = spejsoviZaZadnjuLiniju(zadnjaLinija);
    //cout << "LOL: " << zadnjaLinija << endl;
    //cout << adresaZadnjeLinije << ": " << zadnjaLinija << endl;
    outputFile << adresaZadnjeLinije << ": " << zadnjaLinija;
    //cout << zadnjaLinija << endl;
    //cout << "Brojac: " << brojac << endl;
  }
}

void Linker::obrada(){
  //cout << "U obradi sam" << endl;
  outputFile.open(imeIzlaza);
  if(!outputFile.is_open()){
    cout << "*** Greska prilikom otvaranja izlaznog fajla! ***" << endl;
    exit(-1);
  }else{
    cout << "Uspesno otvoren izlazni fajl: " << imeIzlaza << endl;
    int retVal = prolazakKrozUlazneFajlove();
    //ispisiDoSada();
    if(retVal < 0){
      cout << "GRESKA! Doslo je do greske pri prolasku kroz ulazni fajl za linker!" << endl;
      exit(-1);
    }
    retVal = obradiSekcije();
    if(retVal < 0){
      exit(-1);
    }
    retVal = prepraviTabeluSimbola();
    if(retVal < 0){
      exit(-1);
    }
    retVal = prepraviRelokacije();
    //ispisiDoSada();
    //pomocnaTabelaSekcija();
    if(retVal < 0){
      //cout << "-----" << endl;
      cout << "Zavrsen proces linkovanja ulaznih objektnih fajlova. NEUSPEH!" << endl;
    }else{
      //ispisZaEmulator();
      ispisZaEmulatorNovo();
      //cout << "-----" << endl;
      cout << "Zavrsen proces linkovanja ulaznih objektnih fajlova. USPEH!" << endl;
    }
  }
}
