.section provera
.word 0x15
d:
.global a, b
.extern c
a:
.skip 2
.word d
.word a
b:
.word c
jmp %b
jmp %c
.end