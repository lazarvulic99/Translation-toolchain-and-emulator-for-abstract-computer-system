g++ ./src/assembler.cpp ./src/mainAssembler.cpp -o asembler
g++ ./src/mainLinker.cpp ./src/linker.cpp -o linker
g++ ./src/emulator.cpp ./src/mainEmulator.cpp -o emulator

#ASSEMBLER=./asembler
#LINKER=./linker
#EMULATOR=./emulator
#${ASSEMBLER} -o testAL12.o testAL12.s
#${EMULATOR} program.hex
#rm *.o
#rm program.hex
#${ASSEMBLER} -o main.o main.s
#${ASSEMBLER} -o math.o math.s
#${ASSEMBLER} -o ivt.o ivt.s
#${ASSEMBLER} -o isr_reset.o isr_reset.s
#${ASSEMBLER} -o isr_terminal.o isr_terminal.s
#${ASSEMBLER} -o isr_timer.o isr_timer.s
#${ASSEMBLER} -o isr_user0.o isr_user0.s
#${LINKER} -hex -o program.hex ivt.o math.o main.o isr_reset.o isr_terminal.o isr_timer.o isr_user0.o
#${EMULATOR} program.hex
#${ASSEMBLER} -o testALE1.o testALE1.s
#${ASSEMBLER} -o testALE2.o testALE2.s
#${LINKER} -hex -o program.hex testALE1.o testALE2.o
#${EMULATOR} program.hex
#------------------------
#skripta za emu test
#EMULATOR=./emulator
#${EMULATOR} program.hex
#------------------------
#skripta za start asembler test
#ASSEMBLER=./asembler
#${ASSEMBLER} -o testAL12.o testAL12.s
#------------------------
#skripta za test sva 3
#${ASSEMBLER} -o testALE1.o testALE1.s
#${ASSEMBLER} -o testALE2.o testALE2.s
#${LINKER} -hex -o program.hex testALE1.o testALE2.o
#${EMULATOR} program.hex