#include <system_calls.h>
#include <calls.h>

#include "kerneltasks.h"

#define KEYBD		0x60	/* I/O port for keyboard data */
#define KEYSTATUS	0x64	/* Estado del teclado */
#define KEYCONTROL	0x64	/* para escribir en el */

#define KB_ACK		0xFA	/* keyboard ack response */
#define KB_BUSY		0x02	/* status bit set when KEYBD port ready */
#define LED_CODE	0xED	/* command to keyboard to set LEDs */


//#define _COSALOCA_
#ifdef _COSALOCA_

	#define VIDEO_MEMORY	((unsigned char *)((1020-3)*4*1024*1024 + 0xb8000))
	
#else

	#define VIDEO_MEMORY	((unsigned char *)(_bootInfo.videoMemory))
	
#endif

// posicion de los cursores
static int cursorRow = 0;
static unsigned short offset = 0;

#define SIZE_BUFFER_TECLADO	200

static unsigned char tecladoEsp[][3] = {
	{0 , 0, 0},				// 0
	{0, 0, 0},				// 1
	{'1', '!', '|'},	// 2
	{'2', '"', '@'},	// 3
	{'3', '·', '#'},	// 4
	{'4', '$', '~'},	// 5
	{'5', '%', '½'},	// 6
	{'6', '&', '¬'},	// 7
	{'7', '/', 0},		// 8
	{'8', '(', 0},		// 9
	{'9', ')', 0},		// 10
	{'0', '=', 0},		// 11
	{'\'', '?', 0},		// 12
	{'¡', '¿', 0},		// 13
	{0, 0, 0},				// 14	BACKSPACE
	{'\t', 0, 0},			// 15 TAB
	{'q', 'Q', 0},		// 16
	{'w', 'W', 0},		// 17
	{'e', 'E', 0},		// 18
	{'r', 'R', 0},		// 19
	{'t', 'T', 0},		// 20
	{'y', 'Y', 0},		// 21
	{'u', 'U', 0},		// 22
	{'i', 'I', 0},		// 23
	{'o', 'O', 0},		// 24
	{'p', 'P', 0},		// 25
	{'`', '^', '['},	// 26
	{'+', '*', ']'},	// 27
	{'\n','\n','\n'}, // 28	ENTER
	{0, 0, 0},				// 29 CONTORL
	{'a', 'A', 0},		// 30
	{'s', 'S', 0},		// 31
	{'d', 'D', 0},		// 32
	{'f', 'F', 0},		// 33
	{'g', 'G', 0},		// 34
	{'h', 'H', 0},		// 35
	{'j', 'J', 0},		// 36
	{'k', 'K', 0},		// 37
	{'l', 'L', 0},		// 38
	{164, 'Ñ', 0},		// 39
	{'Ž', 'š', '{'},	// 40
	{'º', 'ª', '\\'},	// 41
	{0 , 0 ,0 },			// 42 SHIFT izquierdo
	{0 , 0 ,0 },			// 43 no se quien es el 43
	{'z', 'Z', 0},		// 44
	{'x', 'X', 0},		// 45
	{'c', 'C', 0},		// 46
	{'v', 'V', 0},		// 47
	{'b', 'B', 0},		// 48
	{'n', 'N', 0},		// 49
	{'m', 'M', 0},		// 50
	{',', ';', 0},		// 51
	{'.', ':', 0},		// 52
	{'-', '_', 0},		// 53
	{0, 0, 0},			// 54 SHIFT derecho
	{'*', '*', '*'},	// 55
	{0, 0, 0},			// 56 ALT
	{' ',' ', 0},		// 57 SPACE	
	{0, 0, 0},			// 58 CAP
	{0, 0, 0},			// 59 F1
	{0, 0, 0},			// 60 F2
	{0, 0, 0},			// 61 F3
	{0, 0, 0},			// 62 F4
	{0, 0, 0},			// 63 F5
	{0, 0, 0},			// 64 F6
	{0, 0, 0},			// 65 F7
	{0, 0, 0},			// 66 F8
	{0, 0, 0},			// 67 F9
	{0, 0, 0},			// 68 F10
	{0, 0, 0},			// 69 BLOQ NUM
	{0, 0, 0},			// 70 BLOQ SCROLL
	{'7',0,0},			// 71
	{'8',0,0},			// 72
	{'9',0,0},			// 73
	{'-',0,0},			// 74
	{'4',0,0},			// 75
	{'5',0,0},			// 76
	{'6',0,0},			// 77
	{'+',0,0},			// 78
	{'1',0,0},			// 79
	{'2',0,0},			// 80
	{'3',0,0},			// 81
	{'0',0,0},			// 82
	{'.',0,0}			// 83
};



static struct
{
	unsigned char capsOn;
	unsigned char numOn;
	unsigned char scrollOn;
	unsigned char shiftPressed;
	unsigned char altGRPressed;
	unsigned char unaEspecial;
	unsigned char keyboardIniciado;
	int front;
	int end;
	int count;
	char cola[SIZE_BUFFER_TECLADO];
} keyboard;

typedef struct
{
	TID from; // quien pide los caracteres (debe ser siempre FS)
	unsigned count ; // cuantos pide
} Request; // representa una solicitud de caracteres

static int pendingRequest = 0; // indica si existe una solicitud pendiente
static int eofFile = 0; // indica si se llego al final der archivo ,un enter por ejemplo
static Request _request[1]; // solicitud de caracteres realizada

/**
 * Esta funcion realiza un scroll de la pantalla para dejar una linea libre al final
 */
static void scroll()
{ 
	unsigned long i,count;
	unsigned *vidmem = (unsigned*)VIDEO_MEMORY;
	count = 960 /* 25 * 80 */;
	i = 0;
	while (i < count){
		vidmem[i] = vidmem[i + 40];
		i ++;
	}

	// la ultima fila se limpia
	count += 40;
	for ( ; i < count ; i++)
		vidmem[i] = 0x0A200A20;
		
	offset = 1920; // 24*80
	
	cursorRow = 24;
}


/**
 * \brief Escribe a la consola lo que me piden y devuelve mensaje
 */
static void Write()
{
	unsigned char *vidmem = VIDEO_MEMORY;
	int count = StoreRegistre(3);
	int from = StoreRegistre(0);
	
	
	
	if (count > 240)
	{
		// error
		//count = 240;
		error(from, 0);
		return;
	}
	
	
	for (int i = 0 ;i < count ; i++)
	{
		char c = StoreByteRegistre(i + 16);

		// Start at writing at cursor position
		
		switch (c){
			case '\n':
				if (cursorRow < 24)
					cursorRow ++;
				else
					scroll();	// corro todo
				offset = cursorRow*80;
				continue;
				break;
			case '\t':
					// mostrar 4 espacios en blanco
					vidmem[offset << 1] = ' ';
					offset ++;
				break;
			default:
				vidmem[offset << 1] = c;
				offset ++;
		}
		
		cursorRow = offset / 80;

		if (offset >= 2000 ){ 
			scroll();
		}
	}
	//offset = cursorRow * 80 + cursorCol;
	out_byte(0x3D4, 15);
	out_byte(0x3D5, offset);
	out_byte(0x3D4, 14);
	out_byte(0x3D5, offset >> 8);
	
	
	success(from);
	
	
}

static void TryAnswerRequest()
{
	if (pendingRequest && (_request[0].count <= keyboard.count || eofFile))
	{
		unsigned count = (_request[0].count < keyboard.count) ? _request[0].count : keyboard.count;
		int size = 3 + (count) + ((count % 4 != 0)? 1:0);
		LoadRegistre(0,_request[0].from);
		LoadRegistre(1,size);
		LoadRegistre(2,count);
		for (int i = 0 ; i < count ; i++)
		{
			LoadByteRegistre(i + 12, keyboard.cola[keyboard.front]);
			keyboard.front = (keyboard.front + 1) % SIZE_BUFFER_TECLADO;
			keyboard.count --;
		}
		
		pendingRequest = 0;
		eofFile = 0;
		ipc(THREAD_SEND, _request[0].from);
	}
	
	
}

/**
 * \brief Lee de la consola lo que me piden y devuelve mensaje 
 */
static void Read()
{
	// VR0 - para 5
	// VR1 - Size, 4
	// VR2 - operacion READ_CHARACTERS
	// VR3 - cantidad de elementos a leer
	int count = StoreRegistre(3);
	int from = StoreRegistre(0);
	
	pendingRequest = 1; // esto no es valido si varios pueden pedir a la vez
	_request[0].from = from;
	_request[0].count = count;
	
	TryAnswerRequest();
}

static void printChar(char c)
{
	unsigned char *vidmem = VIDEO_MEMORY;
	switch (c){
			case '\n':
				if (cursorRow < 24)
					cursorRow ++;
				else
					scroll();	// corro todo
				offset = cursorRow*80;
				break;
			case '\t':
					// mostrar 4 espacios en blanco
					vidmem[offset << 1] = ' ';
					offset ++;
				break;
			default:
				vidmem[offset << 1] = c;
				offset ++;
	}
		
	cursorRow = offset / 80;

	if (offset >= 2000 )
		scroll();
		
	//offset = cursorRow * 80 + cursorCol;
	
	out_byte(0x3D4, 15);
	out_byte(0x3D5, offset);
	out_byte(0x3D4, 14);
	out_byte(0x3D5, offset >> 8);
	
}

/**
 * \brief Lee del controlador PS/2 Keyboard un key code y lo traduce
 */
static void readKeyboard()
{
	int code;
    
	code = in_byte(KEYBD);	/* get the scan code for the key struck */
	
	switch (code){
			case 42:	// shift
			case 54:
				keyboard.shiftPressed = 1;
			break;
			case 42 + 128:	// liberar shift
			case 54 + 128:
				keyboard.shiftPressed = 0;
			break;
			case 0xe0:	// viene una especial detras
				keyboard.unaEspecial = 1;
			break;
			case 56:	// alt
				if (keyboard.unaEspecial)
					keyboard.altGRPressed = 1;
			break;
			case 56 + 128: // liberar alt
				if (keyboard.unaEspecial)
					keyboard.altGRPressed = 0;
			break;
			case 58 + 128:
				keyboard.capsOn = !keyboard.capsOn; 
			break;
			case 29:
			break;
			default:
				if (code <= 128){
					int level = 0;

					// primero shift
					level += (keyboard.shiftPressed)? 1 : 0;
					// luego caps
					if ((level && keyboard.capsOn) || (level == 0 && !keyboard.capsOn))
						level = 0;
					else
						level = 1;

					// luego alt gr
					level = (keyboard.altGRPressed)? 2 : level;

					if (level < 3){	// solo apretado uno de los dos o shift o altgr
						char letra = tecladoEsp[code][level];
						eofFile = (letra == '\n');
						if ((keyboard.end + 1) % SIZE_BUFFER_TECLADO != keyboard.front)
						{
							// compruebo si cabe algo en la cola
							keyboard.cola[keyboard.end] = letra;
							keyboard.end = (keyboard.end + 1) % SIZE_BUFFER_TECLADO;
							keyboard.count ++;
							printChar(letra);
						
						}
					}
				}
	}

	if (code!=0xe0)
		keyboard.unaEspecial = 0;

	//convertir(code , buff);
	//print(buff);
	//print("-");
}


static void kb_wait()
{
	while (in_byte(KEYSTATUS) & KB_BUSY)
	;			/* wait until not busy */
}

static void kb_ack()
{
	while (in_byte(KEYBD) != KB_ACK)
	;			/* wait for ack */
}

#define PIC_MASTER_PORT2 0x21

/**
 * \brief Realiza la inicializacion del teclado
 */
static void InitializeKeyboard()
{
	keyboard.capsOn = 0;
	keyboard.numOn = 0;
	keyboard.scrollOn = 0;

	keyboard.altGRPressed = 0;
	keyboard.shiftPressed = 0;

	keyboard.unaEspecial = 0;

	keyboard.keyboardIniciado = 1;
	
	keyboard.front = 0;
	keyboard.end = 0;
	keyboard.count = 0;

	kb_wait();
	
	out_byte(KEYBD,0xed);
	
	kb_ack();
	kb_wait();
	out_byte(KEYBD,0x00);
	kb_ack();
	
	
	InterruptControl(0x30  + 0x01/*KEYBOARD_IRQ + IRQ0_VECTOR*/);
	
	
	unsigned char valor = 0;
	
	// Activo la interrupcion en el PIC (Todo: Odio este codigo, pero: donde lo pongo?)
	__asm__("cli;"::);
	int irq = 1;
	valor = in_byte(PIC_MASTER_PORT2);
	valor &= ~(1 << irq);
	out_byte(PIC_MASTER_PORT2, valor);
	
	__asm__("sti;"::);
	
	
}

static void InitializeVGA()
{
	unsigned *vidmem = (unsigned*)VIDEO_MEMORY;

	// Clear visible video memory
	for (long loop=0; loop<1000; loop++)
		vidmem[loop] = 0x0A200A20;;
	
	//offset = cursorRow * 80 + cursorCol;
	out_byte(0x3D4, 15);
	out_byte(0x3D5, offset);
	out_byte(0x3D4, 14);
	out_byte(0x3D5, offset >> 8);
}

void Console_Thread(void * utcb)
{
	
	unsigned char *vidmem = (unsigned char*)VIDEO_MEMORY;
	// me reporto como un dispositivo de caracteres
	NewCharacterDeviceInSystem(DEVICE_WRITABLE | DEVICE_READABLE);
	
	// Clear Screen
	InitializeVGA();
	
	
	// Preparo el teclado
	InitializeKeyboard();
	
	while (1)
	{
		
		// ahora me pongo a esperar peticiones hasta el infinito
		ipc(THREAD_RECEIVE,ANY_SOURCE);
		unsigned from = StoreRegistre(0);
		
		
		
		if (from == HARDWARE_SOURCE)
		{
			readKeyboard();
			out_byte(0x20,0x20);
			// veamos si hay algo que satisfacer
			TryAnswerRequest();
		}
		else
		{
			// VR0 - de quien
			// VR1 - Size 
			// VR2 - operacion WRITE_CHARACTERS/READ_CHARACTERS
			// VR3 - cantidad de caracteres
			// VRSimple16 hasta VRSimple(16 + VR3) - caracteres involucrados en la operacion
			unsigned size = StoreRegistre(1);
			if (size < 4)
			{
				// Todo: error, reportarlo
				error(StoreRegistre(0),0);
				continue;
			}
			unsigned type = StoreRegistre(2);
			if (type == WRITE_CHARACTERS)
			{
				//vidmem[0] = 'W';
				Write();
				
				
			}
			else if (type == READ_CHARACTERS)
			{
				//vidmem[0] = 'R';
				Read();
				
			}
			else
			{
				//vidmem[0] = 'E';
				// Todo: error, reportarlo
				error(StoreRegistre(0),0);
				
			}
		
		}
	}
}

// Pila de este hilo
static char pila[1024];
// Area utcb de este hilo
static char utcb[UTCB_AREA_SIZE];

void Init_Console_Thread()
{
	ThreadControl(_console_thread,&pila[1024],&utcb[0]);
}
