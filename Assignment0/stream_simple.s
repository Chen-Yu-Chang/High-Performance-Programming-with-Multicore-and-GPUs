	.file	"stream_simple.c"
	.text
	.globl	mysecond
	.type	mysecond, @function
mysecond:
.LFB15:
	.cfi_startproc
	subq	$40, %rsp
	.cfi_def_cfa_offset 48
	movq	%rsp, %rsi
	leaq	16(%rsp), %rdi
	call	gettimeofday
	cvtsi2sdq	16(%rsp), %xmm1
	cvtsi2sdq	24(%rsp), %xmm0
	mulsd	.LC0(%rip), %xmm0
	addsd	%xmm1, %xmm0
	addq	$40, %rsp
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE15:
	.size	mysecond, .-mysecond
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC4:
	.string	"AI = %f    GFLOPs/s = %f \n"
	.text
	.globl	main
	.type	main, @function
main:
.LFB14:
	.cfi_startproc
	movl	$0, %eax
	movl	$0, %edx
.L5:
	addq	$1, %rdx
	cvtsi2ssq	%rdx, %xmm0
	movss	%xmm0, a(%rax,%rax)
	cvtsi2ssq	%rax, %xmm0
	movss	%xmm0, b(%rax,%rax)
	addq	$2, %rax
	cmpq	$1000000, %rdx
	jne	.L5
	pushq	%r12
	.cfi_def_cfa_offset 16
	.cfi_offset 12, -16
	pushq	%rbp
	.cfi_def_cfa_offset 24
	.cfi_offset 6, -24
	pushq	%rbx
	.cfi_def_cfa_offset 32
	.cfi_offset 3, -32
	subq	$8016, %rsp
	.cfi_def_cfa_offset 8048
	leaq	8016(%rsp), %rbp
	leaq	16(%rsp), %rbx
.L9:
	movl	$0, %eax
	call	mysecond
	movsd	%xmm0, 8(%rsp)
	movq	%rbx, %r12
	movl	$0, %eax
.L7:
	movss	a(,%rax,4), %xmm0
	movaps	%xmm0, %xmm1
	mulss	%xmm0, %xmm1
	mulss	%xmm0, %xmm1
	mulss	%xmm0, %xmm1
	mulss	%xmm0, %xmm1
	mulss	%xmm0, %xmm1
	mulss	%xmm0, %xmm1
	mulss	%xmm0, %xmm1
	mulss	%xmm1, %xmm0
	movss	%xmm0, b(,%rax,4)
	addq	$1, %rax
	cmpq	$1000000, %rax
	jne	.L7
	movl	$0, %eax
	call	mysecond
	subsd	8(%rsp), %xmm0
	movsd	%xmm0, (%r12)
	addq	$8, %rbx
	cmpq	%rbp, %rbx
	jne	.L9
	leaq	16(%rsp), %rax
	xorpd	%xmm0, %xmm0
.L11:
	addsd	(%rax), %xmm0
	addq	$8, %rax
	cmpq	%rbp, %rax
	jne	.L11
	movsd	.LC2(%rip), %xmm1
	divsd	%xmm1, %xmm0
	mulsd	%xmm0, %xmm1
	movsd	.LC3(%rip), %xmm0
	movapd	%xmm0, %xmm2
	divsd	%xmm1, %xmm2
	movapd	%xmm2, %xmm1
	movl	$.LC4, %edi
	movl	$2, %eax
	call	printf
	movl	$0, %eax
	addq	$8016, %rsp
	.cfi_def_cfa_offset 32
	popq	%rbx
	.cfi_restore 3
	.cfi_def_cfa_offset 24
	popq	%rbp
	.cfi_restore 6
	.cfi_def_cfa_offset 16
	popq	%r12
	.cfi_restore 12
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE14:
	.size	main, .-main
	.comm	integer,28000000,32
	.local	b
	.comm	b,4000000,32
	.local	a
	.comm	a,4000000,32
	.section	.rodata.cst8,"aM",@progbits,8
	.align 8
.LC0:
	.long	2696277389
	.long	1051772663
	.align 8
.LC2:
	.long	0
	.long	1083129856
	.align 8
.LC3:
	.long	0
	.long	1071644672
	.ident	"GCC: (GNU) 4.8.5 20150623 (Red Hat 4.8.5-44)"
	.section	.note.GNU-stack,"",@progbits
