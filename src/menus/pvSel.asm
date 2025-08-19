extern implOfInitSongListNum
extern whereInitSongListNum
extern _ZN4diva3pvsE

extern implOfGetMaxSongListNumFiltered
extern whereGetMaxSongListNumFiltered

extern implOfGetMaxSongListNumTotal
extern whereGetMaxSongListNumTotal

song_list_num: db "song_list_num", 0, 0, 0
song_list_num_ex: db "song_list_num_ex", 0, 0, 0, 0, 0, 0, 0

section .text:
GetSongCount:
	mov rcx, [rel _ZN4diva3pvsE]
	mov rax, [rcx + 8]
	mov rcx, [rcx]
	sub rax, rcx
	mov rdx, 0
	mov rcx, 8
	div rcx
	ret

implOfInitSongListNum:
	call GetSongCount

	mov rdx, song_list_num
	mov rcx, song_list_num_ex
	cmp rax, 999
	cmovg rdx, rcx

	mov rcx, [rel whereInitSongListNum]
	add rcx, 7
	jmp rcx

implOfGetMaxSongListNumFiltered:
	mov rsi, [rel whereGetMaxSongListNumFiltered]
	jmp GetMaxSongListNum
implOfGetMaxSongListNumTotal:
	mov rsi, [rel whereGetMaxSongListNumTotal]
GetMaxSongListNum:
	call GetSongCount

	mov ecx, 3
	mov edx, 4
	cmp rax, 999
	cmovg ecx, edx

	xor al, al

	cmp r14b, cl
	jnb .exit
	mov eax, ecx
	sub al, r14b
	mov r14b, cl

.exit:
	add rsi, 4 + 2 + 5 + 3 + 3
	jmp rsi
