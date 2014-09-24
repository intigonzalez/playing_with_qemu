FLAGS_CPP = -I$(INCLUDES) -g -Wall -Wextra -nostdlib -fno-builtin -nostartfiles -nodefaultlibs -fno-exceptions -fno-rtti -fno-stack-protector
all:	myLoader.img
	objcopy -O elf32-i386 myLoader.img

myLoader.img: main.cpp
	gcc -c crt0.s -o crt0.o
	nasm -f elf utils.s -o utils.o
	gcc $(FLAGS_CPP) -c *.cpp
	ld -s -e GlobalInit -Ttext=0x00200000 -TldScript.ld -o myLoader.img *.o

clean:
	rm *.o
	rm todo.x