.section test
.skip 2
a:
.extern b
.word 0xFF
ldr r3, %b
.section nova
.word 0x02
.end
