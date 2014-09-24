#include "kernel.h"
#include "BasicMachine.h"
#include "tss.h"
#include "video.h"
#include "memory.h"




// esto lo pongo tan solo porque el procesador en modo protegido lo necesita
// es la memoria del kernel cuando se produce una interrupcion
static char kernelStackMemoryForFastSysCall[500];
static char kernelStackMemoryForInterrupt[500];
static TaskStateSegment tasksegment = {0, (unsigned long) &kernelStackMemoryForInterrupt[499], KERNEL_DATA_SELECTOR
                                       , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
                                       , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
                                      };


static struct InterruptGate interruptsHandlers[256]; // en el 80x86 existen exactamente 256 posibles interrupciones
static struct SegmentDescriptor descriptors[9]; // nulo, KERNEL_CODE , KERNEL_DATA , USER_CODE , USER_DATA, TSS, UTCB Area

#pragma pack(push,1)

struct systemTable {
	unsigned char limit0;
	unsigned char limit1;
	unsigned char address0;
	unsigned char address1;
	unsigned char address2;
	unsigned char address3;
};

#pragma pack(pop)

typedef void (*HANDLER)();

HANDLER low_level_handlers[] = {
	low_level_0,
	low_level_1,
	low_level_2,
	low_level_3,
	low_level_4,
	low_level_5,
	low_level_6,
	low_level_7,
	low_level_8,
	low_level_9,
	low_level_10,
	low_level_11,
	low_level_12,
	low_level_13,
	low_level_14,
	low_level_15,
	low_level_16,
	low_level_17,
	low_level_18,
	low_level_19
};

/**
 * \brief
 * \param handlerOffset
 */
void BasicMachine::initInterruptTable()
{

	int i, a;
	struct systemTable idt_descriptor;
	for (i = 0 ; i < 20 ; i++) {
		interruptsHandlers[i].selector = KERNEL_CODE_SELECTOR;
		interruptsHandlers[i].offset0_15 = ((unsigned)low_level_handlers[i]) & 0x0000FFFF;
		interruptsHandlers[i].offset16_31 = (((unsigned)low_level_handlers[i]) & 0xFFFF0000) >> 16;
		if (i == SYS_CALL_VECTOR)
			interruptsHandlers[i].flags = 0xEE00; // interrupt gate ,DPL = 3, se cancelan las interrupciones
		else
			interruptsHandlers[i].flags = 0x8E00; // interrupt gate ,DPL = 0 , se cancelan las interrupciones

	}

	printf("%d es %x\n",0,(unsigned)GetBasicHandler(0));
	
	for (i = 20; i < 256; i++) {
		unsigned handlerTemp = (unsigned)GetBasicHandler(i);
		//if (i < 45)
			
		interruptsHandlers[i].selector = KERNEL_CODE_SELECTOR;
		interruptsHandlers[i].offset0_15 = (handlerTemp) & 0x0000FFFF;
		interruptsHandlers[i].offset16_31 = ((handlerTemp) & 0xFFFF0000) >> 16;
		if (i == SYS_CALL_VECTOR)
			interruptsHandlers[i].flags = 0xEE00; // interrupt gate ,DPL = 3, se cancelan las interrupciones
		else
			interruptsHandlers[i].flags = 0x8E00; // interrupt gate ,DPL = 0 , se cancelan las interrupciones
	}

	// ahora cargo la tabla de descriptores
	a = 256 * 8 - 1;
	idt_descriptor.limit0 = (unsigned char) (a & 0xFF); // 256 entradas
	idt_descriptor.limit1 = (unsigned char) ((a & 0xFF00) >> 8);
	a = (int) &interruptsHandlers;
	idt_descriptor.address0 = (a & 0xFF);
	idt_descriptor.address1 = (a & 0xFF00) >> 8;
	idt_descriptor.address2 = (a & 0xFF0000) >> 16;
	idt_descriptor.address3 = (a & 0xFF000000) >> 24;

	set_new_idt(&idt_descriptor);
}

/**
 *
 * */
void BasicMachine::initGDT()
{
	struct systemTable gdt_descriptor;
	// relleno todos los campos
	// DESCRIPTOR 0 : NULO
	descriptors[0].v1 = 0;
	descriptors[0].v2 = 0;
	// DESCRIPTOR 1 : DESCRIPTOR PARA EL CODIGO DEL KERNEL
	descriptors[1].v1 = 0x0000FFFF;
	descriptors[1].v2 = 0x00CF9B00;
	// DESCRIPTOR 2 : DESCRIPTOR PARA LOS DATOS DEL KERNEL
	descriptors[2].v1 = 0x0000FFFF;
	descriptors[2].v2 = 0x00CF9300;

	// DESCRIPTOR 3 : DESCRIPTOR PARA EL CODIGO DEL USUARIO (base 0 mb)
	descriptors[3].v1 = 0x0000FFFF;
	descriptors[3].v2 = 0x00CFFA00;  // le puse DPL(Descriptor Privile Level) = 3
	// DESCRIPTOR 4 : DESCRIPTOR PARA LOS DATOS DEL USUARIO (base 0 mb)
	descriptors[4].v1 = 0x0000FFFF;
	descriptors[4].v2 = 0x00CFF200; // le puse DPL(Descriptor Privile Level) = 3
	/*
	    descriptors[3].v1 = (0x0000FFFF | (USER_BASE_ADDRESS << 16));
	    descriptors[3].v2 = (0x00CFFA00 |
	            (USER_BASE_ADDRESS & 0xFF000000) | ((USER_BASE_ADDRESS & 0x00FF0000)>>16) ); // le puse DPL(Descriptor Privile Level) = 3
	    // DESCRIPTOR 4 : DESCRIPTOR PARA LOS DATOS DEL USUARIO (base USER_BASE_ADDRESS mb)
	    descriptors[4].v1 = (0x0000FFFF | (USER_BASE_ADDRESS << 16));
	    descriptors[4].v2 = (0x00CFF200 |
	                    (USER_BASE_ADDRESS & 0xFF000000) | ((USER_BASE_ADDRESS & 0x00FF0000)>>16) ); // le puse DPL(Descriptor Privile Level) = 3
	*/
	// DESCRIPTOR 5 : DESCRIPTOR PARA TSS
	unsigned long addr = (unsigned long) &tasksegment;
	descriptors[5].v1 = ((addr & 0xFFFF) << 16) | 0x00000067;
	descriptors[5].v2 = 0x0000E900;
	descriptors[5].v2 |= (addr & 0x00FF0000) >> 16;
	descriptors[5].v2 |= (addr & 0xFF000000);
	
	// DESCRIPTOR 1 : DESCRIPTOR PARA EL CODIGO DEL HILO IDLE
	descriptors[7].v1 = 0x0000FFFF;
	descriptors[7].v2 = 0x00CFBA00;
	// DESCRIPTOR 2 : DESCRIPTOR PARA DATOS DEL HILO IDLE
	descriptors[8].v1 = 0x0000FFFF;
	descriptors[8].v2 = 0x00CFB200;

	// establezco el GDT_DESCRIPTOR
	gdt_descriptor.limit0 = 72 - 1; // tamaño menos 1
	gdt_descriptor.limit1 = 0;
	int a = (int) &descriptors[0];
	gdt_descriptor.address0 = a & 0xFF;
	gdt_descriptor.address1 = (a & 0xFF00) >> 8;
	gdt_descriptor.address2 = (a & 0xFF0000) >> 16;
	gdt_descriptor.address3 = (a & 0xFF000000) >> 24;

	// pongo la nueva gdt
	set_new_gdt(&gdt_descriptor);
	set_new_tss(TSS_SELECTOR);
	BasicMachine::setup_sysenter_sysexit(KERNEL_CODE_SELECTOR,(unsigned)&FastSysCall,(unsigned)&kernelStackMemoryForFastSysCall[499]);
	printf("La direccion de FastSysCall es : %x\n", &FastSysCall);
	//while(1);
}

void BasicMachine::set_utcb_area(unsigned address)
{
	//address += USER_BASE_ADDRESS;
	//printf("Direccion %x\n", address);
	// DESCRIPTOR 6 : DESCRIPTOR PARA el UTCB (base USER_BASE_ADDRESS mb)
	descriptors[6].v1 = ((address << 16) | 0x00000100); // el tamaño de esta area es siempre de 256 bytes
	unsigned temp = address & 0xFF000000;
	address = (address & 0x00FF0000) >> 16;
	descriptors[6].v2 = 0x0040F200 | temp | address; // le puse DPL(Descriptor Privile Level) = 3
}

void BasicMachine::init8259A(void)
{
	/* Initialize the 8259s, finishing with all interrupts disabled.
	 */
	/* The AT and newer PS/2 have two interrupt controllers, one master,
	 * one slaved at IRQ 2.  (We don't have to deal with the PC that
	 * has just one controller, because it must run in real mode.)
	 */

	// PASO 1: Comando de INICIALIZACION PARA PIC MASTER (siempre trabajo para AT)
	// suponiendo maquinas modernas
	out_byte(PIC_MASTER_PORT1, ICW1_AT);
	// PASO 2: palabra de inicializacion ICW2 (cual es el vector de interrupciones)
	// para la IRQ0 del master
	out_byte(PIC_MASTER_PORT2, IRQ0_VECTOR); /* ICW2 for master */
	// PASO 3: Le digo al master que irq uso para la cascada con el esclavo
	out_byte(PIC_MASTER_PORT2, (1 << CASCADE_IRQ)); /* ICW3 tells slaves */
	out_byte(PIC_MASTER_PORT2, ICW4_AT);

	// PASO 5: disable las interrupciones de la 0 a 7 obviando la 2
	out_byte(PIC_MASTER_PORT2, ~(1 << CASCADE_IRQ)); /* IRQ 0-7 mask */

	// los pasos se repiten para el PIC esclavo
	out_byte(PIC_SLAVE_PORT1, ICW1_AT);
	out_byte(PIC_SLAVE_PORT2, IRQ8_VECTOR);
	/* ICW2 for slave */
	out_byte(PIC_SLAVE_PORT2, CASCADE_IRQ); /* ICW3 is slave nr */
	out_byte(PIC_SLAVE_PORT2, ICW4_AT);
	out_byte(PIC_SLAVE_PORT2, ~0); /* IRQ 8-15 mask */

}

void BasicMachine::enable_irq(int irq)
{
	unsigned char valor = 0;
	lock_interrupts();


	if (irq < 8) {
		valor = in_byte(PIC_MASTER_PORT2);
		valor &= ~(1 << irq);
		out_byte(PIC_MASTER_PORT2, valor);
	} else {
		// habilito la salida en el master
		//enable_irq(CASCADE_IRQ);
		irq -= 8;
		valor = in_byte(PIC_SLAVE_PORT2);
		valor &= ~(1 << irq);
		out_byte(PIC_SLAVE_PORT2, valor);
		irq += 8;
	}
	/*printNumber(irq);
	valor = in_byte(PIC_MASTER_PORT2);
	printNumber(valor);
	valor = in_byte(PIC_SLAVE_PORT2);
	printNumber(valor);

	 */
}

void BasicMachine::disable_irq(int irq)
{
	unsigned char valor = 0;
	lock_interrupts();
	if (irq < 8) {
		valor = in_byte(PIC_MASTER_PORT2);
		valor |= (1 << irq);
		out_byte(PIC_MASTER_PORT2, valor);
	} else {
		irq -= 8;
		valor = in_byte(PIC_SLAVE_PORT2);
		valor |= (1 << irq);
		out_byte(PIC_SLAVE_PORT2, valor);

	}
}

void BasicMachine::set_low_level_handler(int interrupt, int handler)
{
	lock_interrupts();
	interruptsHandlers[interrupt].offset0_15 = (handler & 0xFFFF);
	interruptsHandlers[interrupt].offset16_31 = (handler & 0xFFFF0000) >> 16;
}

void BasicMachine::out_byte(int port, int value)
{
	__asm__("movl	%0, %%eax;	\
	        movl %1, %%edx;	\
	        outb %%al, %%dx;"
        :
        :"r"(value),"r"(port)
			        :"%eax","%edx");
}

void BasicMachine::out_word(int port, int value)
{
	__asm__("movl	%0, %%eax;	\
	        movl %1, %%edx;	\
	        outw %%ax, %%dx;"
        :
        :"r"(value),"r"(port)
			        :"%eax","%edx");
}

void BasicMachine::out_long(int port, int value)
{
	__asm__("movl	%0, %%eax;	\
	        movl %1, %%edx;	\
	        outl %%eax, %%dx;"
        :
        :"r"(value),"r"(port)
			        :"%eax","%edx");
}

int BasicMachine::in_byte(int port)
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

int BasicMachine::in_word(int port)
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

int BasicMachine::in_long(int port)
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

void BasicMachine::setup_sysenter_sysexit(int cs0_selector, unsigned ip, unsigned esp)
{
	//#define MSR_IA32_SYSENTER_CS 0x174
	//#define MSR_IA32_SYSENTER_ESP 0x175
	//#define MSR_IA32_SYSENTER_EIP 0x176
	int x,y,z;
	__asm__("xor %%edx,%%edx ; \
			 movl %0,%%eax; \
			 movl $0x174,%%ecx; \
			 wrmsr; \
			 xor %%edx,%%edx ; \
			 movl %1,%%eax; \
			 movl $0x176,%%ecx; \
			 wrmsr; \
			 xor %%edx,%%edx ; \
			 movl %2,%%eax; \
			 movl $0x175,%%ecx; \
			 wrmsr; "
			:
			:"r"(cs0_selector),"r"(ip),"r"(esp)
			:"%eax","%edx","%ecx");
	__asm__("movl $0x174,%%ecx; \
			rdmsr; \
			movl %%eax, %0;\
			movl $0x175,%%ecx; \
			rdmsr; \
			movl %%eax, %1;\
			movl $0x176,%%ecx; \
			rdmsr; \
			movl %%eax, %2"
			:"=r"(x),"=r"(y),"=r"(z)
			:
			:);
	printf("+++++++++++++++++++++++++++++++++++++++++\n");
	printf("%x,%x,%x\n",cs0_selector,ip,esp);
	printf("%x,%x,%x\n",x,z,y);
	printf("+++++++++++++++++++++++++++++++++++++++++\n");
	//while(1);
}
