#ifndef EMULATOR_HPP
#define EMULATOR_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;

class Emulator{
private:
  string inputFileName;
  ifstream ulazniFajl;
  int postojiGreska;
  int nasaoHalt;
  int pronadjenKraj;
  unsigned char memorija[65536] = {0}; // velicina memorijskog adresnog prostora iznosi 2^16 bajtova, radi se sa unsigned brojevima
  unsigned short registriOpsteNamene[6] = {0};
  unsigned short regPC = 0; // to je zapravo r7
  unsigned short regSP = 0; // to je zapravo r6
  unsigned short regPSW = 0; // to je zapravo r8
public:
  unsigned short oldPC;
  unsigned short zaAdresiranje;
  Emulator(string imeUlaznogFajla);
  void inicijalizujEmulator();
  void emuliraj();
  string pretvoriUOblikZaIspis(unsigned short registar);
  int pretvoriAdresuUInt(string adresa);
  int pretvoriHexUInt(string hex);
  string pretvoriPSWuBite(unsigned short registarZaIspis);
  void obradi();
  unsigned short dovuciDvaBajta();
  unsigned short dovuciDvaBajtaZaInstrukciju();
  unsigned char dovuciJedanBajt();
  void prekidnaRutina();
  void umanjiSP();
  void uvecajSP();
  void upisiBajtoveUMemoriju(unsigned short vrednostZaUpis, unsigned short adresaUpisa);
  void odradiPosaoOkoRegistra(unsigned char reg1, unsigned char reg2);
  int odradiPosaoOkoAdresiranja(unsigned char reg1, unsigned char reg2, unsigned char nacinAdresiranja, unsigned char nacinAzuriranja, string mnemonik, int type); // 0 - skokovi, 1 - load/pop, 2 - store/push
  int odradiPosaoZaLoadStoreAdresiranja(unsigned char reg1, unsigned char reg2, unsigned char nacinAdres, unsigned char nacinAzur, string mnemonik);
  int odradiStoreAdresiranje(unsigned char reg1, unsigned char reg2, unsigned char nacinAdres, unsigned char nacinAzur);
  int proveriIspravnostRegistra(unsigned char reg1, unsigned char reg2, string mnemonik);
  int proveriIspravnostObaRegistra(string mnemonik, unsigned char reg1, unsigned char reg2);
  int proverObaveznogRegistraKojiJeF(unsigned char reg, string mnemonik);
  unsigned short vratiVrednostRegistra(unsigned char registarMoj);
  void upisiURegistar(unsigned char reg1, unsigned short vrednost);
  void zameniVrednosti(unsigned short vrednost1, unsigned char reg1, unsigned short vrednost2, unsigned char reg2);
  void azurirajFlegove(unsigned short vrednost, string mnemonic);
  bool checkZeroBit();
  bool checkNegativeBit();
  bool checkOverflowBit();
  bool checkCarryBit();
  void postaviZeroFlag();
  void resetujZeroFlag();
  void postaviNegativeFlag();
  void resetujNegativeFlag();
  void postaviCarryFlag();
  void resetujCarryFlag();
  void postaviOverflowFlag();
  void resetujOverflowFlag();
  void postaviInterruptFlag();
  void resetujInterruptFlag();
  //Fja za konvert u bite, radi za razlicite tipove
  template<typename T>
  string toBinaryString(const T& registarZaIspis);
};

#endif
