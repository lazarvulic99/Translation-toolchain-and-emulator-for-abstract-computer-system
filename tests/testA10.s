.section prvaSekcija
.global a, b
.extern c, d
.word 0x12, -3, a
.skip 3
a:
.word 0xf
b:
.word c
.section drugaSekcija
ldr r3, r4
ldr r2, [r6]
.end
