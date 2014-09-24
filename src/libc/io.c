#include "system_calls.h"
#include "calls.h"


typedef struct
{
	char buff[100];
	int length;
} Buffer , *pBuffer;

static void printChar(char c, pBuffer buffer)
{
	buffer->buff[buffer->length++] = c;
	//write(0,&c,1);
}

/*
 * Esta funcion imprime una cadena de caracteres, es utilizada solo por el kernel
 */
static void print(const char* msg, pBuffer buffer)
{
	unsigned long i;
	// Continue until we reach null character
	i = 0;
	while (msg[i] != 0)
		printChar(msg[i++],buffer);
	
}

/**
 * Escribe hacia la pantalla un numero dado en una base menor o igual que 16
 * @param number    Numero a mostrar
 * @param base Una base menor o igual que 16
 */
static void printNumber(unsigned number, int base, pBuffer buffer)
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
			printChar(s[i],buffer);

}

void printf2(const char* format, int* first)
{
	Buffer b;
	b.length = 0;
    // esto es para imprimir una cadena con formato
    int j = 0;
    int c = 0;
    int value;
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
                        printNumber(value,10,&b);
                        break;
                    case 'x':
                        // es un numero en hexadecimal
                        value = *first;
						//char* msg = {'0','x',0};
                        //print(msg,&b);
						printChar('0',&b);
						printChar('x',&b);
                        printNumber(value,16,&b);
                        break;
                    case 's':
                        // es una cadena
                        value = *first;
                        cad = ((char*) value);
                        print(cad,&b);
                        break;
                    default:
                        printChar(format[j],&b);
                        break;
                }
                first++;
                c++;
                
                break;
            default:
                printChar(format[j],&b);
                break;
        }

        j++;
    }
		//b.buff[b.length] = 0;
		write(0,b.buff,b.length);
}