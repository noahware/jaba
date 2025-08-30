extern java_hook_method_detour : proc

public interpreted_method_hook_handler
public i2i_method_hook_handler

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

	; method hook type 0 = interpreted, method hook type 1 = i2i
	generic_method_handler macro method_entry_type:req
		pushfq

		sub rsp, 80h ; 78h for trap frame, 8h for original handler if needed
		save_general_purpose_registers

		lea rcx, [rsp+88h] ; starting_rsp
		mov rdx, rbx ; method
		mov r8, method_entry_type

		push rdx
		lea r9, [rsp] ; should_call_original

		sub rsp, 20h
		call java_hook_method_detour
		add rsp, 20h

		cmp byte ptr [rsp], 1
		pop rdx

		restore_general_purpose_registers

		movq xmm0, rax

		je jump_to_original_handler

		add rsp, 80h
		popfq

		mov rdi, [rsp]
		mov rsp, r13

		jmp rdi

	jump_to_original_handler:
		add rsp, 80h
		popfq

		jmp qword ptr [rsp-16]
	endm

	interpreted_method_hook_handler proc
		generic_method_handler 0
	interpreted_method_hook_handler endp

	i2i_method_hook_handler proc
		generic_method_handler 1
	i2i_method_hook_handler endp
END