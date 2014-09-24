 .text


.global GlobalInit

.extern params;

GlobalInit:

    cli
    cld
	
	/* 
	al llegar aqui venimos del cargador, en la pila tenemos la siguiente informacion
	 RamDisk Addr 
	 OS Addr
	 MemoryCount
	 Addr_Page_Directory
	 */
	lea params, %esi
	pop %eax
	movl %eax, (%esi) /* en la primera posicion cae la la direccion del directorio de paginas */
	pop %eax
	movl %eax, 4(%esi) /* en la primera posicion cae la cantidad de memoria */
	pop %eax
	movl %eax, 8(%esi) /* en la segunda posicion cae la direccion del root server */
	pop %eax
	movl %eax, 12(%esi) /* en lqa tercera cae la direccion del RamDisk */
	
    /*; pongo una pila*/
    lea _pila,%esp

    call main

ciclo: jmp ciclo

    .align 16
    .space 2048
_pila:

 .section .mba
    jmp GlobalInit
    .ascii "Esto no se para que es"
    .align 16, 0x90
_mba_header:
    .long 0x1BADB002
    .long 0x00000000
    .long -0x00000000-0x1BADB002
    .long _mba_header /*; la direccion donde se encuentra este header*/
    .long GlobalInit /*; la direccion de carga de este codigo*/
    .long _edata /*; la direccion de fin de lo cargado*/
    .long _end /*; la direccion de fin de cargado de bss, 0 indica nada*/
    .long GlobalInit
