#include "system_calls.h"



/**
 * Esta funcion es usada para realizar una llamada generica
 * \param type Es un numero con el numero de la llamada
 * \return Devuelve 0 o menor en caso de fallo
 * */
int do_call(unsigned type)
{
	int v = 0;
	__asm__("movl %1,%%eax;		\
					 int $0x64;				\
					 movl %%eax,%0;		"
					 :"=r"(v)
					 :"r"(type)
					 :"%eax");
	return v;
}

/**
 * Esta llamada crea un hilo nuevo en el mismo espacio de direcciones
 * del hilo que realiza la llamada, el nuevo hilo comienza la ejecucion
 * en la posicion dada por eip y con pila esp.
 * El hilo que realiza la llamada es responsable de proveer direcciones 
 * correctas reservando espacio para ellas
 * \param eip Es la direccion donde se comienza a ejecutar el hilo
 * \param esp Es la direccion donde comienza el apuntador de la pila
 * \return El id del hilo creado si no existio ningun problema, 0 o negativo
 * en caso contrario
 * */
int ThreadControl(TIPO eip, void* esp, void* utcb)
{
	// en eax ponemos el tipo de llamada
	// en ebx ponemos el eip
	// en ecx ponemos el esp
	// en edx ponemos la direccion del area utcb
	// en eax ponemos el valor que retorna la llamada
	int v = 0;
	int type = THREAD_CONTROL;
	__asm__("movl %3,%%eax;		\
					 movl	%1,%%ebx;		\
					 movl %2,%%ecx;		\
					 movl %4,%%edx;		\
					 int $0x64;				\
					 movl %%eax,%0;		"
					 :"=r"(v)
					 :"r"(eip),"r"(esp),"m"(type),"m"(utcb)
					 :"%eax","%ebx","%ecx","%edx");
	return v;
}

/**
 * \brief Con esta llamada un hilo informa que desea manejar una interrupcion
 * El hilo debera esperar por mensajes
 * \param interrupt Es el numero de la interrupcion que deseamos manejar
 * \return Un codigo de error
 */
int InterruptControl(unsigned interrupt)
{
	// en eax ponemos el tipo de llamada
	// en ebx ponemos el numero de la interrupcion que deseamos manipular
	int type = INTERRUPT_CONTROL;
	int v = 0;
	__asm__("movl %2,%%eax;		\
					 movl	%1,%%ebx;		\
					 int $0x64;				\
					 movl %%eax,%0;		"
					 :"=r"(v)
					 :"r"(interrupt),"r"(type)
					 :"%eax","%ebx");
	return v;
}

/**
 * \brief Realiza una ipc (interprocess comunication) de forma sincrona
 * \param ACTION - Send, Receive o Send and Receive
 * \param id - a quien le envio, de quien recibo
 * \return Un codigo de error, 0 en caso contrario
 */
int ipc(unsigned ACTION, unsigned id)
{
	// en eax ponemos el tipo
	// en ebx la accion
	// en ecx el id
	int type = IPC;
	int v = 0;
	__asm__("movl %1,%%ebx;		\
					 movl %2,%%ecx;		\
					 movl %3,%%eax;		\
					 int $0x64		;		\
					 movl	%%eax,%0;		"
					 :"=r"(v)
					 :"r"(ACTION),"r"(id),"r"(type)
					 :"%eax","%ebx","%ecx");
	
}
