#include "../inc/emulator.hpp"
//#include "/home/ss/Desktop/resenje/inc/emulator.hpp"
#include <bits/stdc++.h>
#include <cmath>

void Emulator::inicijalizujEmulator(){
  //cout << "Inicijalizacija emulatora zapoceta!" << endl;
  this->memorija[65536] = {0};
  //Naravno ovaj deo cu da zakomentarisem, cisto radi testova stavljeno, zakomentarisi linije: 8, 9, 10, 11, 12
  this->registriOpsteNamene[6] = {0};
  /*this->registriOpsteNamene[5] = 19;
  this->registriOpsteNamene[4] = 4;
  this->registriOpsteNamene[3] = 2;
  this->registriOpsteNamene[0] = 43981;*/
  this->regPC = 0;
  this->regSP = 0;
  this->regPSW = 0x6000; // 0110 0000 0000 0000, moze da se inicijalizuje i na 0010 0000 0000 0000
  // jer prekidi od terminala i tajmera mogu da ostanu omoguceni, a ne maskirani, oni su za B i C nivo
  this->postojiGreska = 0;
  this->nasaoHalt = 0;
  // Interrupti omoguceni, prekidi od terminala i tajmera maskirani, NCOZ = 0000
  //cout << "Inicijalizacija emulatora zavrsena!" << endl;
}

Emulator::Emulator(string imeUlaznogFajla){
  //cout << "Usao u konstruktor" << endl;
  inputFileName = imeUlaznogFajla;
  //cout << "Ulazni fajl: " << inputFileName << endl;
  inicijalizujEmulator();
}

string Emulator::pretvoriPSWuBite(unsigned short registarZaIspis){
  string broj = "";
  for(int i = 15; i >= 0; i--){
    if(registarZaIspis >= pow(2, i)){
      broj += "1";
      registarZaIspis -= pow(2, i);
    }else{
      broj += "0";
    }
  }
  return broj;
}

template<typename T>
string Emulator::toBinaryString(const T& registarZaIspis){ // https://stackoverflow.com/questions/7349689/how-to-print-using-cout-a-number-in-binary-form
  stringstream ss;
  ss << bitset<sizeof(T)*8>(registarZaIspis);
  return ss.str();
}

string Emulator::pretvoriUOblikZaIspis(unsigned short registar){
  ostringstream ss;
  ss << "0x" << setfill('0') << setw(8) << hex << registar;
  string result = ss.str();
  result = result.substr(result.size() - 4, -1);
  return result;
}

int Emulator::pretvoriAdresuUInt(string adresa){
  return stol(adresa, nullptr, 16); // Proveri da li ovo radi za sve slucajeve!
}

int Emulator::pretvoriHexUInt(string hex){
  return stol(hex, nullptr, 16); // Proveri da li ovo radi za sve slucajeve!
}

// Sluzi mi kao i u asembleru i u linkeru, da istokeniziram liniju, po delimiteru koji je ovde blanko
void tokenize(string const &str, const char delim, vector<string> &out)
{
    stringstream ss(str);
    string s;
    while (getline(ss, s, delim)) {
        out.push_back(s);
    }
}

void Emulator::upisiBajtoveUMemoriju(unsigned short vrednostZaUpis, unsigned short adresaUpisa){
  //cout << "Upis bajtova u memoriju!" << endl;
  char niziBajt = vrednostZaUpis & 0xFF;
  char visiBajt = (vrednostZaUpis >> 8) & 0xFF;
  memorija[adresaUpisa] = niziBajt;
  memorija[adresaUpisa + 1] = visiBajt;
  //cout << +niziBajt << "___" << +visiBajt << endl;
  //cout << "Adresa: " << +adresaUpisa << ", vrednost: " << +vrednostZaUpis << endl; 
}

unsigned short Emulator::dovuciDvaBajta(){
  unsigned short niziBajt;
  unsigned short visiBajt;
  niziBajt = memorija[regPC++];
  visiBajt = memorija[regPC++];
  return 256*visiBajt + niziBajt;
}

unsigned short Emulator::dovuciDvaBajtaZaInstrukciju(){
  unsigned short niziBajt;
  unsigned short visiBajt;
  visiBajt = memorija[regPC++];
  niziBajt = memorija[regPC++];
  return 256*visiBajt + niziBajt;
}

unsigned char Emulator::dovuciJedanBajt(){
  unsigned char dovuceniBajt = memorija[regPC++];
  return dovuceniBajt;
}

void Emulator::umanjiSP(){
  regSP = regSP - 2;
  //cout << "SP:" << +regSP << endl;
}

void Emulator::uvecajSP(){
  regSP = regSP + 2;
  //cout << "SP:" << +regSP << endl;
}

void Emulator::postaviInterruptFlag(){
  regPSW |= (1 << 15);
}

void Emulator::resetujInterruptFlag(){
  regPSW &= ~(1 << 15);
}

void Emulator::prekidnaRutina(){
  // Ovde programer mora da obezbedi, da se u ulazu 1 IVT tabele, koja krece od NULE!!! nadje adresa prekidne rutine
  // u MEMORIJI na adresama 2 i 3 su upisane nule, pa ce shodno instrukciji regPC = niziBajt*256 + visiBajt
  // u PC biti upisana nula, a pri narednoj iteraciji while petlje, ce sve biti prekinuto, zbog postojanja
  // greske u emulatoru. Kao rezultat greske, bit I u regPSW ce biti postavljen na 1, odnosno spoljasnji prekidi
  // ce biti maskirani!
  //cout << "Ulazak u prekidnu rutinu za greske!" << endl;
  umanjiSP();
  upisiBajtoveUMemoriju(regPSW, regSP); // bukvalno uradjen push psw za na vrh steka(kog smo ucinili da bude slobodan)
  // zelimo da sacuvamo izgled pswa nakon vracanja iz prekidne rutine
  // isto to zelimo da uradimo i sa PC, kako bi program nastavio da se izvrsava nakon obrade prek. rutine
  umanjiSP();
  upisiBajtoveUMemoriju(regPC, regSP);
  // novi PC treba da bude adresa prekidne rutine
  // ulaz 1 sadrzi adresu prekidne rutine, dakle to je u memoriji na adresi 2 i 3
  unsigned char niziBajt = memorija[2];
  unsigned char visiBajt = memorija[3];


  // KOMENTAR:prvo je bilo kao linija 147, sad je kao 148
  //regPC = niziBajt*256 + visiBajt; // ili obrnuto visiBajt*256 + niziBajt??????????
  regPC = visiBajt*256 + niziBajt;


  // treba i maskirati spoljasnje prekide, tj postaviti 15.bit u regPSW na 1.
  postaviInterruptFlag();
}

int Emulator::proveriIspravnostRegistra(unsigned char reg1, unsigned char reg2, string mnemonik){
  if(reg2 > 8){
    cout << "GRESKA! Registri mogu uzeti vrednosti od [0-8] u instrukciji:" << mnemonik << "!" << endl;
    postojiGreska = 1;
    regPC = oldPC;
    // ovde treba da dodje poziv prekidne rutine!
    prekidnaRutina();
    return 0;
  }else{
    return 1;
  }
}

int Emulator::proverObaveznogRegistraKojiJeF(unsigned char reg, string mnemonik){
  if(reg != 15){
    cout << "GRESKA! Registri koji su nebitni, po konvenciji asemblera dobijaju vrednost F!" << mnemonik << "!" << endl;
    postojiGreska = 1;
    regPC = oldPC;
    // ovde treba da dodje poziv prekidne rutine!
    prekidnaRutina();
    return 0;
  }else{
    return 1;
  }
}

int Emulator::proveriIspravnostObaRegistra(string mnemonik, unsigned char reg1, unsigned char reg2){
  if(reg1 > 8 || reg2 > 8){
    cout << "GRESKA! Registri mogu uzeti vrednosti od [0-8] u instrukciji:" << mnemonik << "!" << endl;
    postojiGreska = 1;
    regPC = oldPC;
    // ovde treba da dodje poziv prekidne rutine!
    prekidnaRutina();
    return 0;
  }else{
    return 1;
  }
}

void Emulator::odradiPosaoOkoRegistra(unsigned char reg1, unsigned char reg2){
  if(reg2 >= 0 && reg2 <= 5){
    zaAdresiranje = registriOpsteNamene[reg2];
    //cout << "Reg 0-5" << endl;
  }else if(reg2 == 6){
    zaAdresiranje = regSP;
    //cout << "Reg SP" << endl;
  }else if(reg2 == 7){
    zaAdresiranje = regPC;
    //cout << "Reg PC" << endl;
  }else if(reg2 == 8){
    zaAdresiranje = regPSW;
    //cout << "Reg PSW" << endl;
  }
}

unsigned short Emulator::vratiVrednostRegistra(unsigned char registarMoj){
  unsigned short regVraceni;
  if(registarMoj >= 0 && registarMoj <= 5){
    regVraceni = registriOpsteNamene[registarMoj];
  }else if(registarMoj == 6){
    regVraceni = regSP;
  }else if(registarMoj == 7){
    regVraceni = regPC;
  }else if(registarMoj == 8){
    regVraceni = regPSW;
  }
  return regVraceni;
}

void Emulator::upisiURegistar(unsigned char reg1, unsigned short vrednost){
  if(reg1 >= 0 && reg1 <= 5){
    registriOpsteNamene[reg1] = vrednost;
  }else if(reg1 == 6){
    regSP = vrednost;
  }else if(reg1 == 7){
    regPC = vrednost;
  }else if(reg1 == 8){
    regPSW = vrednost;
  }
}

void Emulator::zameniVrednosti(unsigned short vrednost1, unsigned char reg1, unsigned short vrednost2, unsigned char reg2){
  unsigned short vred1 = vrednost2;
  unsigned short vred2 = vrednost1;
  upisiURegistar(reg1, vred1);
  upisiURegistar(reg2, vred2);
}

int Emulator::odradiPosaoOkoAdresiranja(unsigned char reg1, unsigned char reg2, unsigned char nacinAdresiranja, unsigned char nacinAzuriranja, string mnemonik, int type){
  if(type == 0){
    //cout << "Obrada adresiranja kod SKOKova" << endl;
    if(nacinAdresiranja == 0){
      //cout << "Absolute/neposredno adr." << endl;
      if(nacinAzuriranja != 0){
        cout << "GRESKA! Nacin azuriranja moze biti specificiran samo kod reg.ind. adresiranja" << mnemonik << "!" << endl;
        postojiGreska = 1;
        regPC = oldPC;
        // ovde treba da dodje poziv prekidne rutine!
        prekidnaRutina();
        return 0;
      }
      zaAdresiranje = dovuciDvaBajtaZaInstrukciju();
      //cout << +zaAdresiranje << endl;
      return 1;
    }else if(nacinAdresiranja == 1){
      //cout << "REGDIR adr." << endl;
      if(nacinAzuriranja != 0){
        cout << "GRESKA! Nacin azuriranja moze biti specificiran samo kod reg.ind. adresiranja" << mnemonik << "!" << endl;
        postojiGreska = 1;
        regPC = oldPC;
        // ovde treba da dodje poziv prekidne rutine!
        prekidnaRutina();
        return 0;
      }
      int ispravno = proveriIspravnostRegistra(reg1, reg2, mnemonik);
      if(ispravno){
        odradiPosaoOkoRegistra(reg1, reg2);
        return 1;
      }else{
        return 0;
      }
    }else if(nacinAdresiranja == 2){
      //cout << "REGIND adr." << endl;
      if(nacinAzuriranja != 0){
        cout << "GRESKA! Nacin azuriranja kod reg.ind. za skokove mora biti 0 - nema azuriranja." << mnemonik << "!" << endl;
        postojiGreska = 1;
        regPC = oldPC;
        // ovde treba da dodje poziv prekidne rutine!
        prekidnaRutina();
        return 0;
      }
      int ispravno = proveriIspravnostRegistra(reg1, reg2, mnemonik);
      if(ispravno){
        odradiPosaoOkoRegistra(reg1, reg2);
        //cout << +zaAdresiranje << endl;
        unsigned char niziBajt = memorija[zaAdresiranje];
        unsigned char visiBajt = memorija[zaAdresiranje + 1];
        zaAdresiranje = visiBajt*256 + niziBajt;
        return 1;
      }else{
        return 0;
      }
    }else if(nacinAdresiranja == 3){
      //cout << "REGIND sa POMERAJEM adr." << endl;
      if(nacinAzuriranja != 0){
        cout << "GRESKA! Nacin azuriranja kod reg.ind. sa pomerajem za skokove mora biti 0 - nema azuriranja." << mnemonik << "!" << endl;
        postojiGreska = 1;
        regPC = oldPC;
        // ovde treba da dodje poziv prekidne rutine!
        prekidnaRutina();
        return 0;
      }
      unsigned short zadnjaDvaBajta = dovuciDvaBajtaZaInstrukciju(); // to je payload iliti pomeraj
      int ispravno = proveriIspravnostRegistra(reg1, reg2, mnemonik);
      if(ispravno){
        odradiPosaoOkoRegistra(reg1, reg2);
        //cout << +zaAdresiranje << endl;
        unsigned char niziBajt = memorija[zaAdresiranje + zadnjaDvaBajta];
        unsigned char visiBajt = memorija[zaAdresiranje + zadnjaDvaBajta + 1];
        zaAdresiranje = visiBajt*256 + niziBajt;
        //cout << +zaAdresiranje << endl;
        return 1;
      }else{
        return 0;
      }
    }else if(nacinAdresiranja == 4){
      //cout << "MEMDIR adr." << endl;
      if(nacinAzuriranja != 0){
        cout << "GRESKA! Nacin azuriranja moze biti specificiran samo kod reg.ind. adresiranja" << mnemonik << "!" << endl;
        postojiGreska = 1;
        regPC = oldPC;
        // ovde treba da dodje poziv prekidne rutine!
        prekidnaRutina();
        return 0;
      }
      unsigned short instPayload = dovuciDvaBajtaZaInstrukciju();
      unsigned char niziBajt = memorija[instPayload];
      unsigned char visiBajt = memorija[instPayload + 1];
      zaAdresiranje = visiBajt*256 + niziBajt;
      //cout << +visiBajt << "_" << +niziBajt << "_" << +zaAdresiranje << endl;
      return 1;
    }else if(nacinAdresiranja == 5){
      //cout << "PC REL. adr." << endl;
      if(nacinAzuriranja != 0){
        cout << "GRESKA! Nacin azuriranja moze biti specificiran samo kod reg.ind. adresiranja" << mnemonik << "!" << endl;
        postojiGreska = 1;
        regPC = oldPC;
        // ovde treba da dodje poziv prekidne rutine!
        prekidnaRutina();
        return 0;
      }
      unsigned short zadnjaDvaBajtaInstr = dovuciDvaBajtaZaInstrukciju();
      int ispravno = proveriIspravnostRegistra(reg1, reg2, mnemonik);
      if(ispravno){
        odradiPosaoOkoRegistra(reg1, reg2);
        //return 1;
      }else{
        return 0;
      }
      zaAdresiranje += zadnjaDvaBajtaInstr;
      return 1;
    }else{
      cout << "GRESKA! Za instrukciju: " << mnemonik << " ne postoji taj nacin azuriranja!" << endl;
      postojiGreska = 1;
      regPC = oldPC;
      // ovde treba da dodje poziv prekidne rutine!
      prekidnaRutina();
      return 0;
    }
    return 1;
  }else if(type == 1){
    // Nacin azruriranja za LOAD/POP instrukcije nam govori o tome da li se radi o instrukciji ldr ili instrukciji pop
    // Ukoliko je prvih 4b 3.bajta instrukcije 4, radi se o pop instrukciji => uvecava se za 2 nakon formiranja adrese operanda
    // Ukoliko je prvih 4b 3.bajta instrukcije 0, radi se ldr instrukciji => nema azuriranja
    //cout << "Odradjujem posao za adresiranje kod load/pop instrukcija" << endl;
    if(nacinAdresiranja == 0){
      //cout << "Absolute/neposredno adr." << endl;
      if(nacinAzuriranja != 0){
        cout << "GRESKA! Nacin azuriranja moze biti specificiran samo kod reg.ind. adresiranja za LDR" << mnemonik << "!" << endl;
        postojiGreska = 1;
        regPC = oldPC;
        // ovde treba da dodje poziv prekidne rutine!
        prekidnaRutina();
        return 0;
      }
      zaAdresiranje = dovuciDvaBajtaZaInstrukciju();
      //cout << +zaAdresiranje << endl;
      return 1;
    }else if(nacinAdresiranja == 1){
      //cout << "REGDIR adr." << endl;
      if(nacinAzuriranja != 0){
        cout << "GRESKA! Nacin azuriranja moze biti specificiran samo kod reg.ind. adresiranja" << mnemonik << "!" << endl;
        postojiGreska = 1;
        regPC = oldPC;
        // ovde treba da dodje poziv prekidne rutine!
        prekidnaRutina();
        return 0;
      }
      int ispravno = proveriIspravnostRegistra(reg1, reg2, mnemonik);
      if(ispravno){
        odradiPosaoOkoRegistra(reg1, reg2);
        return 1;
      }else{
        return 0;
      }
    }else if(nacinAdresiranja == 2){
      //cout << "REGIND adr." << endl;
      int ispravno = proveriIspravnostRegistra(reg1, reg2, mnemonik);
      if(ispravno){
        odradiPosaoOkoRegistra(reg1, reg2);
        //cout << +zaAdresiranje << endl;
        if(nacinAzuriranja != 0 && nacinAzuriranja != 4){
          cout << "Zabranjen nacin azuriranja registra kod: " << mnemonik << endl;
          regPC = oldPC;
          postojiGreska = 1;
          // tu treba da dodje obrada prekidne rutine
          prekidnaRutina();
          return 0;
        }else if(nacinAzuriranja == 4){
          //regSP = regSP + 2;
          // Uvecava se za 2 nakon formiranja adrese operanda
          uvecajSP();
        }
        unsigned char niziBajt = memorija[zaAdresiranje];
        unsigned char visiBajt = memorija[zaAdresiranje + 1];
        zaAdresiranje = visiBajt*256 + niziBajt;
        return 1;
      }else{
        return 0;
      }
    }else if(nacinAdresiranja == 3){
      //cout << "REGIND sa POMERAJEM adr." << endl;
      if(nacinAzuriranja != 0){
        cout << "GRESKA! Nacin azuriranja moze biti specificiran samo kod reg.ind. adresiranja" << mnemonik << "!" << endl;
        postojiGreska = 1;
        regPC = oldPC;
        // ovde treba da dodje poziv prekidne rutine!
        prekidnaRutina();
        return 0;
      }
      unsigned short zadnjaDvaBajta = dovuciDvaBajtaZaInstrukciju(); // to je payload iliti pomeraj
      int ispravno = proveriIspravnostRegistra(reg1, reg2, mnemonik);
      if(ispravno){
        odradiPosaoOkoRegistra(reg1, reg2);
        //cout << +zaAdresiranje << endl;
        unsigned char niziBajt = memorija[zaAdresiranje + zadnjaDvaBajta];
        unsigned char visiBajt = memorija[zaAdresiranje + zadnjaDvaBajta + 1];
        zaAdresiranje = visiBajt*256 + niziBajt;
        //cout << +zaAdresiranje << endl;
        return 1;
      }else{
        return 0;
      }
    }else if(nacinAdresiranja == 4){
      //cout << "MEMDIR adr." << endl;
      if(nacinAzuriranja != 0){
        cout << "GRESKA! Nacin azuriranja moze biti specificiran samo kod reg.ind. adresiranja" << mnemonik << "!" << endl;
        postojiGreska = 1;
        regPC = oldPC;
        // ovde treba da dodje poziv prekidne rutine!
        prekidnaRutina();
        return 0;
      }
      unsigned short instPayload = dovuciDvaBajtaZaInstrukciju();
      unsigned char niziBajt = memorija[instPayload];
      unsigned char visiBajt = memorija[instPayload + 1];
      zaAdresiranje = visiBajt*256 + niziBajt;
      //cout << +visiBajt << "_" << +niziBajt << "_" << +zaAdresiranje << endl;
      return 1;
    }else if(nacinAdresiranja == 5){
      // proveri da li ovo uopste treba
      // Ovde nece ni da se ulazi jer za PCREL adresiranje kod LOAD instrukcija nizih 4b 3. bajta je 3, a ne 5 kao kod skokova
      //cout << "PC REL. adr." << endl;
      unsigned short zadnjaDvaBajtaInstr = dovuciDvaBajtaZaInstrukciju();
      int ispravno = proveriIspravnostRegistra(reg1, reg2, mnemonik);
      if(ispravno){
        odradiPosaoOkoRegistra(reg1, reg2);
        //return 1;
      }else{
        return 0;
      }
      zaAdresiranje += zadnjaDvaBajtaInstr;
      return 1;
    }else{
      cout << "GRESKA! Za instrukciju: " << mnemonik << " ne postoji taj nacin azuriranja!" << endl;
      postojiGreska = 1;
      regPC = oldPC;
      // ovde treba da dodje poziv prekidne rutine!
      prekidnaRutina();
      return 0;
    }
    return 1;
  }else if(type == 2){
    //cout << "Odradjujem posao za adresiranje kod store/push instrukcija" << endl;
    if(nacinAdresiranja == 0){
      //cout << "Absolute/neposredno adr. za STR/PUSH je zabranjeno!" << endl;
      if(nacinAzuriranja != 0){
        cout << "GRESKA! Nacin azuriranja moze biti specificiran samo kod reg.ind. adresiranja" << mnemonik << "!" << endl;
        postojiGreska = 1;
        regPC = oldPC;
        // ovde treba da dodje poziv prekidne rutine!
        prekidnaRutina();
        return 0;
      }
      regPC = oldPC;
      //tu dodje obrada prekidne rutine
      prekidnaRutina();
      return 0;
    }else if(nacinAdresiranja == 1){
      //cout << "REGDIR adr." << endl;
      if(nacinAzuriranja != 0){
        cout << "GRESKA! Nacin azuriranja moze biti specificiran samo kod reg.ind. adresiranja" << mnemonik << "!" << endl;
        postojiGreska = 1;
        regPC = oldPC;
        // ovde treba da dodje poziv prekidne rutine!
        prekidnaRutina();
        return 0;
      }
      int provera = proveriIspravnostRegistra(reg2, reg2, "STR/PUSH");
      if(provera == 0){
        return 0;
      }else{
        upisiURegistar(reg2, zaAdresiranje);
      }
      return 1;
    }else if(nacinAdresiranja == 2){
      //cout << "REGIND adr." << endl;
      int provera = proveriIspravnostRegistra(reg2, reg2, "STR/PUSH");
      if(provera == 0){
        return 0;
      }else{
        if(nacinAzuriranja != 0 && nacinAzuriranja != 1){
          cout << "GRESKA! Zabranjen nacin azuriranja registra kod STR/PUSH instrukcije!" << endl;
          postojiGreska = 1;
          regPC = oldPC;
          // tu treba da dodje poziv prekidne rutine
          prekidnaRutina();
          return 0;
        }else if(nacinAzuriranja == 1){
          //regSP = regSP - 2;
          //nacin azuriranja = 1, umanjuje se za 2 pre formiranja adresa operanda
          umanjiSP();
        }
        upisiBajtoveUMemoriju(zaAdresiranje, vratiVrednostRegistra(reg2));
      }
      return 1;
    }else if(nacinAdresiranja == 3){
      //cout << "REGIND sa POMERAJEM adr." << endl;
      if(nacinAzuriranja != 0){
        cout << "GRESKA! Nacin azuriranja moze biti specificiran samo kod reg.ind. adresiranja" << mnemonik << "!" << endl;
        postojiGreska = 1;
        regPC = oldPC;
        // ovde treba da dodje poziv prekidne rutine!
        prekidnaRutina();
        return 0;
      }
      unsigned short zadnjaDvaBajta = dovuciDvaBajtaZaInstrukciju();
      int provera = proveriIspravnostRegistra(reg2, reg2, "STR/PUSH");
      if(provera == 0){
        return 0;
      }else{
        upisiBajtoveUMemoriju(zaAdresiranje, vratiVrednostRegistra(reg2) + zadnjaDvaBajta);
      }
      return 1;
    }else if(nacinAdresiranja == 4){
      //cout << "MEMDIR adr." << endl;
      if(nacinAzuriranja != 0){
        cout << "GRESKA! Nacin azuriranja moze biti specificiran samo kod reg.ind. adresiranja" << mnemonik << "!" << endl;
        postojiGreska = 1;
        regPC = oldPC;
        // ovde treba da dodje poziv prekidne rutine!
        prekidnaRutina();
        return 0;
      }
      unsigned short zadnjaDvaBajta = dovuciDvaBajtaZaInstrukciju();
      upisiBajtoveUMemoriju(zaAdresiranje, zadnjaDvaBajta);
      return 1;
    }else if(nacinAdresiranja == 5){
      // proveri da li ovo uopste treba da se odradi, ili pak treba da se baca greska i skace na prek. rutinu
      // Ma ovde nikada necu ni uci, jer kao i kod load instrukcija, i kod str je za pcrel nizih 4b 3.bajta jedanko 3, a ne 5 kao kod instr. skokova
      //cout << "PC REL. adr." << endl;
      return 1;
    }else{
      cout << "GRESKA! Za instrukciju: STORE/PUSH ne postoji taj nacin azuriranja!" << endl;
      postojiGreska = 1;
      regPC = oldPC;
      // ovde treba da dodje poziv prekidne rutine!
      prekidnaRutina();
      return 0;
    }
    return 1;
  }else{
    return 0;
  }
}

int Emulator::odradiPosaoZaLoadStoreAdresiranja(unsigned char reg1, unsigned char reg2, unsigned char nacinAdres, unsigned char nacinAzur, string mnemonik){
  //cout << "Odradjujem posao za adresiranje kod load/store instrukcija" << endl;
  if(nacinAdres == 0){
    //cout << "Absolute/neposredno adr." << endl;
    zaAdresiranje = dovuciDvaBajtaZaInstrukciju();
    //cout << +zaAdresiranje << endl;
    return 1;
  }else if(nacinAdres == 1){
    //cout << "REGDIR adr." << endl;
    int ispravno = proveriIspravnostRegistra(reg1, reg2, mnemonik);
    if(ispravno){
      odradiPosaoOkoRegistra(reg1, reg2);
      return 1;
    }else{
      return 0;
    }
  }else if(nacinAdres == 2){
    //cout << "REGIND adr." << endl;
    int ispravno = proveriIspravnostRegistra(reg1, reg2, mnemonik);
    if(ispravno){
      odradiPosaoOkoRegistra(reg1, reg2);
      //cout << +zaAdresiranje << endl;
      if(nacinAzur != 0 && nacinAzur != 4){
        cout << "Zabranjen nacin azuriranja registra kod: " << mnemonik << endl;
        regPC = oldPC;
        postojiGreska = 1;
        // tu treba da dodje obrada prekidne rutine
        prekidnaRutina();
        return 0;
      }else if(nacinAzur == 4){
        //regSP = regSP + 2;
        uvecajSP();
      }
      unsigned char niziBajt = memorija[zaAdresiranje];
      unsigned char visiBajt = memorija[zaAdresiranje + 1];
      zaAdresiranje = visiBajt*256 + niziBajt;
      return 1;
    }else{
      return 0;
    }
  }else if(nacinAdres == 3){
    //cout << "REGIND sa POMERAJEM adr." << endl;
    unsigned short zadnjaDvaBajta = dovuciDvaBajtaZaInstrukciju(); // to je payload iliti pomeraj
    int ispravno = proveriIspravnostRegistra(reg1, reg2, mnemonik);
    if(ispravno){
      odradiPosaoOkoRegistra(reg1, reg2);
      //cout << +zaAdresiranje << endl;
      unsigned char niziBajt = memorija[zaAdresiranje + zadnjaDvaBajta];
      unsigned char visiBajt = memorija[zaAdresiranje + zadnjaDvaBajta + 1];
      zaAdresiranje = visiBajt*256 + niziBajt;
      //cout << +zaAdresiranje << endl;
      return 1;
    }else{
      return 0;
    }
  }else if(nacinAdres == 4){
    //cout << "MEMDIR adr." << endl;
    unsigned short instPayload = dovuciDvaBajtaZaInstrukciju();
    unsigned char niziBajt = memorija[instPayload];
    unsigned char visiBajt = memorija[instPayload + 1];
    zaAdresiranje = visiBajt*256 + niziBajt;
    //cout << +visiBajt << "_" << +niziBajt << "_" << +zaAdresiranje << endl;
    return 1;
  }else if(nacinAdres == 5){
    // proveri da li ovo uopste treba
    //cout << "PC REL. adr." << endl;
    unsigned short zadnjaDvaBajtaInstr = dovuciDvaBajtaZaInstrukciju();
    int ispravno = proveriIspravnostRegistra(reg1, reg2, mnemonik);
    if(ispravno){
      odradiPosaoOkoRegistra(reg1, reg2);
      //return 1;
    }else{
      return 0;
    }
    zaAdresiranje += zadnjaDvaBajtaInstr;
    return 1;
  }else{
    cout << "GRESKA! Za instrukciju: " << mnemonik << " ne postoji taj nacin azuriranja!" << endl;
    postojiGreska = 1;
    regPC = oldPC;
    // ovde treba da dodje poziv prekidne rutine!
    prekidnaRutina();
    return 0;
  }
  return 1;
}

int Emulator::odradiStoreAdresiranje(unsigned char reg1, unsigned char reg2, unsigned char nacinAdres, unsigned char nacinAzur){
  //cout << "Odradjujem posao za adresiranje kod load/store instrukcija" << endl;
  if(nacinAdres == 0){
    //cout << "Absolute/neposredno adr. za STR/PUSH je zabranjeno!" << endl;
    regPC = oldPC;
    //tu dodje obrada prekidne rutine
    prekidnaRutina();
    return 0;
  }else if(nacinAdres == 1){
    //cout << "REGDIR adr." << endl;
    int provera = proveriIspravnostRegistra(reg2, reg2, "STR/PUSH");
    if(provera == 0){
      return 0;
    }else{
      upisiURegistar(reg2, zaAdresiranje);
    }
    return 1;
  }else if(nacinAdres == 2){
    //cout << "REGIND adr." << endl;
    int provera = proveriIspravnostRegistra(reg2, reg2, "STR/PUSH");
    if(provera == 0){
      return 0;
    }else{
      if(nacinAzur != 0 && nacinAzur != 1){
        cout << "GRESKA! Zabranjen nacin azuriranja registra kod STR/PUSH instrukcije!" << endl;
        postojiGreska = 1;
        regPC = oldPC;
        // tu treba da dodje poziv prekidne rutine
        prekidnaRutina();
        return 0;
      }else if(nacinAzur == 1){
        //regSP = regSP - 2;
        umanjiSP();
      }
      upisiBajtoveUMemoriju(zaAdresiranje, vratiVrednostRegistra(reg2));
    }
    return 1;
  }else if(nacinAdres == 3){
    //cout << "REGIND sa POMERAJEM adr." << endl;
    unsigned short zadnjaDvaBajta = dovuciDvaBajtaZaInstrukciju();
    int provera = proveriIspravnostRegistra(reg2, reg2, "STR/PUSH");
    if(provera == 0){
      return 0;
    }else{
      upisiBajtoveUMemoriju(zaAdresiranje, vratiVrednostRegistra(reg2) + zadnjaDvaBajta);
    }
    return 1;
  }else if(nacinAdres == 4){
    //cout << "MEMDIR adr." << endl;
    unsigned short zadnjaDvaBajta = dovuciDvaBajtaZaInstrukciju();
    upisiBajtoveUMemoriju(zaAdresiranje, zadnjaDvaBajta);
    return 1;
  }else if(nacinAdres == 5){
    // proveri da li ovo uopste treba da se odradi, ili pak treba da se baca greska i skace na prek. rutinu
    //cout << "PC REL. adr." << endl;
    return 1;
  }else{
    cout << "GRESKA! Za instrukciju: STORE/PUSH ne postoji taj nacin azuriranja!" << endl;
    postojiGreska = 1;
    regPC = oldPC;
    // ovde treba da dodje poziv prekidne rutine!
    prekidnaRutina();
    return 0;
  }
  return 1;
}

bool Emulator::checkZeroBit(){
  if(regPSW & 1){
    return true;
  }else{
    return false;
  }
}

bool Emulator::checkOverflowBit(){
  if(regPSW & 2){
    return true;
  }else{
    return false;
  }
}

bool Emulator::checkCarryBit(){
  if(regPSW & 4){
    return true;
  }else{
    return false;
  }
}

bool Emulator::checkNegativeBit(){
  if(regPSW & 8){
    return true;
  }else{
    return false;
  }
}

void Emulator::postaviZeroFlag(){
  regPSW |= 1; // samo postavi 1 kao najnizi bit
}

void Emulator::resetujZeroFlag(){
  regPSW &= ~1; // svi biti sem zadnjeg ostaju isti, radi se sa AND, a zadnji bit treba da postane 0
}

void Emulator::postaviNegativeFlag(){
  regPSW |= (1 << 3); // 16 = 1000
}

void Emulator::resetujNegativeFlag(){
  regPSW &= ~(1 << 3); // 16 = 0000 1000 => ~16 = 1111 0111
}

void Emulator::postaviCarryFlag(){
  regPSW |= (1 << 2); // 8 = 0100
}

void Emulator::resetujCarryFlag(){
  regPSW &= ~(1 << 2); // 8 = 0000 0100 => ~8 = 1111 1011
}

void Emulator::postaviOverflowFlag(){
  regPSW |= (1 << 1); // 2 = 0010
}

void Emulator::resetujOverflowFlag(){
  regPSW &= ~(1 << 1); // 2 = 0000 0010 => ~2 = 1111 1101
}

void Emulator::azurirajFlegove(unsigned short vrednost, string mnemonic){
  if(!mnemonic.compare("TEST")){
    //cout << "Azuriranje PSW za TEST" << endl;
    if(vrednost == 0){
      postaviZeroFlag();
    }else{
      resetujZeroFlag();
    }
    if((short)vrednost < 0){ //Da bi se postavio zero flag na 1 za -2 & -2, bez (short) bi ostajalo 0
      postaviNegativeFlag();
    }else{
      resetujNegativeFlag();
    }
  }else if(!mnemonic.compare("SHL")){
    //cout << "Azuriranje PSW za SHL" << endl;
    //cout << +(short)vrednost << endl;
    if(vrednost == 0){
      postaviZeroFlag();
    }else{
      resetujZeroFlag();
    }
    if((short)vrednost < 0){
      postaviNegativeFlag();
    }else{
      resetujNegativeFlag();
    }
    // Carry flag moram da setujem nazad u samoj obradi, jer nisam prosledjivao ovde i value od reg1 i reg2 :/
  }else if(!mnemonic.compare("SHR")){
    //cout << "Azuriranje PSW za SHR" << endl;
    //cout << +(short)vrednost << endl;
    if(vrednost == 0){
      postaviZeroFlag();
    }else{
      resetujZeroFlag();
    }
    if((short)vrednost < 0){
      postaviNegativeFlag();
    }else{
      resetujNegativeFlag();
    }
    // Carry flag moram da setujem nazad u samoj obradi, jer nisam prosledjivao ovde i value od reg1 i reg2 :/
  }else if(!mnemonic.compare("CMP")){
    //cout << "Azuriranje PSW za CMP" << endl;
    //cout << +(short)vrednost << endl;
    if(vrednost == 0){
      postaviZeroFlag();
    }else{
      resetujZeroFlag();
    }
    if((short)vrednost < 0){
      postaviNegativeFlag();
    }else{
      resetujNegativeFlag();
    }
    // Carry flag i overflow flag moram da setujem nazad u samoj obradi, jer nisam prosledjivao ovde i value od reg1 i reg2 :/
  }
}

void Emulator::obradi(){
  //cout << "Ucitavanje u memoriju zavrseno! Zapocinjem posao!" << endl;
  //cout << +memorija[4] << endl;
  regPC = dovuciDvaBajta();
  //cout << "PC: " << regPC << endl;
  unsigned char nextFetchedByte;
  while(true){
    //cout << +registriOpsteNamene[0] << "," << +registriOpsteNamene[1] << "," << +registriOpsteNamene[2] << ","+registriOpsteNamene[3] << "," << +registriOpsteNamene[4] << "," << +registriOpsteNamene[5] << "," << +regSP << "," << +regPC << "," << +regPSW << endl;
    



    /*
    // ************** Ovo ce mozda trebati da se zakomentarise !!! ************
    if(postojiGreska){
      //cout << "Postoji greska u emulatoru!" << endl;
      break; // Proveriti da li treba da se radi sa exit(-1);
    }
    // ************* Do ovde **************************************************
    */



    if(nasaoHalt){
      //cout << "Nasao sam HALT instrukciju" << endl;
      break;
    }
    oldPC = regPC;
    nextFetchedByte = dovuciJedanBajt();
    /*if(postojiGreska){
      //cout << "Postoji greska u emulatoru!" << endl;
      break; // Proveriti da li treba da se radi sa exit(-1);
    }*/
    //cout << "Dohvaceni bajt u int: " << +nextFetchedByte << endl;
    if(nextFetchedByte == 0){
      cout << "HALT INSTRUCTION" << endl;
      cout << "Program se zavrsava!" << endl;
      nasaoHalt = 1;
    }else if(nextFetchedByte == 16){
      //cout << "INT INSTRUCTION" << endl;
      unsigned char registriInstr = dovuciJedanBajt();
      //cout << "Registri: " << +registriInstr << endl;
      unsigned char reg1 = (registriInstr >> 4) & 0x0F;
      unsigned char reg2 = (registriInstr >> 0) & 0x0F;
      //cout << +reg1 << "_" << +reg2 << endl;
      int ispravnost = proveriIspravnostRegistra(reg2, reg1, "INT");
      // da li je potrebna provera da li je reg2 F, jer sam u asembleru tako radio da su nebitni bitovi stavljani na F
      int ispravnostObaveznog = proverObaveznogRegistraKojiJeF(reg2, "INT");
      if(ispravnostObaveznog == 0){
        continue;
      }
      if(ispravnost){
        unsigned short brojUlazaIVT = (vratiVrednostRegistra(reg1) % 8)*2;
        unsigned char visiBajt = memorija[brojUlazaIVT];
        unsigned char niziBajt = memorija[brojUlazaIVT + 1];
        //regSP = regSP - 2; // stek raste na dole, a treba da gurnem pc i psw
        umanjiSP();
        upisiBajtoveUMemoriju(regPC, regSP);
        //regSP = regSP - 2;
        umanjiSP();
        upisiBajtoveUMemoriju(regPSW, regSP);
        regPC = 256*niziBajt + visiBajt;
        //cout << +regSP << endl;
        //cout << +regPC << endl;
        //cout << +regPSW << endl;
      }else{
        continue;
      }
      //cout << "PC: " << +regPC << endl;
      //cout << "SP: " << +regSP << endl;
      //cout << "PSW: " << +regPSW << endl;
    }else if(nextFetchedByte == 32){
      //cout << "IRET INSTRUCTION" << endl;
      unsigned char niziBajt;
      unsigned char visiBajt;
      //cout << +regSP << endl;
      niziBajt = memorija[regSP];
      visiBajt = memorija[regSP + 1];
      regPSW = visiBajt*256 + niziBajt;
      //regSP += 2;
      uvecajSP();
      //cout << +regPSW << endl;
      //cout << +regSP << endl;
      niziBajt = memorija[regSP];
      visiBajt = memorija[regSP + 1];
      regPC = visiBajt*256 + niziBajt;
      //cout << +regPC << endl;
      //regSP += 2;
      uvecajSP();
    }else if(nextFetchedByte == 48){
      //cout << "CALL INSTRUCTION" << endl;
      unsigned char registriSkoka = dovuciJedanBajt();
      //cout << "Registri: " << +registriSkoka << endl;
      unsigned char reg1 = (registriSkoka >> 4) & 0x0F;
      unsigned char reg2 = (registriSkoka >> 0) & 0x0F;
      //cout << +reg1 << "_" << +reg2 << endl;
      unsigned char adresiranjeAzuriranje = dovuciJedanBajt();
      unsigned char nacinAzuriranja = (adresiranjeAzuriranje >> 4) & 0x0F;
      unsigned char nacinAdresiranja = (adresiranjeAzuriranje >> 0) & 0x0F;
      //cout << "Nacin azuriranja, adresiranje: " << +adresiranjeAzuriranje << endl;
      //cout << +nacinAzuriranja << "_" << +nacinAdresiranja << endl;
      // da li je potrebna provera da li je reg1 F, jer sam u asembleru tako radio da su nebitni bitovi stavljani na F
      int ispravanObavezni = proverObaveznogRegistraKojiJeF(reg1, "CALL");
      if(ispravanObavezni == 0){
        continue;
      }
      int ispravnoDaNe = odradiPosaoOkoAdresiranja(reg1, reg2, nacinAdresiranja, nacinAzuriranja, "CALL", 0);
      if(ispravnoDaNe){
        //regSP = regSP - 2;
        umanjiSP();
        upisiBajtoveUMemoriju(regPC, regSP);
        regPC = zaAdresiranje;
      }else{
        continue;
      }
      //cout << +zaAdresiranje << endl;
      //break; // Ova linija treba da se zakomentarise, sluzi mi samo dok testiram
    }else if(nextFetchedByte == 64){
      //cout << "RET INSTRUCTION" << endl;
      unsigned char niziBajt;
      unsigned char visiBajt;
      niziBajt = memorija[regSP];
      visiBajt = memorija[regSP + 1];
      regPC = visiBajt*256 + niziBajt;
      //regSP += 2;
      uvecajSP();
      //cout << +regPC << endl;
      //cout << +regSP << endl;
    }else if(nextFetchedByte == 96){
      //cout << "XCHG INSTRUCTION" << endl;
      unsigned char registriSkoka = dovuciJedanBajt();
      //cout << "Registri: " << +registriSkoka << endl;
      unsigned char reg1 = (registriSkoka >> 4) & 0x0F;
      unsigned char reg2 = (registriSkoka >> 0) & 0x0F;
      //cout << +reg1 << "_" << +reg2 << endl;
      int ispravnost = proveriIspravnostObaRegistra("XCHG", reg1, reg2);
      if(ispravnost == 1){
        unsigned short prviReg = vratiVrednostRegistra(reg1);
        unsigned short drugiReg = vratiVrednostRegistra(reg2);
        //cout << +prviReg << "_" << +drugiReg << endl;
        zameniVrednosti(prviReg, reg1, drugiReg, reg2);
      }else{
        continue;
      }
      //break; // Ova linija treba da se zakomentarise, sluzi mi samo dok testiram
    }else if(nextFetchedByte == 112){
      //cout << "ADD INSTRUCTION" << endl;
      unsigned char registriSkoka = dovuciJedanBajt();
      //cout << "Registri: " << +registriSkoka << endl;
      unsigned char reg1 = (registriSkoka >> 4) & 0x0F;
      unsigned char reg2 = (registriSkoka >> 0) & 0x0F;
      //cout << +reg1 << "_" << +reg2 << endl;
      int ispravnost = proveriIspravnostObaRegistra("ADD", reg1, reg2);
      if(ispravnost == 1){
        unsigned short prviReg = vratiVrednostRegistra(reg1);
        unsigned short drugiReg = vratiVrednostRegistra(reg2);
        //cout << +prviReg << "_" << +drugiReg << endl;
        //cout << +(prviReg + drugiReg) << endl;
        upisiURegistar(reg1, prviReg + drugiReg);
      }else{
        continue;
      }
      // Ovo sve lepo radi iako radim sa unsigned brojevima(samo pozitivnim brojevima)
      // npr ukoliko sabiram FFFF i 0005, dobicu 0004 jer se prebaci preko max vrednosti
      // koja moze da se smesta u unsigned short
      // da su trebali da se azuriraju psw flagovi, morao bih da da radim eksplicitni cast u (signed)
      // (signed) cast za unsigned short brojeve, samo ih predstavi kao negativne
      // celo ovo objasnjenje vazi i za sve ostale aritmeticke i logicke instrukcije koje ne rade sa PSW flagovima
      // kao sto su ADD, SUB, DIV, MUL, CMP, AND, OR, XOR
      //break; // Ova linija treba da se zakomentarise, sluzi mi samo dok testiram
    }else if(nextFetchedByte == 113){
      //cout << "SUB INSTRUCTION" << endl;
      unsigned char registriSkoka = dovuciJedanBajt();
      //cout << "Registri: " << +registriSkoka << endl;
      unsigned char reg1 = (registriSkoka >> 4) & 0x0F;
      unsigned char reg2 = (registriSkoka >> 0) & 0x0F;
      //cout << +reg1 << "_" << +reg2 << endl;
      int ispravnost = proveriIspravnostObaRegistra("SUB", reg1, reg2);
      if(ispravnost == 1){
        unsigned short prviReg = vratiVrednostRegistra(reg1);
        unsigned short drugiReg = vratiVrednostRegistra(reg2);
        //cout << +prviReg << "_" << +drugiReg << endl;
        //cout << +(prviReg - drugiReg) << endl;
        upisiURegistar(reg1, prviReg - drugiReg);
      }else{
        continue;
      }
      //break; // Ova linija treba da se zakomentarise, sluzi mi samo dok testiram
    }else if(nextFetchedByte == 114){
      //cout << "MUL INSTRUCTION" << endl;
      unsigned char registriSkoka = dovuciJedanBajt();
      //cout << "Registri: " << +registriSkoka << endl;
      unsigned char reg1 = (registriSkoka >> 4) & 0x0F;
      unsigned char reg2 = (registriSkoka >> 0) & 0x0F;
      //cout << +reg1 << "_" << +reg2 << endl;
      int ispravnost = proveriIspravnostObaRegistra("MUL", reg1, reg2);
      if(ispravnost == 1){
        unsigned short prviReg = vratiVrednostRegistra(reg1);
        unsigned short drugiReg = vratiVrednostRegistra(reg2);
        //cout << +prviReg << "_" << +drugiReg << endl;
        //cout << +(prviReg * drugiReg) << endl;
        upisiURegistar(reg1, prviReg * drugiReg);
      }else{
        continue;
      }
      //break; // Ova linija treba da se zakomentarise, sluzi mi samo dok testiram
    }else if(nextFetchedByte == 115){
      //cout << "DIV INSTRUCTION" << endl;
      unsigned char registriSkoka = dovuciJedanBajt();
      //cout << "Registri: " << +registriSkoka << endl;
      unsigned char reg1 = (registriSkoka >> 4) & 0x0F;
      unsigned char reg2 = (registriSkoka >> 0) & 0x0F;
      //cout << +reg1 << "_" << +reg2 << endl;
      int ispravnost = proveriIspravnostObaRegistra("DIV", reg1, reg2);
      if(ispravnost == 1){
        unsigned short prviReg = vratiVrednostRegistra(reg1);
        unsigned short drugiReg = vratiVrednostRegistra(reg2);
        //cout << +prviReg << "_" << +drugiReg << endl;
        //cout << +(prviReg / drugiReg) << endl;
        if(drugiReg == 0){
          cout << "GRESKA! Deljenje nulom!" << "DIV" << "!" << endl;
          postojiGreska = 1;
          regPC = oldPC;
          // ovde treba da dodje poziv prekidne rutine!
          prekidnaRutina();
          continue; 
        }
        upisiURegistar(reg1, prviReg / drugiReg);
      }else{
        continue;
      }
      //break; // Ova linija treba da se zakomentarise, sluzi mi samo dok testiram
    }else if(nextFetchedByte == 80){
      //cout << "JMP INSTRUCTION" << endl;
      unsigned char registriSkoka = dovuciJedanBajt();
      //cout << "Registri: " << +registriSkoka << endl;
      unsigned char reg1 = (registriSkoka >> 4) & 0x0F;
      unsigned char reg2 = (registriSkoka >> 0) & 0x0F;
      //cout << +reg1 << "_" << +reg2 << endl;
      unsigned char adresiranjeAzuriranje = dovuciJedanBajt();
      unsigned char nacinAzuriranja = (adresiranjeAzuriranje >> 4) & 0x0F;
      unsigned char nacinAdresiranja = (adresiranjeAzuriranje >> 0) & 0x0F;
      //cout << "Nacin azuriranja, adresiranje: " << +adresiranjeAzuriranje << endl;
      //cout << +nacinAzuriranja << "_" << +nacinAdresiranja << endl;
      // da li je potrebna provera da li je reg1 F, jer sam u asembleru tako radio da su nebitni bitovi stavljani na F
      int ispravanObavezni = proverObaveznogRegistraKojiJeF(reg1, "JMP");
      if(ispravanObavezni == 0){
        continue;
      }
      int ispravnoDaNe = odradiPosaoOkoAdresiranja(reg1, reg2, nacinAdresiranja, nacinAzuriranja, "JMP", 0);
      if(ispravnoDaNe){
        regPC = zaAdresiranje;
      }else{
        continue;
      }
      //cout << +zaAdresiranje << endl;
      //break; // Ova linija treba da se zakomentarise, sluzi mi samo dok testiram
    }else if(nextFetchedByte == 81){
      //cout << "JEQ INSTRUCTION" << endl;
      unsigned char registriSkoka = dovuciJedanBajt();
      //cout << "Registri: " << +registriSkoka << endl;
      unsigned char reg1 = (registriSkoka >> 4) & 0x0F;
      unsigned char reg2 = (registriSkoka >> 0) & 0x0F;
      //cout << +reg1 << "_" << +reg2 << endl;
      unsigned char adresiranjeAzuriranje = dovuciJedanBajt();
      unsigned char nacinAzuriranja = (adresiranjeAzuriranje >> 4) & 0x0F;
      unsigned char nacinAdresiranja = (adresiranjeAzuriranje >> 0) & 0x0F;
      //cout << "Nacin azuriranja, adresiranje: " << +adresiranjeAzuriranje << endl;
      //cout << +nacinAzuriranja << "_" << +nacinAdresiranja << endl;
      // da li je potrebna provera da li je reg1 F, jer sam u asembleru tako radio da su nebitni bitovi stavljani na F
      int ispravanObavezni = proverObaveznogRegistraKojiJeF(reg1, "JEQ");
      if(ispravanObavezni == 0){
        continue;
      }
      int ispravnoDaNe = odradiPosaoOkoAdresiranja(reg1, reg2, nacinAdresiranja, nacinAzuriranja, "JEQ", 0);
      if(ispravnoDaNe){
        if(checkZeroBit()){
          regPC = zaAdresiranje;
        }
      }else{
        continue;
      }
      //cout << +zaAdresiranje << endl;
      /*break;*/
    }else if(nextFetchedByte == 82){
      //cout << "JNE INSTRUCTION" << endl;
      unsigned char registriSkoka = dovuciJedanBajt();
      //cout << "Registri: " << +registriSkoka << endl;
      unsigned char reg1 = (registriSkoka >> 4) & 0x0F;
      unsigned char reg2 = (registriSkoka >> 0) & 0x0F;
      //cout << +reg1 << "_" << +reg2 << endl;
      unsigned char adresiranjeAzuriranje = dovuciJedanBajt();
      unsigned char nacinAzuriranja = (adresiranjeAzuriranje >> 4) & 0x0F;
      unsigned char nacinAdresiranja = (adresiranjeAzuriranje >> 0) & 0x0F;
      //cout << "Nacin azuriranja, adresiranje: " << +adresiranjeAzuriranje << endl;
      //cout << +nacinAzuriranja << "_" << +nacinAdresiranja << endl;
      // da li je potrebna provera da li je reg1 F, jer sam u asembleru tako radio da su nebitni bitovi stavljani na F
      int ispravanObavezni = proverObaveznogRegistraKojiJeF(reg1, "JNE");
      if(ispravanObavezni == 0){
        continue;
      }
      int ispravnoDaNe = odradiPosaoOkoAdresiranja(reg1, reg2, nacinAdresiranja, nacinAzuriranja, "JNE", 0);
      if(ispravnoDaNe){
        if(!checkZeroBit()){
          regPC = zaAdresiranje;
        }
      }else{
        continue;
      }
    }else if(nextFetchedByte == 83){
      //cout << "JGT INSTRUCTION" << endl;
      unsigned char registriSkoka = dovuciJedanBajt();
      //cout << "Registri: " << +registriSkoka << endl;
      unsigned char reg1 = (registriSkoka >> 4) & 0x0F;
      unsigned char reg2 = (registriSkoka >> 0) & 0x0F;
      //cout << +reg1 << "_" << +reg2 << endl;
      unsigned char adresiranjeAzuriranje = dovuciJedanBajt();
      unsigned char nacinAzuriranja = (adresiranjeAzuriranje >> 4) & 0x0F;
      unsigned char nacinAdresiranja = (adresiranjeAzuriranje >> 0) & 0x0F;
      //cout << "Nacin azuriranja, adresiranje: " << +adresiranjeAzuriranje << endl;
      //cout << +nacinAzuriranja << "_" << +nacinAdresiranja << endl;
      // da li je potrebna provera da li je reg1 F, jer sam u asembleru tako radio da su nebitni bitovi stavljani na F
      int ispravanObavezni = proverObaveznogRegistraKojiJeF(reg1, "JGT");
      if(ispravanObavezni == 0){
        continue;
      }
      int ispravnoDaNe = odradiPosaoOkoAdresiranja(reg1, reg2, nacinAdresiranja, nacinAzuriranja, "JNE", 0);
      if(ispravnoDaNe){
        if(!(checkNegativeBit() ^ checkOverflowBit()) & !checkZeroBit()){//Uslov da li je greater than
          regPC = zaAdresiranje;
        }
      }else{
        continue;
      }
      //cout << +regPC << endl;
      //cout << +zaAdresiranje << endl;
      //break !!! zakomentarisati, sluzi samo za potrebe testiranja
    }else if(nextFetchedByte == 116){
      //cout << "CMP INSTRUCTION" << endl;
      unsigned char registriInstr = dovuciJedanBajt();
      //cout << "Registri: " << +registriInstr << endl;
      unsigned char reg1 = (registriInstr >> 4) & 0x0F;
      unsigned char reg2 = (registriInstr >> 0) & 0x0F;
      //cout << +reg1 << "_" << +reg2 << endl;
      int ispravnost = proveriIspravnostObaRegistra("CMP", reg1, reg2);
      if(ispravnost == 1){
        unsigned short prviReg = vratiVrednostRegistra(reg1);
        unsigned short drugiReg = vratiVrednostRegistra(reg2);
        //cout << +prviReg << "_" << +drugiReg << endl;
        unsigned short temporaryReg = prviReg - drugiReg;
        //cout << +temporaryReg << endl;
        azurirajFlegove(temporaryReg, "CMP");
        // za CMP se radi SUB, pa je za detekciju carry bit dovoljno videte da li je prvi reg manji od drugog reg
        if(prviReg < drugiReg){
          postaviCarryFlag();
        }else{
          resetujCarryFlag();
        }
        // proveri za uslov za overflow flag da li treba da se dodaju jos neki slucajevi
		// Nekad mi je bilo ovako:
		// ((short)prviReg > 0 && (short)drugiReg > 0 && (short)temporaryReg < 0) || ((short)prviReg < 0 && (short)drugiReg < 0 && (short)temporaryReg > 0)
        if(((short)prviReg > 0 && (short)drugiReg < 0 && (short)temporaryReg < 0) || ((short)prviReg < 0 && (short)drugiReg > 0 && (short)temporaryReg > 0)){
          // Da li mozda da dodam i ovo:
          // (short)prviReg > 0 && (short)drugiReg < 0 && (short)temporaryReg < 0
          // (short)prviReg < 0 && (short)drugiReg > 0 && (short)temporaryReg > 0
          // Proveriti ovo dobro!!!!????!!!!
          postaviOverflowFlag();
        }else{
          resetujOverflowFlag();
        }
      }else{
        continue;
      }
      //break;
    }else if(nextFetchedByte == 128){
      //cout << "NOT INSTRUCTION" << endl;
      unsigned char registriSkoka = dovuciJedanBajt();
      //cout << "Registri: " << +registriSkoka << endl;
      unsigned char reg1 = (registriSkoka >> 4) & 0x0F;
      unsigned char reg2 = (registriSkoka >> 0) & 0x0F;
      //cout << +reg1 << "_" << +reg2 << endl;
      // da li je potrebna provera da li je reg2 F, jer sam u asembleru tako radio da su nebitni bitovi stavljani na F
      int ispravanObavezni = proverObaveznogRegistraKojiJeF(reg2, "NOT");
      if(ispravanObavezni == 0){
        continue;
      }
      int ispravnost = proveriIspravnostRegistra(reg2, reg1, "NOT");
      if(ispravnost == 1){
        unsigned short prviReg = vratiVrednostRegistra(reg1);
        //cout << +prviReg << endl;
        //cout << +(~prviReg) << endl;
        upisiURegistar(reg1, ~prviReg);
      }else{
        continue;
      }
      //break; // Ova linija treba da se zakomentarise, sluzi mi samo dok testiram
    }else if(nextFetchedByte == 129){
      //cout << "AND INSTRUCTION" << endl;
      unsigned char registriSkoka = dovuciJedanBajt();
      //cout << "Registri: " << +registriSkoka << endl;
      unsigned char reg1 = (registriSkoka >> 4) & 0x0F;
      unsigned char reg2 = (registriSkoka >> 0) & 0x0F;
      //cout << +reg1 << "_" << +reg2 << endl;
      int ispravnost = proveriIspravnostObaRegistra("AND", reg1, reg2);
      if(ispravnost == 1){
        unsigned short prviReg = vratiVrednostRegistra(reg1);
        unsigned short drugiReg = vratiVrednostRegistra(reg2);
        //cout << +prviReg << "_" << +drugiReg << endl;
        //cout << +(prviReg & drugiReg) << endl;
        upisiURegistar(reg1, prviReg & drugiReg);
      }else{
        continue;
      }
      //break; // Ova linija treba da se zakomentarise, sluzi mi samo dok testiram
    }else if(nextFetchedByte == 130){
      //cout << "OR INSTRUCTION" << endl;
      unsigned char registriSkoka = dovuciJedanBajt();
      //cout << "Registri: " << +registriSkoka << endl;
      unsigned char reg1 = (registriSkoka >> 4) & 0x0F;
      unsigned char reg2 = (registriSkoka >> 0) & 0x0F;
      //cout << +reg1 << "_" << +reg2 << endl;
      int ispravnost = proveriIspravnostObaRegistra("OR", reg1, reg2);
      if(ispravnost == 1){
        unsigned short prviReg = vratiVrednostRegistra(reg1);
        unsigned short drugiReg = vratiVrednostRegistra(reg2);
        //cout << +prviReg << "_" << +drugiReg << endl;
        //cout << +(prviReg | drugiReg) << endl;
        upisiURegistar(reg1, prviReg | drugiReg);
      }else{
        continue;
      }
      //break; // Ova linija treba da se zakomentarise, sluzi mi samo dok testiram
    }else if(nextFetchedByte == 131){
      //cout << "XOR INSTRUCTION" << endl;
      unsigned char registriSkoka = dovuciJedanBajt();
      //cout << "Registri: " << +registriSkoka << endl;
      unsigned char reg1 = (registriSkoka >> 4) & 0x0F;
      unsigned char reg2 = (registriSkoka >> 0) & 0x0F;
      //cout << +reg1 << "_" << +reg2 << endl;
      int ispravnost = proveriIspravnostObaRegistra("XOR", reg1, reg2);
      if(ispravnost == 1){
        unsigned short prviReg = vratiVrednostRegistra(reg1);
        unsigned short drugiReg = vratiVrednostRegistra(reg2);
        //cout << +prviReg << "_" << +drugiReg << endl;
        //cout << +(prviReg ^ drugiReg) << endl;
        upisiURegistar(reg1, prviReg ^ drugiReg);
      }else{
        continue;
      }
      //break; // Ova linija treba da se zakomentarise, sluzi mi samo dok testiram
    }else if(nextFetchedByte == 132){
      //cout << "TEST INSTRUCTION" << endl;
      unsigned char registriSkoka = dovuciJedanBajt();
      //cout << "Registri: " << +registriSkoka << endl;
      unsigned char reg1 = (registriSkoka >> 4) & 0x0F;
      unsigned char reg2 = (registriSkoka >> 0) & 0x0F;
      //cout << +reg1 << "_" << +reg2 << endl;
      int ispravnost = proveriIspravnostObaRegistra("TEST", reg1, reg2);
      if(ispravnost == 1){
        unsigned short prviReg = vratiVrednostRegistra(reg1);
        unsigned short drugiReg = vratiVrednostRegistra(reg2);
        //cout << +prviReg << "_" << +drugiReg << endl;
        unsigned short temporaryReg = prviReg & drugiReg;
        //cout << +temporaryReg << endl;
        azurirajFlegove(temporaryReg, "TEST");
      }else{
        continue;
      }
      //break; // Ova linija treba da se zakomentarise, sluzi mi samo dok testiram
    }else if(nextFetchedByte == 144){
      // tu stavljam carry bit na 1 ako zadnji bit koji siftovanjem u levo ispadne, bio 1
      //cout << "SHL INSTRUCTION" << endl;
      unsigned char registriInstr = dovuciJedanBajt();
      //cout << "Registri: " << +registriInstr << endl;
      unsigned char reg1 = (registriInstr >> 4) & 0x0F;
      unsigned char reg2 = (registriInstr >> 0) & 0x0F;
      //cout << +reg1 << "_" << +reg2 << endl;
      int ispravnost = proveriIspravnostObaRegistra("SHL", reg1, reg2);
      if(ispravnost == 1){
        unsigned short prviReg = vratiVrednostRegistra(reg1);
        unsigned short drugiReg = vratiVrednostRegistra(reg2);
        //cout << +prviReg << "_" << +drugiReg << endl;
        unsigned short temporaryReg = prviReg << drugiReg;
        //cout << +temporaryReg << endl;
        short prviRegistar = (short)prviReg;
        short drugiRegistar = (short)drugiReg;
        //cout << +prviRegistar << "___" << +drugiRegistar << endl;
        upisiURegistar(reg1, temporaryReg);
        azurirajFlegove(temporaryReg, "SHL");
        // carry moram ovde da azuriram, jer sam zaboravio da u metodi azurirajFlegove posaljem i prviReg i drugiReg
        // u toj metodi mogu samo negative i zero flag da azuriram :///
        // a mrzi me da prepravljam metode
        if((prviReg >> ((16 - drugiReg)) & 1) && drugiReg < 16){
          postaviCarryFlag();
        }else{
          resetujCarryFlag();
        }
      }else{
        continue;
      }
      //break; // Ova linija treba da se zakomentarise, sluzi mi samo dok testiram
    }else if(nextFetchedByte == 145){
      // tu stavljam carry bit na 1 ako zadnji bit koji ispadne udesno pri siftovanju je bio 1
      //cout << "SHR INSTRUCTION" << endl;
      unsigned char registriInstr = dovuciJedanBajt();
      //cout << "Registri: " << +registriInstr << endl;
      unsigned char reg1 = (registriInstr >> 4) & 0x0F;
      unsigned char reg2 = (registriInstr >> 0) & 0x0F;
      //cout << +reg1 << "_" << +reg2 << endl;
      int ispravnost = proveriIspravnostObaRegistra("SHR", reg1, reg2);
      if(ispravnost == 1){
        unsigned short prviReg = vratiVrednostRegistra(reg1);
        unsigned short drugiReg = vratiVrednostRegistra(reg2);
        //cout << +prviReg << "_" << +drugiReg << endl;
        unsigned short temporaryReg = prviReg >> drugiReg;
        //cout << +temporaryReg << endl;
        short prviRegistar = (short)prviReg;
        short drugiRegistar = (short)drugiReg;
        //cout << +prviRegistar << "___" << +drugiRegistar << endl;
        upisiURegistar(reg1, temporaryReg);
        azurirajFlegove(temporaryReg, "SHR");
        // carry moram ovde da azuriram, jer sam zaboravio da u metodi azurirajFlegove posaljem i prviReg i drugiReg
        // u toj metodi mogu samo negative i zero flag da azuriram :///
        // a mrzi me da prepravljam metode
        if(prviReg >> ((drugiReg - 1)) & 1){
          postaviCarryFlag();
        }else{
          resetujCarryFlag();
        }
      }else{
        continue;
      }
      //break; // Ova linija treba da se zakomentarise, sluzi mi samo dok testiram
    }else if(nextFetchedByte == 160){
      //cout << "LDR ili POP INSTRUCTION" << endl;
      unsigned char registriInstr = dovuciJedanBajt();
      //cout << "Registri: " << +registriInstr << endl;
      unsigned char reg1 = (registriInstr >> 4) & 0x0F;
      unsigned char reg2 = (registriInstr >> 0) & 0x0F;
      //cout << +reg1 << "_" << +reg2 << endl;
      unsigned char adresiranjeAzuriranje = dovuciJedanBajt();
      unsigned char nacinAzuriranja = (adresiranjeAzuriranje >> 4) & 0x0F;
      unsigned char nacinAdresiranja = (adresiranjeAzuriranje >> 0) & 0x0F;
      //cout << "Nacin azuriranja, adresiranje: " << +adresiranjeAzuriranje << endl;
      //cout << +nacinAzuriranja << "_" << +nacinAdresiranja << endl;
      //int ispravnost = odradiPosaoZaLoadStoreAdresiranja(reg1, reg2, nacinAdresiranja, nacinAzuriranja, "LDR_POP");
      // sve ovo mogu da strpam u jednu veliku metodu, koja ce da radi i za skokove, i za ldr, i za str
      int ispravnost = odradiPosaoOkoAdresiranja(reg1, reg2, nacinAdresiranja, nacinAzuriranja, "LDR_POP", 1);
      if(ispravnost){
        int ispravnostOdredista = proveriIspravnostRegistra(reg2, reg1, "LDR_POP");
        if(ispravnostOdredista){
          upisiURegistar(reg1, zaAdresiranje);
          //cout << "r" << +reg1 << "=0x" << +zaAdresiranje << endl; 
        }else{
          continue;
        }
      }else{
        continue;
      }
      //break;
    }else if(nextFetchedByte == 176){
      //cout << "STR ili PUSH INSTRUCTION" << endl;
      unsigned char registriInstr = dovuciJedanBajt();
      //cout << "Registri: " << +registriInstr << endl;
      unsigned char reg1 = (registriInstr >> 4) & 0x0F;
      unsigned char reg2 = (registriInstr >> 0) & 0x0F;
      //cout << +reg1 << "_" << +reg2 << endl;
      int ispravnostOdredista = proveriIspravnostRegistra(reg2, reg1, "STR_PUSH");
      if(ispravnostOdredista == 0){
        continue;
      }
      odradiPosaoOkoRegistra(reg1, reg1);
      unsigned char adresiranjeAzuriranje = dovuciJedanBajt();
      unsigned char nacinAzuriranja = (adresiranjeAzuriranje >> 4) & 0x0F;
      unsigned char nacinAdresiranja = (adresiranjeAzuriranje >> 0) & 0x0F;
      //cout << "Nacin azuriranja, adresiranje: " << +adresiranjeAzuriranje << endl;
      //cout << +nacinAzuriranja << "_" << +nacinAdresiranja << endl;
      //int ispravnost = odradiStoreAdresiranje(reg1, reg2, nacinAdresiranja, nacinAzuriranja);
      // sve ovo mogu da strpam u jednu veliku metodu, koja ce da radi i za skokove, i za ldr, i za str
      int ispravnost = odradiPosaoOkoAdresiranja(reg1, reg2, nacinAdresiranja, nacinAzuriranja, "STR_PUSH", 2);
      if(ispravnost == 0){
        continue;
      }
      // tako je, ako nije bilo problema sa registrima, metoda ce da zavrsi sav posao pa ce se u promenljivoj zaAdresiranje
      // naci sracunata vrednost
      //break;
    }else{
      cout << "GRESKA! Emulator ne zna o kojoj instrukciji se radi!" << endl;
      //proveri da li ovde treba da se radi poziv prekidne rutine
      // ili treba da se promenljiva postojiGreska postavlja na 1
      // ili da nasilno gasim petlju iskakanjem sa break
      // ili da nasilno gasim ceo program sa exit(-1) pa da nemam ispis ni registara na kraju
      
      //******************************
      postojiGreska = 1;
      regPC = oldPC;
      // ovde treba da dodje poziv prekidne rutine!
      prekidnaRutina();
      //return 0;
      //******************************
    }
    //break;
  }

  /*
  Ukoliko je doslo do greske, bit I bice postavljen na 1 pa i ne mora da ga proveravam!
  */

  cout << "Emulacija zavrsena!" << endl;
  if(postojiGreska == 1){
    cout << "*** Postoji GRESKA u emulatoru! ***" << endl;
  }
  if(nasaoHalt == 1){
    cout << "*** Zavrseno HALT instrukcijom! ***" << endl;
  }
  cout << "---------------------------------------------------------" << endl;
  cout << "Emulated processor executed halt instruction!" << endl;
  //cout << "Processore state before emulation: psw=0b" << bitset<16>(regPSW) << endl; // https://www.geeksforgeeks.org/binary-representation-of-a-given-number/
  cout << "Emulated processor state: psw=0b" << toBinaryString(regPSW) << endl;
  //cout << "Processore state before emulation: psw=0b" << pretvoriPSWuBite(regPSW) << endl;
  for(int i = 0; i < 6; i++){
    if((i%4 == 0) && i != 0){
      cout << endl << "r" << i << "=0x" << pretvoriUOblikZaIspis(registriOpsteNamene[i]) << "\t";
    }else if(i == 0){
      cout << "r" << i << "=0x" << pretvoriUOblikZaIspis(registriOpsteNamene[i]) << "\t";
    }else{
      cout << "r" << i << "=0x" << pretvoriUOblikZaIspis(registriOpsteNamene[i]) << "\t";
    }
  }
  cout << "r" << 6 << "=0x" << pretvoriUOblikZaIspis(regSP) << "\t";
  cout << "r" << 7 << "=0x" << pretvoriUOblikZaIspis(regPC) << "\t";
  cout << endl;
}

void Emulator::emuliraj(){
  /*cout << "Emulacija zapoceta!" << endl;
  cout << "-------------------------------------------------------------" << endl;
  cout << "Registers before emulation" << endl;
  //cout << "Processore state before emulation: psw=0b" << bitset<16>(regPSW) << endl; // https://www.geeksforgeeks.org/binary-representation-of-a-given-number/
  cout << "Processore state before emulation: psw=0b" << toBinaryString(regPSW) << endl;
  //cout << "Processore state before emulation: psw=0b" << pretvoriPSWuBite(regPSW) << endl;
  for(int i = 0; i < 6; i++){
    if((i%4 == 0) && i != 0){
      cout << endl << "r" << i << "=0x" << pretvoriUOblikZaIspis(registriOpsteNamene[i]) << "\t";
    }else if(i == 0){
      cout << "r" << i << "=0x" << pretvoriUOblikZaIspis(registriOpsteNamene[i]) << "\t";
    }else{
      cout << "r" << i << "=0x" << pretvoriUOblikZaIspis(registriOpsteNamene[i]) << "\t";
    }
  }
  cout << "r" << 6 << "=0x" << pretvoriUOblikZaIspis(regSP) << "\t";
  cout << "r" << 7 << "=0x" << pretvoriUOblikZaIspis(regPC) << "\t";
  cout << endl;

  cout << endl;*/

  ulazniFajl.open(inputFileName);
  int brojNepraznihLinija = 0;
  if(ulazniFajl.is_open()){
    //cout << "Sve je u redu sa otvaranjem: " << inputFileName << endl;
    //inicijalizujEmulator();
    string currentLine = "";
    string offsetLinije = "";
    string ostatakLinije = "";
    int lineOffset;
    while(getline(ulazniFajl, currentLine)){
      brojNepraznihLinija++;
      //cout << currentLine << endl;
      offsetLinije = currentLine.substr(0, 4);
      lineOffset = pretvoriAdresuUInt(offsetLinije);
      ostatakLinije = currentLine.substr(6, -1);
      //cout << "Adresa: " << lineOffset << ", " << ostatakLinije << endl;
      //cout << lineOffset << endl;
      const char delimiter = ' ';
      vector<string> bajtoviLinije;
      tokenize(ostatakLinije, delimiter, bajtoviLinije);
      //cout << bajtoviLinije.size() << endl;
      int br = 0;
      for(vector<string>::iterator it = bajtoviLinije.begin() ; it != bajtoviLinije.end() ;){
        if(br + lineOffset >= 65280){
          cout << "GRESKA! Zasli ste u prostor za memorijski mapirane registre!" << endl;
          postojiGreska = 1;
          exit(-1);
        }
        string simbol = (*it);
        int vrednost = pretvoriHexUInt(simbol);
        //cout << vrednost << ":";
        memorija[lineOffset + br] = vrednost;
        //cout << "mem[" << lineOffset + br << "]=" << +memorija[lineOffset + br] << endl;
        ++it;
        br++;
      }
      //cout << endl;
    }
    if(ulazniFajl.eof() && brojNepraznihLinija == 0){
      postojiGreska = 1;
      cout << "GRESKA! Fajl program.hex je prazan!" << endl;
      exit(-1);
    }
    ulazniFajl.close();
    obradi();
  }else{
    postojiGreska = 1;
    cout << "Greska prilikom otvaranja ulaznog fajla: " << inputFileName << endl;
    exit(-1);
  }
}
