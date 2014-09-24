SEGMENT .text

GLOBAL ENTRADA

extern OSInit

extern pbootInfo

ENTRADA:
	;cli
	; en la pila inicial se tienen dos cosas
	; en el tope la longitud de una secuencia de dword de datos
	; despues los bytes de datos
	mov edi, dword 0
	mov esi, dword [pbootInfo]
	pop ecx
ciclocopiando:
	pop  eax ; tomo 4 bytes
	mov [esi + edi*4], eax
	inc edi
	dec ecx
	cmp ecx,0
	jne ciclocopiando

jojojo:
	; activo la pila nueva
	mov eax,pila
	mov esp,eax

	; salto al verdadero kernel
	mov eax, _end
	push eax
	call OSInit
	add esp , 4
	jmp jojojo
	
SEGMENT .bss
iniciopila resb	2048
pila:
SEGMENT .data
EXTERN _end	; este simbolo es definido por el script del linker, solo importa la direccion
	