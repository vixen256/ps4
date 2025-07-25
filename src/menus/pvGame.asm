%include "cmn.asm"

extern implOfProcessRenderCommand
extern whereProcessRenderCommand
extern realProcessRenderCommand

section .text
implOfProcessRenderCommand:
	cmp rax, 0x90
	jne .genuine
	pushaq
	call realProcessRenderCommand
	popaq
	jmp .exit
.genuine:
	call [rsp + rax * 8 + 0x20]
.exit:
	mov rcx, [rbp + 0x10]
	mov rax, [rel whereProcessRenderCommand]
	add rax, 8
	jmp rax
