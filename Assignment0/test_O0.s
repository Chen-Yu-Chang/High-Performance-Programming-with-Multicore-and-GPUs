	.file	"test_O_level.c"
	.section	.rodata
.LC0:
	.string	"\n Starting a loop "
.LC1:
	.string	"\n done "
	.text
	.globl	main
	.type	main, @function
main:
.LFB0:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movl	%edi, -20(%rbp)
	movq	%rsi, -32(%rbp)
	movq	$0, -16(%rbp)
	movl	$.LC0, %edi
	call	puts
	movq	$0, -8(%rbp)
	jmp	.L2
.L3:
	addq	$3, -16(%rbp)
	addq	$1, -8(%rbp)
.L2:
	cmpq	$100000000, -8(%rbp)
	jle	.L3
	movl	$.LC1, %edi
	call	puts
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	main, .-main
	.ident	"GCC: (GNU) 4.8.5 20150623 (Red Hat 4.8.5-44)"
	.section	.note.GNU-stack,"",@progbits
