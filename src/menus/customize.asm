%include "cmn.asm"

extern implOfLoadModuleChoiceList
extern whereLoadModuleChoiceList
extern realLoadModuleChoiceList

extern implOfLoadHairstyleChoiceList
extern whereLoadHairstyleChoiceList
extern realLoadHairstyleChoiceList

extern implOfLoadItemChoiceList
extern whereLoadItemChoiceList
extern realLoadItemChoiceList

extern implOfLoadReccomendChoiceList
extern whereLoadReccomendChoiceList
extern realLoadReccomendChoiceList

extern implOfSetModuleChoiceListPriority
extern whereSetModuleChoiceListPriority
extern implOfSetHairstyleChoiceListPriority
extern whereSetHairstyleChoiceListPriority

extern implOfSetModuleSprPriority
extern whereSetModuleSprPriority
extern implOfSetHairstyleSprPriority
extern whereSetHairstyleSprPriority

extern implOfMemset
extern originalMemset

extern implOfUpdateBG10SpriteColor
extern whereUpdateBG10SpriteColor
extern implOfUpdateBG10TextColor
extern whereUpdateBG10TextColor
extern UpdateBG10Color

extern implOfUpdateBg05TextColor
extern whereUpdateBg05TextColor
extern UpdateBG05Color

section .text
strlen:
	mov r8d, -1
	dec rax
.loop:
	inc r8d
	inc rax
	cmp byte [rax], 0
	jne .loop
	ret

implOfLoadModuleChoiceList:
	push rax
	push rbx
	push rcx
	push rsi
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	sub rsp, 0x200

	mov rcx, rdi
	mov rdx, rsi
	mov r8, rbx
	call realLoadModuleChoiceList
	mov rdx, rax
	call strlen

	add rsp, 0x200
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop rsi
	pop rcx
	pop rbx
	pop rax

	mov r9, [rel whereLoadModuleChoiceList]
	add r9, 6 + 7
	jmp r9

implOfLoadHairstyleChoiceList:
	push rax
	push rbx
	push rcx
	push rsi
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	sub rsp, 0x200

	mov rcx, rdi
	mov rdx, r14
	mov r8, rbx
	call realLoadHairstyleChoiceList
	mov rdx, rax
	call strlen

	add rsp, 0x200
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop rsi
	pop rcx
	pop rbx
	pop rax

	mov r9, [rel whereLoadHairstyleChoiceList]
	add r9, 6 + 7
	jmp r9

implOfLoadItemChoiceList:
	push rax
	push rbx
	push rcx
	push rsi
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	sub rsp, 0x200

	mov rcx, rdi
	mov r8, rbx
	call realLoadItemChoiceList
	mov rdx, rax
	call strlen

	add rsp, 0x200
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop rsi
	pop rcx
	pop rbx
	pop rax

	mov r9, [rel whereLoadItemChoiceList]
	add r9, 6 + 7
	jmp r9

implOfLoadReccomendChoiceList:
	push rax
	push rbx
	push rcx
	push rsi
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	sub rsp, 0x200

	mov ecx, r15d
	cmp ecx, 5
	jl .lesser
	mov ecx, [rsp + 0x288]
	add ecx, [rdi + 0x110 + 0x98]
.lesser:
	call realLoadReccomendChoiceList
	mov rdx, rax
	call strlen

	add rsp, 0x200
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop rsi
	pop rcx
	pop rbx
	pop rax

	mov r9, [rel whereLoadReccomendChoiceList]
	add r9, 4 + 7
	jmp r9

implOfSetModuleChoiceListPriority:
	mov rax, [rel whereSetModuleChoiceListPriority]
	jmp SetChoiceListPriority
implOfSetHairstyleChoiceListPriority:
	mov rax, [rel whereSetHairstyleChoiceListPriority]
SetChoiceListPriority:
	mov r9d, ebx
	cmp r9d, 5
	jle .lesser
	neg r9d
	add r9d, 10
.lesser:
	imul r9d, 2
	add r9d, 9

	add rax, 6
	jmp rax

implOfSetModuleSprPriority:
	mov rdx, [rel whereSetModuleSprPriority]
	add rdx, 5 + 3 + 3
	mov eax, r11d
	jmp SetSprPriority
implOfSetHairstyleSprPriority:
	mov rdx, [rel whereSetHairstyleSprPriority]
	add rdx, 5 + 4 + 3 + 3
	mov eax, esi
	mov esi, [rsp+0x20]
SetSprPriority:
	cmp eax, 5
	jle .lesser
	neg eax
	add eax, 10
.lesser:
	imul eax, 2
	add eax, 10

	jmp rdx

implOfMemset:
	push r11
	call [rel originalMemset]
	pop r11
	ret

implOfUpdateBG10SpriteColor:
	pushaq
	sub rsp, 0x20
	call UpdateBG10Color

	mov eax, __?float32?__(255.0)
	movd xmm1, eax
	mulss xmm0, xmm1
	cvttss2si eax, xmm0
	shl eax, 0x18
	or eax, 0xFFFFFF
	mov [rbp + 0x208], eax

	add rsp, 0x20
	popaq

	mov rax, [rel whereUpdateBG10SpriteColor]
	add rax, 10
	jmp rax

implOfUpdateBG10TextColor:
	pushaq
	sub rsp, 0x20
	call UpdateBG10Color

	mov eax, __?float32?__(255.0)
	movd xmm1, eax
	mulss xmm0, xmm1
	cvttss2si eax, xmm0
	shl eax, 0x18
	or eax, 0xFFFFFF
	mov [rbp + 0x190], eax

	add rsp, 0x20
	popaq

	mov rax, [rel whereUpdateBG10TextColor]
	add rax, 9
	jmp rax

implOfUpdateBg05TextColor:
	pushaq
	sub rsp, 0x20
	call UpdateBG05Color

	mov eax, __?float32?__(255.0)
	movd xmm1, eax
	mulss xmm0, xmm1
	cvttss2si eax, xmm0
	shl eax, 0x18
	or eax, 0xFFFFFF
	mov [rbp + 0x208], eax

	add rsp, 0x20

	popaq
	mov rax, [rel whereUpdateBg05TextColor]
	add rax, 10
	jmp rax
