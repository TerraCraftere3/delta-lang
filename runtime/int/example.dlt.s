global _start
extern ExitProcess

section .text
_start:
	sub rsp, 40
	mov ecx, 21
	call ExitProcess
