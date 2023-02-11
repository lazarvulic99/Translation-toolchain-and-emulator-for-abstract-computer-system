.extern labela1
.section firstSection
push r5
push r4
call labela2
ldr r5, $labela1
ldr r4, $labela1
cmp r5, r4
sub r5, r4
.section thirdSection
.word 0x1234
.word 0x0123
push r3
labela2: 
ldr r3, $0x0005
add r3, r3
div r3, r1
mul r3, r3
ret
.end 