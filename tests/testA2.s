.section lakisa
.global ok, k
jmp ok
jmp %k
.word 5, ok
.skip 4
ok:
push r4
jmp %ok
k:

.section lakisa2
jeq %ok
.end
