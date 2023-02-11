.section test
.skip 2
a:
.global b
b:
.word b
c:
.word 0xFF
.skip 3
ldr r3, %b
str r4, $c
.word 0x02
.skip 7
.section nova
ldr r2, %b
str r6, $c
.word 0xAC
.end
