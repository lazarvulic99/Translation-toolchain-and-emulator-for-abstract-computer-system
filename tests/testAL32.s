.extern boo
.section prvasekcija
add r3, r0
.word 0x1234, -1024, b
lab:
.global lab
ldr r5, lab
ldr r0, [r1 + lab]
b: ldr r0, [r6 + c]
.word c, lab
.word b
jmp %b
ldr r1, %a
xchg r5, r2
xor r1, r3
.section drugasekcija
shr r5, r6
str r3, [r4 + labela1]
a:
str r6, 0x10
.section trecasekcija
push r5
gloo:
.global gloo
c:
pop r4
jmp %a
jmp %gloo
.word gloo
cmp r4, r5
labela1:
.section proba
jmp %vulic
push r0
vulic:
push r4
.end