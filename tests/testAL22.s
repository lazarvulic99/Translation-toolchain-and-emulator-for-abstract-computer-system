.section prva
.global b
.skip 7
.word b
d:
push r2
b:
pop r3
e:
.section druga
.word 0x12
f:
.skip 5
jmp b
jmp b
jmp d
jmp e
halo:
jmp f
jmp budala
.word 0x05
jmp halo
budala:
.end
