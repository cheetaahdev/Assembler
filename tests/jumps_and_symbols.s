.global ant
.equ minion, 063
.extern kiks
#.word simbol # ovo verovatno ne bi trebalo da je dozvoljeno
.section text
.word ant
cmp r1, r2
ant: ldr r1, r3
jmp %ant
jmp %kiks
jeq kiks
jmp *ant
jne 0x04D1
jgt %minion
jgt *[r3]
jne *[r2 + ant]
