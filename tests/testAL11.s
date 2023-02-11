.global c, d, e
.extern a, b
.section prvaSekcija
.skip 2
c:
.word 0xAC
jmp c
jmp %a
d:
.word 11
.section drugaSekcija
.skip 5
.word 0xC
.global d
e:
pop r4
add r3, r2
ldr r4, [r2 + b]
.end