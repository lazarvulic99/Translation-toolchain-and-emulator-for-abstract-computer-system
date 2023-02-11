.section test
.skip 2
a:
.word 0xFF
.skip 3
.word 0xAB
ldr r3, %b
.word 0x02, 0x03, 0x04
.skip 7
.section nova
ldr r2, %b
.word 0xAC
b:

.end
