SEGMENT .text

GLOBAL low_level_clock_handler

GLOBAL low_level_sys_call

GLOBAL _save, _restore, _tablaHandlers, _allInterruptHandlers

GLOBAL activate_multitasking , puntero
Global GetBasicHandler,FastSysCall

;Global _restore1

; esto es haraganeria, es una forma de definir varios simbolos globales
%macro DefineGlobalSymbol 2
		GLOBAL %{1}%{2}
%endmacro

%macro DefineSymbol 2
		%{1}%{2}:
%endmacro

%assign iii 0
%rep 20
	DefineGlobalSymbol low_level_,iii
%assign iii iii+1
%endrep


EXTERN high_level_clock_handler ; handler del reloj
EXTERN high_level_handler,high_level_handler_faults

EXTERN microkernel_syscall


EXTERN activeThread ; este es el hilo activo
EXTERN _hwParameters;

; forma de usarlo 
; DefineFaultHandler <number>, [1|0]
; el 2 parametro es 1 si la interrupcion produce un error code
%macro DefineFaultHandler 2
	low_level_%1:
	push ebp
	mov ebp, esp
	pusha
	mov eax , %1
	%if %2<1
		jmp _manage
	%else
		jmp _manage_with_error_code
	%endif
%endmacro

DefineFaultHandler 0, 0
DefineFaultHandler 1, 0
DefineFaultHandler 2, 0
DefineFaultHandler 3, 0
DefineFaultHandler 4, 0
DefineFaultHandler 5, 0
DefineFaultHandler 6, 0
DefineFaultHandler 7, 0
DefineFaultHandler 8, 1
DefineFaultHandler 9, 0
DefineFaultHandler 10, 1
DefineFaultHandler 11, 1
DefineFaultHandler 12, 1
DefineFaultHandler 13, 1
DefineFaultHandler 14, 1
DefineFaultHandler 15, 0
DefineFaultHandler 16, 0
DefineFaultHandler 17, 0
DefineFaultHandler 18, 0
DefineFaultHandler 19, 0
				
				
_manage_with_error_code:
		push dword [ebp + 4] ; tomo error code
		push dword [ebp + 8] ; tomo eip
		push dword [ebp + 12] ; tomo cs
        push eax
		mov eax, 0x10
		mov ds, ax
        call high_level_handler_faults
        add esp , 16
		mov eax, 0x23
		mov ds, ax		
        popa
		pop ebp
		; saco el codigo de error
		add esp, 4
        iret
_manage:
		push dword 0
		push dword [ebp + 4] ; tomo eip
		push dword [ebp + 8] ; tomo cs
        push eax
		mov eax, 0x10
		mov ds, ax
        call high_level_handler_faults
        add esp , 16
		mov eax, 0x23
		mov ds, ax
        popa
		pop ebp
        iret

_tablaHandlers:		
%assign interruptId 0
%rep 256
	call strict _save
	mov strict eax, interruptId
	jmp strict _allInterruptHandlers
	%assign interruptId interruptId+1
%endrep

_allInterruptHandlers:
	; llamo al handler de interrupciones universal
	push eax
	call high_level_handler
	add esp , 4
	
	; Regreso los valores de contexto al hilo que se va a ejecutar
	call _restore
	; ahora le doy el poder a ese hilo
	iret

SizeSize equ (_allInterruptHandlers-_tablaHandlers)/256
; void* GetBasicHandler(unsigned number)
GetBasicHandler:
	push ebp
	mov ebp, esp
	
	push ebx
	push edx
	
	mov eax, dword [ebp + 8]
	mov ebx, SizeSize
	mul ebx
	add eax, _tablaHandlers
	
	pop edx
	pop ebx
	pop ebp
	ret
	
low_level_clock_handler:
	call _save
	;add esp , ecx
	;push eax	; salvo lo que devolvio _save
	;;pusha

	;pusha
	;push 12
	; las interrupciones estan ahora desabilitas por el procesador
	call high_level_clock_handler
	; habilito una vez mas las interrupciones en el PIC
	mov al , 0x20
	out 0x20 , al
	
	
	;;popa
	;pop eax
	call _restore


  ;popa
	iret

; ESTE HANDLER CONTROLA LAS LLAMADAS AL SISTEMA
; LOS PARAMETROS (QUE_HACER , help_PId, MSG)
; el supone que _save no cambiara estos registros 	
low_level_sys_call:
	call _save
	; ahora que ya salve el estado de ejecucion del hilo activo se puede hacer lo que sea
	push dword 0
	call microkernel_syscall
	add esp , 4
	; ya termine mi trabajo, demosle el poder a otro hilo
	call _restore
	iret

FastSysCall:
	;cli
	mov esi, dword [activeThread] ; pongo en esi el puntero
	mov eax, dword [esi + 68]
	add eax, 256
	mov esp, eax
	sti

	push ecx
	push edx
	mov bx , 0x10
    mov ds, bx       ; KERNEL_DATA_SEGMENT
	; en edx esta eip de user code
	; en ecx esta esp de user
	push dword 43
	call microkernel_syscall
	add esp , 4
	
	mov bx , 0x23
    mov ds, bx       ; USER_DATA_SELECTOR
	
	pop edx
	pop ecx
	sysexit

; esta rutina comienza la ejecucion de los hilos del sistema
; debe llamarse cuando se termino la configuracion inicial del
; microkernel
activate_multitasking:
	call _restore
	; un forma linda de activar las interrupciones y comenzar a ejecutar un hilo
	; 3
	; 2
	; 1
	; 0
	iret

; salva el estado del hilo activo
_save:
    push esi
    push ebx
    mov bx, ds
    push ebx
    mov bx , 0x10
    mov ds, bx       ; el selector de KERNEL_DATA_SEGMENT
    pop ebx

    mov esi, dword [activeThread] ; pongo en esi el puntero

    mov dword [esi + 0],    eax

    mov eax, dword [esp + 0]              ; estas dos lineas son para salvar ebx
    mov dword [esi + 4],    eax

    mov dword [esi + 8],    ecx
    mov dword [esi + 12],   edx

	; bien, deseo salvar los apuntadores a la pila para el proceso que se ejecutaba, necesito para ello saber 
	; si se ejecutaba en modo kernel o modo usuario, eso me lo dice cs
	mov eax, dword [esp + 16]  
	cmp eax, 0x08
	je modo_kernel
	
    mov eax, dword [esp + 24]             ; estas dos lineas son para salvar esp
    mov dword [esi + 16],   eax
	mov eax, dword [esp + 28]             ; estas dos lineas son para salvar ss
    mov dword [esi + 44] ,   eax

	jmp ss_saved
	
modo_kernel:
	
	xor eax, eax
	mov ax, ss
	mov dword [esi + 44] ,   eax
	mov eax, esp
	add eax, 24
	mov dword [esi + 16],   eax
ss_saved:

    mov dword [esi + 20],   ebp						

    mov eax, dword [esp + 4]              ; estas dos lineas son para salvar esi
    mov dword [esi + 24],   eax

    mov dword [esi + 28],   edi

    mov eax, dword [esp + 12]             ; estas dos lineas son para salvar eip
    mov dword [esi + 32],   eax

    mov eax, dword [esp + 16]             ; estas dos lineas son para salvar cs
    mov dword [esi + 36],   eax

    mov word [esi + 40],    bx            ; salvo ds

    mov word [esi + 48] ,   es
    mov word [esi + 52] ,   fs
    mov word [esi + 56] ,   gs

    mov eax, dword [esp + 20]             ; estas dos lineas son para salvar eflags
    mov dword [esi + 60] ,  eax

    ; pongamos el nuevo valor de cr3 para usar el directorio de paginas del kernel
    mov eax , [_hwParameters + 4]
    mov cr3 , eax   ; muy lento

    pop ebx
    pop esi
    ret

_restore:
    mov esi , dword [activeThread] ; pongo en esi el puntero

    ; pongamos el nuevo valor de cr3 al directorio de paginas del hilo
    mov eax , dword [esi + 64]
    mov cr3 , eax       ; muy lento

	mov eax , dword [esi + 36]      ; tomo el valor de cs
									; en base a este valor sabré si el hilo a activar estaba 
									; ejecutandose con privilegio 0 o 3 y en base a eso restauro SS
	cmp eax, 0x08
	jne restore_privilege3
	
	pop ebx 						; acabo de tomar la posicion de retorno de la funcion _restore
	mov eax , dword [esi + 44]       ; estas restauran ss
	mov ss, ax
	mov eax , dword [esi + 16]      ; estas restauran esp
	mov esp, eax
	push ebx
	push ebx
	push ebx
	push ebx
	jmp ss_restored
	
restore_privilege3:
	; voy a cambiar de pila cuando haga iret asi que no me importa acabar con esta pila
	mov eax, dword [esp]
	push eax
	push eax
	push eax
	push eax
	push eax
	mov eax , dword [esi + 44]        ; estas restauran ss
    mov dword [esp + 20] , eax

    mov eax , dword [esi + 16]      ; estas restauran esp
    mov dword [esp + 16] , eax
	
ss_restored:

    mov eax , dword [esi + 24]      ; lineas para restaurar esi
    push eax

    mov eax , dword [esi + 4]       ; lineas para restaurar ebx
    push eax

    ; empiezo a restaurar valores
	mov eax , dword [esi + 60]      ; estas restauran eflags
    mov dword [esp + 20] , eax

    mov es , word [esi + 48]
    mov fs , word [esi + 52]
    mov gs , word [esi + 56]

    mov bx , word [esi + 40]        ; estas son para ds

    mov eax , dword [esi + 36]      ; estas cs
    mov dword [esp + 16] , eax

    mov eax , dword [esi + 32]      ; estas restauran eip
    mov dword [esp + 12] , eax

    mov edi , dword [esi + 28]

    mov ebp , dword [esi + 20]

    mov ecx , dword [esi + 8]
    mov edx , dword [esi + 12]

    mov eax , dword [esi + 0]

    mov ds, bx
    pop ebx
    pop esi
    ret
