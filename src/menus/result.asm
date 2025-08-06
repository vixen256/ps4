%include "cmn.asm"

extern implOfLoadSurvivalSprite
extern whereLoadSurvivalSprite
extern realLoadSurvivalSprite

section .text
implOfLoadSurvivalSprite:
	push rcx
	push rdx
	push rsi
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15

	mov rcx, rax
	call realLoadSurvivalSprite
	mov r8, rax

	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rsi
	pop rdx
	pop rcx
	pop rax

	mov rax, [rel whereLoadSurvivalSprite]
	add rax, 7
	jmp rax
