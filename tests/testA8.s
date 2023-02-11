.section prvaSekcija
.global a
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
ldr r3, [r4 + 0x11]
ldr r3, [r1 + 11]
ldr r4, [r3 + a]
ldr r4, [r3 + b]
ldr r4, [r3 + c]
ldr r4, [r3 + l]
ldr r2, [r6]
.word b
.word c
jmp b
jmp %b
jmp c
ldr r4, [r2 + 0x11]
ldr r3, [r2 + a]
ldr r3, [r2 + c]
ldr r2, l
jmp %c
.global l
.end
