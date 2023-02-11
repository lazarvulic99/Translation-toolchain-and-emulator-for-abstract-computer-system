.global a, b
.extern c
.section prvaSekcija
.skip 2
a:
.word 0xAC
jmp a
jmp %a
b:
.word 11
.section drugaSekcija
.skip 5
.word 0xC
.global l
l:
pop r4
add r3, r2
ldr r4, [r2 + c]
.end