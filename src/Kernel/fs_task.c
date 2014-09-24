#include <system_calls.h>
#include <calls.h>

#include "kerneltasks.h"

/**
 * Este archivo contiene el codigo para un servidor de sistemas de archivos. 
 * Se encarga de mantener dos listas de dispositovos, estos son dispositivos de
 * caracteres y de bloques.
 * Los dispositivos deben registrarse para existir.
 * Todo hilo del sistema, incluidos los de usuarios solicitar servicios asociados
 * con los archivos.
*/

#define MAX_DEVICE_COUNT 40

typedef struct
{
	char freeEntry;	
	TID idDriver;
	unsigned permisions; // DEVICE_WRITABLE/DEVICE_READABLE
	char name[10];	// un nombre
} CharacterDevice;

// Vamos a poner fijo el manejo de memoria
static CharacterDevice _characterDevices[MAX_DEVICE_COUNT];


/**
 * \brief Escribe a un dispositivo dado
 * \param to
 * \return OS_OK u OS_FAIL
 */
static int do_Write(TID to)
{
	
	// El utcb lo tiene todo, solo debo borrar informacion de mas
	unsigned size = StoreRegistre(1);
	unsigned length = StoreRegistre(4);
	// lo que debo borrar es el descriptor que se encuentra en VR3
	// debe quedar
	// VR0 - de quien
	// VR1 - Size 
	// VR2 - operacion WRITE_CHARACTERS/READ_CHARACTERS
	// VR3 - cantidad de caracteres
	// VRSimple16 hasta VRSimple(16 + VR3) - caracteres involucrados en la operacion
	LoadRegistre(0,to);
	LoadRegistre(1,size - 1);
	LoadRegistre(2,WRITE_CHARACTERS);
	LoadRegistre(3,length);
	int i = 0;
	while (i < length)
	{
		unsigned v = StoreByteRegistre(20 + i);
		LoadByteRegistre(16 + i,v);
		i++;
	}
	// TODO : Esto debe ser SEND solamente, pues muchos hilos quieren trabajar a la vez
	// mediante este tipo
	ipc(THREAD_SND_RCV,to); // Esto es un sacrilegio
	return StoreRegistre(2);
}

static void do_read(TID to)
{
	TID from = StoreRegistre(0);
	unsigned size = StoreRegistre(1);
	unsigned count = StoreRegistre(4);
	if (size != 5 || count >= 240 )
	{
		// Todo: error
	}
	LoadRegistre(0,to);
	LoadRegistre(1,4);
	LoadRegistre(3,count);
	// TODO : Esto debe ser SEND solamente, pues muchos hilos quieren trabajar a la vez
	// mediante este tipo
	ipc(THREAD_SND_RCV,to); // Esto es un sacrilegio
	// esto devuelve
	// VR0 - from = to
	// VR1 - Size
	// VR2 - Cantidad leida, negativo en caso de error
	// A partir de VRSimple12 lo leido
	LoadRegistre(0,from);
	ipc(THREAD_SEND,from);
}


void fs_thread(void * utcb)
{
	char err,i;
	// marco todas las entradas como libres
	for (i = 0 ; i < MAX_DEVICE_COUNT ; i++)
		_characterDevices[i].freeEntry = 1;
	
	while (1)
	{
		err = 0;
		// espero por un mensaje
		ipc(THREAD_RECEIVE ,ANY_SOURCE);
		
		// veo que tipo de mensaje es
		unsigned from = StoreRegistre(0);
		unsigned size = StoreRegistre(1);
		/*if (size != 5 && size != 6)
		{
			// TODO: hay un error, lo reporto
			continue;
		}
		 */
		unsigned type = StoreRegistre(2);
		switch (type)
		{
			case DEVICE_INITIALITED:
				// lo pongo en la lista
				// los registro virtuales estan de la siguiente forma
				// VR0 - Sender
				// VR1 - Size
				// VR2 - DEVICE_INITIALITED
				// VR3 - Type : Character / Block
				// if type is Character
				// 			VR4 - Permisions
				// if type is Block
				//			VR4 - BlockSize
				//			VR5 - BlockCount
				if (size == 5 && StoreRegistre(3) == CHARACTER_DEVICE)
				{
					i = 0;
					while (i < MAX_DEVICE_COUNT && !_characterDevices[i].freeEntry)
						i++;
					if (i < MAX_DEVICE_COUNT)
					{
						_characterDevices[i].freeEntry = 0;
						_characterDevices[i].idDriver = from;
						type = StoreRegistre(4);
						_characterDevices[i].permisions = (DEVICE_WRITABLE & type) | (DEVICE_READABLE & type);
						
						success(from); // todo ok
					} else err = 1;
				} 
				else if (size == 6 && StoreRegistre(3) == BLOCK_DEVICE)
				{
					// TODO : Programar esto
				}
				else err = 1;
			break;
			case DEVICE_UNINITIALITED:
				// lo quito de la lista
			break;
			case WRITE_CHARACTERS:
				// Todo: buscar que archivo tiene ese descriptor para el hilo que me hace la peticion
				// Por ahora escribo directo para la consola
				// VR0 - para 5
				// VR1 - Size 
				// VR2 - operacion WRITE_CHARACTERS/READ_CHARACTERS
				// VR3 - File Descriptor
				// VR4 - cantidad de caracteres
				// VRSimple20 hasta VRSimple(20 + VR3) - caracteres involucrados en la operacion
				if (do_Write(_characterDevices[0].idDriver) == OS_OK)
					success(from);
				else
					error(from,0);
			break;
			case READ_CHARACTERS:
				// VR0 - para 5
				// VR1 - Size, 4
				// VR2 - operacion READ_CHARACTERS
				// VR3 - File Descriptor
				// VR4 - cantidad de elementos a leer
				do_read(_characterDevices[0].idDriver);
			break;
			default:
				
				err = 1;
			break;
		}
		if (err)
		{
			// TODO: hay un error, lo reporto
			error(from,0);
		}
	}
}


// Pila del este hilo
static char pila[1024];
// Area utcb de este hilo
static char utcb[UTCB_AREA_SIZE];

/**
 * \brief Crea un hilo para acceder a los dispositivos de bloque AT
 */
void Init_FS_Control_Thread()
{
	ThreadControl(&_fs_thread,&pila[1024],&utcb[0]);
}