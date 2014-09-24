SEGMENT .text
GLOBAL lock_interrupts,unlock_interrupts

GLOBAL set_new_gdt, set_new_idt, set_new_tss, active32bit_pagination

GLOBAL interrupt_test

GLOBAL idle_thread_entry

idle_thread_entry:
	jmp idle_thread_entry


lock_interrupts:
	cli
	ret
	
unlock_interrupts:
	sti
	ret
	
set_new_gdt:
	push ebp
	mov ebp , esp
	push esi
	mov esi , dword [ebp + 8]
	lgdt [esi]
	jmp 08h:veremos
	veremos:
	
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	
	pop esi
	pop ebp
	ret
	
set_new_idt:
	push ebp
	mov ebp , esp
	push esi
	mov esi , dword [ebp + 8]
	lidt [esi]
	pop esi
	pop ebp
	ret
	
set_new_tss:
	push ebp
	mov ebp , esp
	mov ax , word [ebp + 8]
	ltr ax
	pop ebp
	ret

; el prototipo de esta funcion es
;	void _active32bit_pagination(void* page_directory)
active32bit_pagination:
	push ebp
	mov ebp , esp
	push ebx
	
	mov ebx , [ebp + 8]	; cojo la direccion de la tabla de paginas
	mov cr3 , ebx
	mov eax, cr4
	and eax , 0xFFFFFFD7 ; PAE = 2 y PSE = 0 que marca paginas de 4 kb
	mov cr4 , eax

	mov eax , cr0
	and eax , 0xBFFFFFFF ; deshabilitamos la cache
	or eax , 0x80000000 ; activo la paginacion
	mov cr0 , eax	
	pop ebx
	pop ebp
	ret
	
interrupt_test:
	int 0x15
	int 0x50
	int 0xAF
	ret

