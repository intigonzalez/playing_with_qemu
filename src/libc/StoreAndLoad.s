SEGMENT .text

Global LoadRegistre, StoreRegistre , StoreByteRegistre , LoadByteRegistre, PassToKernelMode, EtiquetaDeRetorno

; int StoreRegistre(unsigned index);
StoreRegistre:
	push esi
	mov esi, [esp + 8]  ; el primer parametro
	mov eax, dword [gs:esi*4] ; tomo el valor del registro
	pop esi
	ret

; void LoadRegistre(unsigned index, int value);
LoadRegistre:
	push esi
	push eax
	mov esi, [esp + 12]  			; el primer parametro
	mov eax, [esp + 16]
	mov dword [gs:esi*4],eax  ; establesco el valor del registro
	pop eax
	pop esi
	ret

; int StoreByteRegistre(unsigned index);
StoreByteRegistre:
	push esi
	mov esi, [esp + 8]  ; el primer parametro
	xor eax, eax
	mov al, byte [gs:esi] ; tomo el valor del registro
	pop esi
	ret
	
; void LoadByteRegistre(unsigned index, unsigned value);
LoadByteRegistre:
	push esi
	push eax
	mov esi, [esp + 12]  			; el primer parametro
	mov eax, [esp + 16]
	mov byte [gs:esi],al  ; establezco el valor del registro
	pop eax
	pop esi
	ret
	
; entra a modo kernel, esto es por el momento para prueba
PassToKernelMode:
	push ebp
	mov ebp, esp
	pusha
	mov dword edx, [ebp+8] ; es el primer parametro y la direccion de retorno
	mov ecx, esp
	sysenter
	
EtiquetaDeRetorno:
	popa
	pop ebp
	sti
	ret

