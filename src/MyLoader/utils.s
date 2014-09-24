SEGMENT .text

GLOBAL active32bit_pagination


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