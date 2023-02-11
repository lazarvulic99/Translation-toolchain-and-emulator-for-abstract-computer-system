.section prvaSekcija
.global a, b
.extern c, d
.word 0x12, -3, a
e:
.skip 3
a:
.word 0xf
b:
.word c
.skip 4
l:
jmp %e
jmp e
.section drugaSekcija
ldr r3, r4
ldr r2, [r6]
.word b
.word c
jmp b
jmp %b
jmp c
ldr r4, %l
ldr r2, l
jmp %c
.global l
.end
