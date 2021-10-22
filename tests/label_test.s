.global ant
.equ minion, 063
.extern kiks
.word simbol # ovo verovatno ne bi trebalo da je dozvoljeno
.section text
.word ant
.word mom
.extern mom
mem: cmp r1, r2
ant:
 ldr r1, r3
jmp %ant
pep:
jeq kiks
mffa: jmp *ant
jgt %minion
