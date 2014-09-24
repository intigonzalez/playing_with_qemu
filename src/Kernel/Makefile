ROOT_SERVER = OS
INCLUDES = ../libc
LIBRARIES = ../libc
FLAGS = -I$(INCLUDES) -std=gnu99 -ffreestanding -nostdlib -nostdinc

all: $(ROOT_SERVER).x servers
	
	objcopy -O elf32-i386 $(ROOT_SERVER).x
	cp $(ROOT_SERVER).x $(ROOT_SERVER).img

$(ROOT_SERVER).x: *.c *.s ../libc/*.a
	nasm -f elf crt0.s -o crt0.o 
	gcc -c OsThread.s -o OsThread.o
	gcc  $(FLAGS) -c *.c
	ld -s -e ENTRADA -Ttext 0x00000000 -L$(LIBRARIES) -lSystemCalls -TldScript.ld -o $(ROOT_SERVER).x *.o
	#ld -s -e main -Ttext 0x00000000 -o $(ROOT_SERVER).x $(ROOT_SERVER).o
	
servers:
	cd FS && $(MAKE)

clean:
	
	rm *.o
	rm *.x
	rm *.img
