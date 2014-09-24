MICROKERNEL_CODE = *.cpp *.s *.h
INCLUDES = ../libc
FLAGS_C = -I$(INCLUDES) -std=gnu99 -ffreestanding -nostdlib -nostdinc
FLAGS_CPP = -I$(INCLUDES) -g -Wall -Wextra -nostdlib -fno-builtin -nostartfiles -nodefaultlibs -fno-exceptions -fno-rtti -fno-stack-protector
all:	todo.x
# MOVEMOS EL MICROKERNEL PARA EL DIRECTORIO DONDE ESTARA
	objcopy -O elf32-i386 todo.x
	cp todo.x microkernel.img

todo.x: $(MICROKERNEL_CODE)
# COMPILAMOS EL MICROKERNEL
	nasm -o0 -f elf kutils.s -o kutils.o
	nasm -o0 -f elf video.s -o video2.o
	nasm -o0 -f elf interrupt_handlers.s -o interrupt_handlers.o
	gcc -c crt0.s -o crt0.o
	gcc $(FLAGS_CPP) -c *.cpp
	ld -M -s -e GlobalInit -Ttext=0xC0000000 -TldScript.ld -o todo.x *.o

clean:
	
	rm *.o
	rm todo.x
	rm *.img


	

