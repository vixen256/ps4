%include "cmn.asm"

extern getSurvivalIdForIndex

extern implOfLoadSurvivalSprite
extern whereLoadSurvivalSprite
extern realLoadSurvivalSprite

section .text
implOfLoadSurvivalSprite:
	push rbx
	push rcx
	push rdx
	push rsi
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15

	mov ebx, eax
	mov ecx, eax
	call getSurvivalIdForIndex

	mov ecx, ebx
	mov ebx, eax

	call realLoadSurvivalSprite

	mov r8, rax
	mov r9d, ebx

	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop rsi
	pop rdx
	pop rcx
	pop rbx

	mov rax, [rel whereLoadSurvivalSprite]
	add rax, 7
	jmp rax
