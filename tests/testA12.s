.section test
.skip 2
.global b
a:
.word 0xFF
.skip 3
ldr r3, %b
ldr r3, c
ldr r3, b
ldr r3, 0x12
ldr r3, 11
ldr r3, a
ldr r3, %c
ldr r3, %a
jmp c
jmp a
jmp b
jmp *c
jmp *a
jmp *b
jmp *[r3 + c]
jmp *[r3 + b]
jmp *[r3 + a]
.word 0x02
c:
.skip 7
.section nova
ldr r2, %b
ldr r2, %c
ldr r2, %a
ldr r3, b
jmp c
jmp a
jmp b
ldr r4, [r3 + c]
ldr r4, [r3 + b]
ldr r4, [r3 + a]
jmp *c
jmp *a
jmp *b
jmp *[r3 + c]
jmp *[r3 + b]
jmp *[r3 + a]
ldr r3, c
ldr r4, [r3 + c]
ldr r4, [r3 + b]
ldr r4, [r3 + a]
ldr r3, a
.word 0xAC
b:

.end