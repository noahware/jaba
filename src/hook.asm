extern original_interpreted_method_handler : qword
extern original_i2i_method_handler : qword
extern method_detour : proc

.code
	save_general_purpose_registers macro
		mov [rsp], rax
		mov [rsp+8], rcx
		mov [rsp+16], rdx
		mov [rsp+24], rbx
		mov [rsp+32], rbp
		mov [rsp+40], rsi
		mov [rsp+48], rdi
		mov [rsp+56], r8
		mov [rsp+64], r9
		mov [rsp+72], r10
		mov [rsp+80], r11
		mov [rsp+88], r12
		mov [rsp+96], r13
		mov [rsp+104], r14
		mov [rsp+112], r15
	endm

	restore_general_purpose_registers macro
		mov rax, [rsp]
		mov rcx, [rsp+8]
		mov rdx, [rsp+16]
		mov rbx, [rsp+24]
		mov rbp, [rsp+32]
		mov rsi, [rsp+40]
		mov rdi, [rsp+48]
		mov r8, [rsp+56]
		mov r9, [rsp+64]
		mov r10, [rsp+72]
		mov r11, [rsp+80]
		mov r12, [rsp+88]
		mov r13, [rsp+96]
		mov r14, [rsp+104]
		mov r15, [rsp+112]
	endm

	generic_method_handler macro original_handler:req
		pushfq

		sub rsp, 78h
		save_general_purpose_registers

		lea rcx, [rsp+88h]

		sub rsp, 20h
		call method_detour
		add rsp, 20h

		cmp al, 1

		restore_general_purpose_registers

		je jump_to_original_handler

		add rsp, 78h
		popfq

		push rax
		mov rax, [rsp]
		mov [r13-8], rax
		pop rax

		mov rsp, r13
		jmp qword ptr [rsp-8]
	jump_to_original_handler:
		add rsp, 78h
		popfq

		jmp original_handler
	endm

	interpreted_method_hook_handler proc
		generic_method_handler original_interpreted_method_handler
	interpreted_method_hook_handler endp

	i2i_method_hook_handler proc
		generic_method_handler original_i2i_method_handler
	i2i_method_hook_handler endp
END