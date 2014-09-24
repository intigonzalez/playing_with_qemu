#ifndef BASICMCHINE_H_
#define BASICMCHINE_H_

#include "includes/global.h"
#include "kernel.h"

// puertos para acceder al PIC(programable interrupt controller)
#define PIC_MASTER_PORT1 0x20
#define PIC_MASTER_PORT2 0x21
#define PIC_SLAVE_PORT1 0xA0
#define PIC_SLAVE_PORT2 0xA1

// palabras de inicializacion del PIC
#define ICW1_AT         0x11	/* edge triggered, cascade, need ICW4 */
#define ICW1_PC         0x13	/* edge triggered, no cascade, need ICW4 */
#define ICW1_PS         0x19	/* level triggered, cascade, need ICW4 */
#define ICW4_AT         0x01	/* not SFNM, not buffered, normal EOI, 8086 */
#define ICW4_PC         0x09	/* not SFNM, buffered, normal EOI, 8086 */

//#pragma pack(push,1) // alineacion a 1 byte

/*
 * Esta estructura representa una puerta a un manipulador de interrupciones
 */
struct InterruptGate{
	unsigned short offset0_15; // primeros 16 bits de la direccion de memoria virtual
	unsigned short selector; // selector de segmento donde se encuentra el codigo de manipulacion
	unsigned short flags; // control de acceso 
	unsigned short offset16_31; // los 16 bits restantes de la direccion
}; 

/** 
 * Estructura que representa un descriptor de segmento segun el 80x86
 * En el sistema, siguiendo el modelo de memoria FLAT con paginacion se tienen 6 descriptores
 * */
struct SegmentDescriptor{
		unsigned int v1;
		unsigned int v2;
};


//#pragma pack(pop)
class BasicMachine
{
public:
	
	static void initGDT();
	static void init8259A(void);
	static void initInterruptTable();
	static void set_low_level_handler(int interrupt, int handler);

	static void setup_sysenter_sysexit(int cs0_selector,unsigned ip, unsigned esp);

	static void set_utcb_area(unsigned address);

	static void enable_irq(int irq);
	static void disable_irq(int irq);

	static void out_byte(int port , int value);
	static void out_word(int port , int value);
	static void out_long(int port , int value);

	static int in_byte(int port);
	static int in_word(int port);
	static int in_long(int port);
};


#endif /*BASICMCHINE_H_*/
