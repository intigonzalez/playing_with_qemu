
#include "includes/global.h"
#include "kernel.h"
#include "BasicMachine.h"
#include "video.h"
#include "clock.h"
#include "memory.h"
#include "elfLoader.h"
#include "pci.h"

HARDWARE_PARAMETERS _hwParameters;

unsigned params [4];


void panic(const char * text)
{
	// quito la opcion de debug
	__asm__
	( "			 	\
		 pushf;	\
		 movl (%%esp),%%eax;	\
		 andl $0xFFFFFEFF,%%eax;	\
		 movl %%eax, (%%esp); \
		 popf;"
		:
		:
		:"%eax"
	);
	printf("Ohh dios mio!!!\n");
	printf(text);
	
	while (true);
}

extern "C" void __cxa_pure_virtual()
{
    // Do nothing or print an error message.
}


static void setupHardware() {
	
	// Inicializamos el DRIVER DE VIDEO (Esto es medio risible pues todo el trabajo lo hizo el BIOS, aqui solo 
	// limpio la pantalla y pongo el cursor en su posicion)
    InitVideoDriver((unsigned char*)0xB8000 + 4080*1024*1024);
	
    // PASO 1: PREPARAR LA MAQUINA PARA EJECUTAR PROCESOS EN UN AMBIENTE MULTITAREA
    // Inicialiazamos la tabla de descriptores globales
    BasicMachine::initGDT();
	
    // Inicializamos la tabla de interrupciones, con todos los maniuladores a un manipulador nulo que no hace nada
    BasicMachine::initInterruptTable();
	
	// inicializamos el bus pci (en realidad es enumerarlo, aunque no se para que 
	// porque estoy considerando maquinas IBM compatibles)
	enumerate_pci_devices();
	
    // Inicializamos el PIC, para manejar las interrupciones de los dispositivos de HARDWARE
    BasicMachine::init8259A();
	
    //Inicializamos el gestor de memoria
    InitMemory(&_hwParameters);

    // Inicializamos el reloj, ya tenemos algo de interrupciones y la base de la multitarea, antes de esto debemos preparar la tabla de procesos
    InitClockDriver();

	// Iniciamos algunas IRQ 
	//BasicMachine::enable_irq(AT_WINI_IRQ);
	//BasicMachine::enable_irq(KEYBOARD_IRQ);
}


/********************************************************************
 * ESTA FUNCION ES EL PUNTO DE ENTRADA AL KERNEL
 ********************************************************************/
int main(int argc, char* argv[]) {

	_hwParameters.kernel_page_directory = params[0];
	_hwParameters.memoryCount = params[1];
	_hwParameters.modules[0] = (void*)params[2];
	_hwParameters.modules[1] = (void*)params[3]; 
	
    setupHardware();
	
	//printf("feo fe %x\n", _hwParameters.kernel_page_directory);
	
    // analizo el modulo y lo pongo en el lugar que va
    LoadRootServer(_hwParameters.modules[0]);
	
	BasicMachine::set_low_level_handler(0x64,(int)&low_level_sys_call);
	
	printf("La direccion de _save : %x\n", &_save);
	printf("La direccion de _restore : %x\n", &_restore);
	//panic("");

	printf("Adelante!!!!!!\n");
	
    activate_multitasking(); 

    while (true);

    return 0;
}
