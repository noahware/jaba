// system v version of jaba_hk_stub.asm, the interpreter still keeps the method* in rbx
#ifndef _WIN32

// rsi and rdi are argument registers here, so they are saved on top of what the windows stub saves
asm(R"(
.intel_syntax noprefix
.text
.globl jaba_hk_stub
.type jaba_hk_stub, @function
jaba_hk_stub:
	pushfq
	push rax
	push rcx
	push rdx
	push rsi
	push rdi
	push r8
	push r9
	push r10
	push r11

	sub rsp, 0x60
	movdqu xmmword ptr [rsp], xmm0
	movdqu xmmword ptr [rsp+0x10], xmm1
	movdqu xmmword ptr [rsp+0x20], xmm2
	movdqu xmmword ptr [rsp+0x30], xmm3
	movdqu xmmword ptr [rsp+0x40], xmm4
	movdqu xmmword ptr [rsp+0x50], xmm5

	push rbp
	push rbx
	mov rdi, rsp
	mov rbp, rsp
	and rsp, -16
	call jaba_hook_c_handler
	mov rsp, rbp
	pop rbx
	pop rbp

	movdqu xmm5, xmmword ptr [rsp+0x50]
	movdqu xmm4, xmmword ptr [rsp+0x40]
	movdqu xmm3, xmmword ptr [rsp+0x30]
	movdqu xmm2, xmmword ptr [rsp+0x20]
	movdqu xmm1, xmmword ptr [rsp+0x10]
	movdqu xmm0, xmmword ptr [rsp]
	add rsp, 0x60

	pop r11
	pop r10
	pop r9
	pop r8
	pop rdi
	pop rsi
	pop rdx
	pop rcx
	pop rax
	popfq

	jmp qword ptr [rip + _ZN4jaba3jvm20abstract_interpreter12native_entryE]
.size jaba_hk_stub, .-jaba_hk_stub
.att_syntax
)");

#endif
