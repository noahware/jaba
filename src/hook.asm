extern original_compiled_method_handler : qword
extern original_interpreted_method_handler : qword

.code
	compiled_method_hook_handler proc
		jmp original_compiled_method_handler
	compiled_method_hook_handler endp

	interpreted_method_hook_handler proc
		jmp original_interpreted_method_handler
	interpreted_method_hook_handler endp
END