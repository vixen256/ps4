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
	push rdi
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15

	mov ebx, eax
	mov ecx, eax
	call getSurvivalIdForIndex
	mov r9d, eax

	mov ecx, ebx
	call realLoadSurvivalSprite
	mov r8, rax

	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop rdi
	pop rsi
	pop rdx
	pop rcx
	pop rbx

	mov rax, [rel whereLoadSurvivalSprite]
	add rax, 7
	jmp rax
