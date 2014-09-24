#include <system_calls.h>
#include "kerneltasks.h"


BootInfo _bootInfo = 
{
	0,0,0,0,0
};

pBootInfo pbootInfo = &_bootInfo;


char pilaDelReloj[512];
char utcbReloj[UTCB_AREA_SIZE];
typedef struct 
{
	unsigned id;
	int amount;
} EntradaEspera;
EntradaEspera cuantos[10];
int cantidad = 0;
void HiloDelReloj(void * utcb)
{
	// informo al microkernel que yo atiendo las interrupciones del reloj
	InterruptControl(0x30  /*CLOCK_IRQ + IRQ0_VECTOR*/);
	// ahora recibo mensajes de otros hilos o del hardware
	int value,t2,i,j;
	value = 56;
	while (1)
	{
		// espero de cualquiera
		ipc(THREAD_RECEIVE,(cantidad < 10)?ANY_SOURCE:HARDWARE_SOURCE);
		value = StoreRegistre(0);
		if (value <= 2)
		{
			// es el hardware, debo comprobar si alguno ya esta listo
			for (i = 0, j = 0; i < cantidad ; i++)
			{
				if (cuantos[i].amount - 50 > 0)
				{
					cuantos[j].amount = cuantos[i].amount - 50;
					cuantos[j].id = cuantos[i].id;
					j++;
				}
				else
				{
					LoadRegistre(0,cuantos[i].id);
					LoadRegistre(1,2);
					ipc(THREAD_SEND,cuantos[i].id);
				}
			}
			cantidad = j;
		}
		else 
		{
			// inserto una nueva entrada
			cuantos[cantidad].amount = StoreRegistre(2);
			cuantos[cantidad].id = value;
			cantidad++;
		}
		
		
	}
}


int OSInit(unsigned int EndOfThisImage)
{
	/*
	unsigned *vidmem = (unsigned*)pbootInfo->videoMemory;
	int v = '0';
	
	do {
		PassToKernelMode((unsigned)&EtiquetaDeRetorno);
		
		// Clear visible video memory
		for (long loop=320; loop<1000; loop++)
			vidmem[loop] = 0x0A000000 + (v << 16) + 0x0A00 + v;
		v++;
		if (v > '9') v = '0';
		
	}
	while(1);
	*/
	

	// Inicializamos el hilo del reloj
	ThreadControl(&_HiloDelReloj,&pilaDelReloj[512],&utcbReloj[0]); // este es 4
	
	// Inicializamos el hilo del FS
	Init_FS_Control_Thread(); // este es 5
	
	// Inicializanmos el hilo de la consola
	Init_Console_Thread();	// 6
	
	// Inicializamos el hilo del control de Discos
	//Init_AT_Control_Thread();
	
	// Inicializamos hilo de pruebas
	Init_Test_Thread();		// 7

	
	while (1)
	{
		ipc(THREAD_RECEIVE,4);
		
	}
		
	// este el programa
	return 0;
}
