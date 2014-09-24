INCLUDES = ../../libc
LIBRARIES = ../../libc
FLAGS = -I$(INCLUDES) -std=gnu99 -ffreestanding -nostdlib -nostdinc

all:
	gcc  $(FLAGS) -c *.c
	ld -s -L$(LIBRARIES) -lSystemCalls -TldScript.ld -o ../fs.X *.o
clean: