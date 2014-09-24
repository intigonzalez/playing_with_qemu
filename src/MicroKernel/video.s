SECTION .text

Global printf
EXTERN printf2

; void printf(char* format, ...)
printf:
    ; los parametros son colocados de derecha a izquierda en la pila

    push ebp
    mov ebp , esp

    mov esi,[ebp + 8] ; creo que esta es la cadena de formato

    
    mov edi, ebp
    add edi, 12 ; le paso la direccio del primer parametro en la pila
    push edi
    push esi
    call printf2
    pop esi
    pop edi
   

    pop ebp

    

    ret