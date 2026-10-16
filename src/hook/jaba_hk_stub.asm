extern jaba_hook_c_handler : proc;
extern ?native_entry@abstract_interpreter@jvm@jaba@@2PEAEEA : qword

.code
	; these exclude rbx as that is what the C handler will modify
	push_most_regs macro
		pushfq
		push rax
		push rcx
		push rdx
		push r8
		push r9
		push r10
		push r11

		sub rsp, 60h
		movdqu xmmword ptr [rsp], xmm0
		movdqu xmmword ptr [rsp+10h], xmm1
		movdqu xmmword ptr [rsp+20h], xmm2
		movdqu xmmword ptr [rsp+30h], xmm3
		movdqu xmmword ptr [rsp+40h], xmm4
		movdqu xmmword ptr [rsp+50h], xmm5
	endm

	pop_most_regs macro
		movdqu xmm5, xmmword ptr [rsp+50h]
		movdqu xmm4, xmmword ptr [rsp+40h]
		movdqu xmm3, xmmword ptr [rsp+30h]
		movdqu xmm2, xmmword ptr [rsp+20h]
		movdqu xmm1, xmmword ptr [rsp+10h]
		movdqu xmm0, xmmword ptr [rsp]
		add rsp, 60h

		pop r11
		pop r10
		pop r9
		pop r8
		pop rdx
		pop rcx
		pop rax
		popfq
	endm

	jaba_hk_stub proc
		push_most_regs

		push rbp
		push rbx
		mov rcx, rsp ; now rcx holds ref to rbx (method ptr)

		; arg0 = method**
		mov rbp, rsp
		and rsp, -16
		sub rsp, 20h
		call jaba_hook_c_handler

		mov rsp, rbp
		pop rbx ; write whatever the C handler wrote method ptr to
		pop rbp

		pop_most_regs

		jmp ?native_entry@abstract_interpreter@jvm@jaba@@2PEAEEA
	jaba_hk_stub endp
end
