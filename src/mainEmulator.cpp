#include <iostream>
#include "../inc/emulator.hpp"
//#include "/home/ss/Desktop/resenje/inc/emulator.hpp"

int main(int argc, char* argv[]){
  //cout << "POKRECEM EMULATOR!" << endl;
  //cout << "Usao u main od emulatora" << endl;
  string argumentPrvi = "";
  string argumentDrugi = "";
  string ime = "";
  if(argc < 2){
    cout << "Pogresno ste pokrenuli emulator. Pokrecu se kao ./emulator imeDatoteke.hex" << endl;
    return -1;;
  }else if(argc == 2){
    argumentPrvi = argv[0];
    argumentDrugi = argv[1];
    int pozicija = argumentPrvi.find_last_of("/");
    ime = argumentPrvi.substr(pozicija + 1, -1);
    if(!ime.compare("emulator") || !ime.compare("EMULATOR")){
      //cout << "Sve je u redu sa pozivom." << endl;
      Emulator emulator(argumentDrugi);
      emulator.emuliraj();
    }else{
      cout << "Prvi argument mora biti emulator" << endl;
      return -1;
    }
  }else{
    cout << "Greska pri pokretanju. Pokrece se kao ./emulator imeDatoteke.hex" << endl;
    return -1;
  }
  return 0;
}
