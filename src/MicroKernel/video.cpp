#include "video.h"
#include "BasicMachine.h"

int cursorRow = 0;
int cursorCol = 0;

unsigned char *VIDMEM;

/**
 * Esta funcion realiza un scroll de la pantalla para dejar una linea libre al final
 */
static void scroll()
{
	unsigned long i;
	unsigned char *vidmem = VIDMEM;
	i = 25 * 80 - 80;
	while (i){
		vidmem[0] = (vidmem[160]);
		vidmem+=2;
		i --;
	}

	// la ultima fila se limpia
	vidmem = (unsigned char *)((unsigned)VIDMEM + (80*25 - 80)*2);
	for (i = 0 ; i < 80 ; i++)
		vidmem[i*2] = ' ';
}

/*
 * Esta funcion imprime un caracter hacia la pantalla
 */
static void printChar(char c)
{
	unsigned short offset;
	unsigned long i;
	unsigned char *vidmem = VIDMEM;
	offset = cursorRow * 80 + cursorCol;

	// Start at writing at cursor position
	vidmem += offset*2;
	switch (c){
		case '\n':
			cursorCol = 0;
			if (cursorRow < 24)
				cursorRow ++;
			else{
				// corro todo
				scroll();
			}
			return;
		case '\t':
			for (i = 0 ; i < 7 ; i++)
				printChar(' ');
			return;
		default:
			*vidmem = c;
			offset ++;
	}
	cursorRow = offset / 80;
	cursorCol = offset % 80;

	if (offset >= 25*80){
		scroll();
		cursorRow = 24;
		cursorCol = 0;
	}
}



/*
 * Esta funcion imprime una cadena de caracteres, es utilizada solo por el kernel
 */
static void print(const char* msg)
{
	unsigned short offset;
	unsigned long i;

	offset = cursorRow * 80 + cursorCol;

	// Continue until we reach null character
	i = 0;
	while (msg[i] != 0)
		printChar(msg[i++]);
}


/**
 * Escribe hacia la pantalla un numero dado en una base menor o igual que 16
 * @param number    Numero a mostrar
 * @param base Una base menor o igual que 16
 */
static void printNumber(unsigned number,int base)
{
	char s[10];
	int c = 0;
	int neg = number < 0;
	if (neg) number = -number;

	while (number != 0)
	{
		char u = (char)(number % base);
                if (u > 9)
                    s[c++] = (u - 10) + 'A';
                else
                    s[c++] = u + '0';
		number = number / base;
	}
	if (c == 0)
		s[c++] = '0';
        if (neg)
            s[c++]='-';

	s[c] = 0;

	for (int i = c-1 ; i >= 0 ; i--)
			printChar(s[i]);
}

void printf2(const char* format, int* first)
{
    // esto es para imprimir una cadena con formato
    int j = 0;
    int c = 0;
    int value,offset;
    char* cad;
    while (format[j]!=0)
    {
        switch (format[j])
        {
            case '%':
                j++;
                switch (format[j])
                {
                    case 'd':
                        // es un numero
                        value = *first;
                        printNumber(value,10);
                        break;
                    case 'x':
                        // es un numero en hexadecimal
                        value = *first;
                        print("0x");
                        printNumber(value,16);
                        break;
                    case 's':
                        // es una cadena
                        value = *first;
                        cad = ((char*) value);
                        print(cad);
                        break;
                    default:
                        printChar(format[j]);
                        break;
                }
                first++;
                c++;
                break;
            default:
                printChar(format[j]);
                break;
        }
        j++;
    }
		
	offset = cursorRow * 80 + cursorCol;

	BasicMachine::out_byte(0x3D4, 15);
	BasicMachine::out_byte(0x3D5, (unsigned char)(offset));
	BasicMachine::out_byte(0x3D4, 14);
	BasicMachine::out_byte(0x3D5, (unsigned char)(offset >> 8));
}



/*
 * Inicializa la memoria de video limpiandola
 */
void InitVideoDriver(unsigned char * video)
{
	VIDMEM = video;
	// LIMPIO LA PANTALLA
	unsigned char *vidmem = VIDMEM;
	const long size = 80*25;

	// Clear visible video memory
	for (long loop=0; loop<size; loop++) {
		*vidmem++ = 0;
		*vidmem++ = 0x0D;
	}
	cursorRow = 0;
	cursorCol = 0;
}


