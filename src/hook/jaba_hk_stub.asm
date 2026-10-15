extern jaba_hook_c_handler : proc;
extern ?native_entry@abstract_interpreter@jaba@@2PEAEEA : qword

.code
	; these exclude rbx as that is what the C handler will modify
	push_most_regs macro
		pushfq
		push rax
		push rcx
		push rdx
		push rbp
		push rsi
		push rdi
		push r8
		push r9
		push r10
		push r11
		push r12
		push r13
		push r14
		push r15
	endm

	pop_most_regs macro
		pop r15
		pop r14
		pop r13
		pop r12
		pop r11
		pop r10
		pop r9
		pop r8
		pop rdi
		pop rsi
		pop rbp
		pop rdx
		pop rcx
		pop rax
		popfq
	endm

	jaba_hk_stub proc
		push_most_regs

		push rbx
		mov rcx, rsp ; now rcx holds ref to rbx (method ptr)

		; arg0 = method**
		mov rbp, rsp
		and rsp, -16
		sub rsp, 20h
		call jaba_hook_c_handler

		mov rsp, rbp
		pop rbx ; write whatever the C handler wrote method ptr to

		pop_most_regs

		jmp ?native_entry@abstract_interpreter@jaba@@2PEAEEA
	jaba_hk_stub endp
end
