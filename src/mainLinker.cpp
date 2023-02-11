#include <stdio.h>
#include <iostream>
#include <vector>

#include "../inc/linker.hpp"
//#include "/home/ss/Desktop/resenje/inc/linker.hpp"

using namespace std;

int main(int argc, char* argv[]){
  string argument = argv[0];
  string imeArgumenta = argument.substr(2, -1);
  string imeIzlaza;
  int naReduJeIzlaz = 0;
  vector<string> inputFiles;
  if(!imeArgumenta.compare("linker")){
    if(argc < 2){
      cout << "Niste prosledili opcione komande" << endl;
    }else{
      string opcija = argv[1];
      if(!opcija.compare("-hex") || !opcija.compare("-HEX")){
        //cout << "Opcija za pokretanje je: " << opcija << endl;
        if(argc < 5){
          cout << "Morate uneti: -o imeIzlaza.hex i bar 1 ulazni fajl za linker, pozeljno 2" << endl;
        }else{
          for(int i = 2; i < argc; i++){
            string currArg = argv[i];
            //cout << currArg << endl;
            if(naReduJeIzlaz == 1){
              naReduJeIzlaz = 0;
              imeIzlaza = argv[i];
              string ekstenzija = imeIzlaza.substr(imeIzlaza.size() - 4, 4);
              //cout << ekstenzija << endl;
              if(imeIzlaza.size() < 5 || ekstenzija.compare(".hex")){
                cout << "*** Ekstenzija izlaznog fajla mora biti .hex! ***" << endl;
                exit(-1);
              }
            }else if(!currArg.compare("-o") || !currArg.compare("-O")){
              naReduJeIzlaz = 1;
            }else{
              inputFiles.push_back(currArg);
            }
          }
          //cout << "Ime izlaza: " << imeIzlaza << endl;
          Linker ld(imeIzlaza, inputFiles);
          ld.obrada();
          ld.zatvoriFajlove();
        }
      }else{
        cout << "Uneta opcija za pokretanje nije podrzana: " << opcija << endl;
      }
    }
  }else{
    cout << "Komanda za pokretanje linkera mora poceti sa: ./linker" << endl;
  }
  return 0;
}

/*cout << "TABELA RELOKACIJA" << endl;
for(int i = 0; i < tableReallocations.size(); i++){
  cout << tableReallocations[i].fajl << ":" << tableReallocations[i].sekcija << ":" << tableReallocations[i].refSym << ":";
  cout << tableReallocations[i].location << ":" << tableReallocations[i].locationHex << ":" << tableReallocations[i].type << ":";
  cout << tableReallocations[i].addend << ":" << tableReallocations[i].realAddend << endl;
}*/