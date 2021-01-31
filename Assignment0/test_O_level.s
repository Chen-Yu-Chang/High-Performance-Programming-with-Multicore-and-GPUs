	.file	"test_O_level.c"
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"\n Starting a loop "
.LC1:
	.string	"\n done "
	.text
	.globl	main
	.type	main, @function
main:
.LFB11:
	.cfi_startproc
	subq	$8, %rsp
	.cfi_def_cfa_offset 16
	movl	$.LC0, %edi
	call	puts
	movl	$100000001, %eax
.L3:
	subq	$1, %rax
	jne	.L3
	movl	$.LC1, %edi
	call	puts
	addq	$8, %rsp
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE11:
	.size	main, .-main
	.ident	"GCC: (GNU) 4.8.5 20150623 (Red Hat 4.8.5-44)"
	.section	.note.GNU-stack,"",@progbits
