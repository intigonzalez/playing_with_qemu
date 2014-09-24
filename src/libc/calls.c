
#include "calls.h"

/**
 * \brief Envia un mensaje de error
 */
void error(TID to, int code)
{
	LoadRegistre(0,to);
	LoadRegistre(1,4);
	LoadRegistre(2,OS_FAIL);
	LoadRegistre(3,code);
	ipc(THREAD_SEND, to);
}

/**
 * \brief Informa a un hilo del exito de una peticion
 * \param to
 */
void success(TID to)
{
	LoadRegistre(0,to);
	LoadRegistre(1,3);
	LoadRegistre(2,OS_OK);
	ipc(THREAD_SEND, to);
}

/**
 * \brief El hilo que realiza la llamada espera al menos la cantidad de
 * milisegundos indicada
 * \param milliseconds
 */
void sleep(unsigned milliseconds)
{
	// se realiza un send receive a la tarea del reloj
	LoadRegistre(0,4);
	LoadRegistre(1,3);
	LoadRegistre(2,milliseconds);
	ipc(THREAD_SND_RCV,4);
}

/**
 * \brief Escribe para un archivo un conjunto de caracteres
 * \param fd Descriptor del archivo
 * \param buff - buffer
 * \param length - longitud del buffer
 * \return 
 */
int write(int fd, const char* buff, unsigned length)
{
	// VR0 - para 5
	// VR1 - Size 
	// VR2 - operacion WRITE_CHARACTERS
	// VR3 - File Descriptor
	// VR4 - cantidad de caracteres
	// VRSimple20 hasta VRSimple(20 + VR3) - caracteres involucrados en la operacion
	
	length = (length > 240) ? 240 : length;
	unsigned size = 5 + (length / 4) + ((length % 4 != 0)? 1 : 0);
	
	LoadRegistre(0,5); // envio al FS
	LoadRegistre(1, size);
	LoadRegistre(2,WRITE_CHARACTERS);
	LoadRegistre(3, fd);
	LoadRegistre(4, length);
	for (int i = 0 ; i < length ; i++)
		LoadByteRegistre(20 + i, buff[i]);
	
	ipc(THREAD_SND_RCV,5);
	
	return StoreRegistre(2);
}

/**
 * \brief Lee una cantidad de bytes desde un archivo
 * \param fd Descriptor del archivo
 * \param buff Buffer donde se pondra lo leido
 * \param count	Cantidad de elmentos a leer
 * \return Cantidad de bytes leidos, negativo en caso de error
 */
int read(int fd, char* buff, unsigned count)
{
	// VR0 - para 5
	// VR1 - Size, 4
	// VR2 - operacion READ_CHARACTERS
	// VR3 - cantidad de elementos a leer
	count = (count > 240) ? 240 : count;
	LoadRegistre(0,5); // envio al FS
	LoadRegistre(1,5); // 5 registros seran usados
	LoadRegistre(2,READ_CHARACTERS);
	LoadRegistre(3, fd);
	LoadRegistre(4,count);
	ipc(THREAD_SND_RCV, 5);
	// al retornar la respuesta
	// VR0 - from = 5
	// VR1 - Size
	// VR2 - Cantidad leida, negativo en caso de error
	// A partir de VRSimple12 lo leido
	int result = StoreRegistre(2);
	for (int i = 0; i < result ; i++)
			buff[i] = StoreByteRegistre(12 + i);
	return result;
}

/**
 * \brief Registra al hilo que realiza la llamada como un dispositivo de caracteres
 * \param permissions Que puede hacer el dispositivo, DEVICE_WRITABLE | DEVICE_READABLE
 * \return OS_OK si todo fue bien, OS_FAIL en caso contrario
 */
int NewCharacterDeviceInSystem(unsigned permissions)
{
	LoadRegistre(0,5); // le mandaremos a 5, que es FS_THREAD
	LoadRegistre(1,5);
	LoadRegistre(2,DEVICE_INITIALITED);
	LoadRegistre(3,CHARACTER_DEVICE);
	LoadRegistre(4,permissions);
	ipc(THREAD_SND_RCV,5);
	return StoreRegistre(2);
}

/**
 * \brief Envia un byte a un puerto I/O dado
 * \param port 
 * \param value
 */
void out_byte(int port, int value)
{
	__asm__("movl	%0, %%eax;	\
	        movl %1, %%edx;	\
	        outb %%al, %%dx;"
        :
        :"r"(value),"r"(port)
			        :"%eax","%edx");
}

/**
 * \brief Envia un word (16 bits) a un puerto I/O dado
 * \param port
 * \param value
 */
void out_word(int port, int value)
{
	__asm__("movl	%0, %%eax;	\
	        movl %1, %%edx;	\
	        outw %%ax, %%dx;"
        :
        :"r"(value),"r"(port)
			        :"%eax","%edx");
}

/**
 * \brief Envia un dword (32 bits) a un puerto I/O dado
 * \param port
 * \param value
 */
void out_long(int port, int value)
{
	__asm__("movl	%0, %%eax;	\
	        movl %1, %%edx;	\
	        outl %%eax, %%dx;"
        :
        :"r"(value),"r"(port)
			        :"%eax","%edx");
}

/**
 * \brief Lee un byte de un puerto I/O dado
 * \param port
 * \return 
 */
int in_byte(int port)
{
	int v = 0;
	__asm__("movl %1, %%edx;	\
	        xor %%eax, %%eax;	\
	        inb %%dx, %%al;	\
	        movl %%eax,%0;"
        :"=r"(v)
			        :"r"(port)
			        :"%eax","%edx");
	return v;
}

/**
 * \brief Lee un word (16 bits) de un puerto I/O dado
 * \param port
 * \return 
 */
int in_word(int port)
{
	int v = 0;
	__asm__("movl %1, %%edx;	\
	        xor %%eax, %%eax;	\
	        inw %%dx, %%ax;	\
	        movl %%eax,%0;"
        :"=r"(v)
			        :"r"(port)
			        :"%eax","%edx");
	return v;
}

/**
 * \brief Lee un dword (32 bits) de un puerto I/O dado
 * \param port
 * \return 
 */
int in_long(int port)
{
	int v = 0;
	__asm__("movl %1, %%edx;	\
	        inl %%dx, %%eax;	\
	        movl %%eax,%0;"
        :"=r"(v)
			        :"r"(port)
			        :"%eax","%edx");
	return v;
}