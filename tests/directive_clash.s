.global ant

.section eight

.word miks
.word ant
.word misko
.extern miks
.global misko

.equ ant, 1996 #section="ABS", info = 'G'

misko: ldr r5, [r1 + ant]
jmp %ant
kiuh: ldr r1, $ant
#ant: ldr r2, %kiuh
str r2, %ant


#dobro pitanje u ovom primeru: u koju sekciju treba da ide ant?
