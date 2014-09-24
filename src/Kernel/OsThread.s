.text


.global _HiloDelReloj
.global _at_thread
.global _fs_thread
.global _console_thread
.global _test_thread


_test_thread:
	push %ebp
	mov %esp, %ebp
	/*
		En ebx aparece la direccion del utcb area
	*/
	pushl %ebx
	call Test_Thread
	addl 4, %esp

	/* Espero no llegar aqui nunca*/

_console_thread:
	push %ebp
	mov %esp, %ebp
	/*
		En ebx aparece la direccion del utcb area
	*/
	pushl %ebx
	call Console_Thread
	addl 4, %esp

	/* Espero no llegar aqui nunca*/

_fs_thread:
	push %ebp
	mov %esp, %ebp
	/*
		En ebx aparece la direccion del utcb area
	*/
	pushl %ebx
	call fs_thread
	addl 4, %esp

	/* Espero no llegar aqui nunca*/

_at_thread:
	push %ebp
	mov %esp, %ebp
	/*
		En ebx aparece la direccion del utcb area
	*/
	pushl %ebx
	call at_thread
	addl 4, %esp

	/* Espero no llegar aqui nunca*/

_HiloDelReloj:
	push %ebp
	movl %esp, %ebp
	/*
		En ebx aparece la direccion del utcb area
	*/
	
	pushl %ebx
	call HiloDelReloj
	addl 4, %esp
	
	/* Espero no llegar aqui nunca*/
	