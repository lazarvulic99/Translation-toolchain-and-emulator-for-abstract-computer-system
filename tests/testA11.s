.section prva
.global b
.skip 7
.word b
push r2
b:
pop r3
.section druga
.word 0x12
.skip 5
jmp b
jmp b
.end
