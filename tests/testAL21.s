.section prva
.global a
.skip 7
.word a
.skip 1
.word a
push r2
a:
pop r3
.section najnovija
.skip 7
jmp %a
.word 0x11
jmp %a
.end
