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

extern implOfSetModuleSprArgs
extern whereSetModuleSprArgs
extern realSetModuleSprArgs

extern implOfSetHairstyleSprArgs
extern whereSetHairstyleSprArgs
extern realSetHairstyleSprArgs

extern implOfSetItemSprArgs
extern whereSetItemSprArgs
extern realSetItemSprArgs

extern implOfLoadReccomendChoiceList
extern whereLoadReccomendChoiceList
extern realLoadReccomendChoiceList

extern implOfSetModuleChoiceListPriority
extern whereSetModuleChoiceListPriority
extern implOfSetHairstyleChoiceListPriority
extern whereSetHairstyleChoiceListPriority
extern implOfSetItemChoiceListPriority
extern whereSetItemChoiceListPriority
extern realSetChoiceListPriority

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

implOfSetModuleSprArgs:
	pushaq
	sub rsp, 0x200

	mov rcx, rsi
	lea rdx, [rbp + 0x50]
	mov r8d, r11d
	call realSetModuleSprArgs

	add rsp, 0x200
	popaq
	mov rax, [rel whereSetModuleSprArgs]
	add rax, 6 + 8
	jmp rax

implOfSetHairstyleSprArgs:
	pushaq
	sub rsp, 0x200

	mov rcx, rdi
	lea rdx, [rbp + 0x110]
	mov r8d, r14d
	cmp r8d, 5
	jl .lesser
	mov r8d, [rsp + 0x288]
	add r8d, [rdi + 0x138 + 0x98]
.lesser:
	call realSetHairstyleSprArgs

	add rsp, 0x200
	popaq
	mov rax, [rel whereSetHairstyleSprArgs]
	add rax, 6 + 8
	jmp rax

implOfSetItemSprArgs:
	pushaq
	sub rsp, 0x200

	mov rcx, rdi
	lea rdx, [rbp + 0x40]
	mov r8d, r11d
	call realSetItemSprArgs

	add rsp, 0x200
	popaq
	mov rax, [rel whereSetItemSprArgs]
	add rax, 6 + 8
	jmp rax

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

	mov rcx, rdi
	mov edx, r15d
	cmp edx, 5
	jl .lesser
	mov edx, [rsp + 0x288]
	add edx, [rdi + 0x110 + 0x98]
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
	mov ecx, [rdi + 0x1BC]
	jmp SetChoiceListPriority
implOfSetHairstyleChoiceListPriority:
	mov rax, [rel whereSetHairstyleChoiceListPriority]
	mov ecx, [rdi + 0x1E4]
	jmp SetChoiceListPriority
implOfSetItemChoiceListPriority:
	mov rax, [rel whereSetItemChoiceListPriority]
	mov ecx, [rdi + 0x124]
SetChoiceListPriority:
	push rax
	push rcx
	push rdx
	push r8
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	sub rsp, 0x200

	mov edx, ebx
	call realSetChoiceListPriority
	mov r9d, eax

	add rsp, 0x200
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r8
	pop rdx
	pop rcx
	pop rax

	add rax, 6
	jmp rax

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
