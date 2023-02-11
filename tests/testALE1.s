.global labela1
.section firstSection
.word 0x0005
push r2
ldr r2, $0x1234
ldr r1, $0x0002
ldr r0, $0x02AB
.section secondSection
.word 2
labela1:
.word labela1
.word 4
halt
.end