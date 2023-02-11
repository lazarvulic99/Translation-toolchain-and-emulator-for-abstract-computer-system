#include <iostream>
#include <fstream>
#include <regex>

#include "../inc/assembler.hpp"
//#include "/home/ss/Desktop/resenje/inc/assembler.hpp"

using namespace std;

int main(int argc, const char *argv[]){
  string ulazniFile;
  string izlazniFile;
  if(argc < 2){
    cout << "*** Niste prosledili dovoljan broj argumenata. Treba biti u formatu ./asembler -o izlaz.o ulaz.o ***" << endl;
    return -1;
  }
  string opcija = argv[1];
  if(opcija != "-o"){
    cout << "*** Asembler kao 2. argument komandne linije zahteva -o ***" << endl;
    return -1;
  }else{
    //cout << "[ Uspesno stigao ovde ]" << endl;
    izlazniFile = argv[2];
    ulazniFile = argv[3];
    Assembler as(ulazniFile, izlazniFile);
    int ret = as.checkFileExtensions();
    if(ret < 0){
      cout << "*** Greska pri prosledjivanju parametara. Proverite argumente ! ***" << endl;
      return -1;
    }
    as.initializeAssembler();
    ret = as.assembly();
    if(ret < 0){
      cout << "*** Greska pri radu asemblera. ***" << endl;
      return -1;
    }
    return 0;
  }
}