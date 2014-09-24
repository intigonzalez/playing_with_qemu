 .text


.global GlobalInit

/*
;EXTERN main
;EXTERN pMultibootInfo

*/
GlobalInit:
    movl %ebx,pMultibootInfo

    cli
    cld

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
