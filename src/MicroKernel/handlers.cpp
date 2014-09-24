


#include "includes/global.h"
#include "kernel.h"

#include <system_calls.h>

#include "video.h"
#include "ThreadControl.h"


/**
 * 
 * */
static const char * messages[] ={
    "0 - Divide Error",
    "1 - Debug Exception",
    "2 - NMI Interrupt",
    "3 - Breakpoint",
    "4 - Overflow",
    "5 - Bound Range Exceeded",
    "6 - Invalid Opcode",
    "7 - Device not available",
    "8 - Double Fault",
    "9 - Co-processor segment",
    "10 - Invalid TSS",
    "11 - Segment not Present",
    "12 - Stack Segment-Fault",
    "13 - General Protection",
    "14 - Page Fault",
    "15 - Intel Reserved",
    "16 - x87 FPU error",
    "17 - Alignment Check",
    "18 - Machine Check",
    "19 - SIMD Floating Point"

};

/**
 * Los id de los hilos para realizar llamadas, este arreglo contiene el hilo
 * que debe ser interrumpido cuando se produce una excepcion
 * 0 es valor que indica que ningun hilo debe ser interrumpido.
 * Los valores de 0 son asignados por el GRUB, Creo.
 * */
static TID _ids[256];
 
/**
 * \brief Envia un ipc al hilo que se encarga de manejar una interrupcion en especifico
 * \param value - Que interrupcion es
 */
int DoThreadInterrupt(const unsigned char value)
{
	//printf("Ocurrio la interrupcion %x",value);
	if (_ids[value] == 0)	return 0;
	
	if (!threadControl.SendHardwareIPC(_ids[value], value))
	{
		_ids[value] = 0;
		return 0;
	}
	return 1;
}

/**
 * \brief Establece que hilo debe interrumpirse cuando ocurre una interrupcion dada
 * \param interrupt - Interrupcion
 * \param thread - Id del hilo a interrumpir
 */
void RegistreThreadToInterrupt(const unsigned char interrupt, TID thread)
{
	
	_ids[interrupt] = thread;
}

/**
 * \brief 
 * \param cs
 * \param eip
 */
static void debug_exception(unsigned cs, unsigned eip)
{
	printf("Debugging step at %x:%x\n",cs,eip);
}

static void page_fault(unsigned cs, unsigned eip,unsigned error_code)
{
	printf("%s\n Estado :%x:%x\nError Code : %x\n",messages[14],cs,eip,error_code);
	
	int addr;
	__asm__("movl %%cr2,%%eax;	\
					 movl %%eax,%0;"
					 :"=r"(addr)
					 :
					 :"%eax");
	if (error_code & 0x01)
		printf("The fault was caused by a page-level protection violation.\n");
	else
		printf("The fault was caused by a non-present page.\n");
	if (error_code & 0x02)
		printf("The access causing the fault was a write.\n");
	else
		printf("The access causing the fault was a read.\n");
	if (error_code & 0x04)
		printf("The access causing the fault originated when the processor was executing in user mode.\n");
	else
		printf("The access causing the fault originated when the processor was executing in supervisor mode.\n");		
	if (error_code & 0x08)
		printf("The fault was caused by a page-level protection violation.\n");
	else
		printf("The fault was not caused by reserved bit violation.\n");
		
	if (error_code & 0x10)
		printf("The fault was caused by an instruction fetch.\n");
	else
		printf("The fault was not caused by an instruction fetch.\n");
	printf("La direccion que causo el fallo de pagina fue %x \n",addr);
	while (1);
}

/**
 * \brief Esta funcion es usada para las interrupciones de 0 a 19, es decir las 
 * producidas dentro del procesador, SEGmentation Fault, Page Fault y esas
 * \param number - El numero de la interrupcion
 * \param cs - El segmento de codigo donde se ejecutaba
 * \param eip - El eip donde se ejecutaba al ocurrir la interrupcion
 * \param error_code - El codigo de error si existe
 */
void high_level_handler_faults(unsigned number, unsigned cs, unsigned eip, unsigned error_code)
{
	switch (number)
	{
		case 1:
			debug_exception(cs,eip);
		break;
		case 0:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 15:
		case 16:
		case 17:
		case 18:
		case 19:
			printf("%s\n Estado :%x:%x\nError Code : %x\n",messages[number],cs,eip,error_code);
			
			if (eip == 0xAC)
			{
				printf("Que cosa mas loca (el hilo es %d)\n",activeThread->tid);
			}
			
			while (true);
		break;
		case 14: // Page Fault
			page_fault(cs,eip,error_code);
		break;
		default:
			printf("Esto que cosa es %s(%d)\n",__FILE__,__LINE__);
			while (true);
		break;
	}
}

/**
 * \brief Esta funcion es el handler general de toda interrupcion
 * \param number - Numero de la interrupcion ocurrida
 */
void high_level_handler(unsigned number)
{
	if (DoThreadInterrupt(number)) return;
	// Ocurre una interrupcion, debo atenderla
	printf("Interrupt Handler At %x\n",number);
	while (true);
}

/**
 * Es llamada por el procedimiento de atencion a la interrupcion SYS_CALL_VECTOR
 * es la forma en que los hilos realizan peticiones al microkernel
 */
void microkernel_syscall(unsigned type)
{
	if (type == 43){
		printf("Ha entrado correctamenta a FastSysCall\n");
		return;
	}
	switch (activeThread->eax)
	{
		case THREAD_CONTROL: // ThreadControl
			//printf("Se produce una ThreadControl\n");
			//printf("Area utcb creada %x\n", activeThread->edx);
			threadControl.CreateThread(activeThread->ebx,activeThread->ecx,
																	activeThread->edx,activeThread->page_directory);
			//printf("ThreadControl Done!!!\n");
		break;
		case INTERRUPT_CONTROL: // InterruptControl
			//printf("\n\nOcurre un registro%x %d\n\n",activeThread->ebx,activeThread->tid);
			RegistreThreadToInterrupt(activeThread->ebx,activeThread->tid);
		break;
		case IPC:	// IPC
			threadControl.ipc(activeThread->ebx, activeThread->ecx, activeThread);
		break;
		default:
			//printf("Calls %d\n",activeThread->eax);
		break;
	}
	
}
