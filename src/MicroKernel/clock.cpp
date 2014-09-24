#include "clock.h"
#include "video.h"
#include "BasicMachine.h"
#include "ThreadControl.h"

#define CLOCK_PORT	0x43	// el puerto de acceso al reloj
#define CLOCK_COUNTER0 0x40	// el puerto de acceso al contador del reloj


static bool iniciado; // indica si el reloj ha sido inicializado
static unsigned long ticks_desde_inicio; // un contador con los ticks que han ocurrido desde el inicio del sistema


// EL HANDLER DE LAS INTERRUPCIONES

void high_level_clock_handler() {
	//if (ticks_desde_inicio % 100 == 0)
	//s	printf("\n\n================================\n\n");
	
    // Envio un mensaje al hilo encargado de gestionar las interrupciones del reloj
	DoThreadInterrupt(CLOCK_IRQ + IRQ0_VECTOR);
	// Realizo la planificacion de los hilos
	threadControl.shedule();
    // aumento la cantidad de ticks ocurridos desde el inicio del sistema
    ticks_desde_inicio++;
}

void InitClockDriver() {
    iniciado = false;
    ticks_desde_inicio = 0;

    iniciado = true; 

    int value = 59659; // esto son 20 tick por segundo(ARREGLAR ESTO)

    BasicMachine::out_byte(CLOCK_PORT, 0x34);
    BasicMachine::out_byte(CLOCK_COUNTER0, value);
    BasicMachine::out_byte(CLOCK_COUNTER0, value >> 8);

    // habilito esa irq
    BasicMachine::set_low_level_handler(CLOCK_IRQ + IRQ0_VECTOR, (int) &low_level_clock_handler);
    BasicMachine::enable_irq(CLOCK_IRQ);
}

