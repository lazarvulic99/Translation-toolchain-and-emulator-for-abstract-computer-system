.section test
.skip 2
.global a
.extern c
a:
.word 0xFF
e:
.word a
ldr r5, [r4 + b]
ldr r5, [r4 + e]
.word b
ldr r2, [r3 + c]
.word c
ldr r3, $c
.word e
.section test2
.skip 6
b:
.end
