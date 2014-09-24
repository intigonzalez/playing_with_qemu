NOMBRE = libSystemCalls
FLAGS = -std=gnu99 -ffreestanding -nostdlib -nostdinc
all: *.c *.h *.s
	nasm -f elf video.s -o video.o
	nasm -f elf StoreAndLoad.s -o StoreAndLoad.o
	gcc -c crt0.s -o crt0.o
	gcc  $(FLAGS) -c *.c
	ld -static -o $(NOMBRE).a *.o
clean:
	rm *.o *.sa
	
