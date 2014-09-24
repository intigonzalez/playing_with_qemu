#ifndef __THREAD__
#define __THREAD__

#include "includes/global.h"
#include "kernel.h"
#include "queue.h"

#include <system_calls.h>

// ESTADOS EN LOS QUE PUEDE ESTAR UN PROCESO
#define THREAD_READY        0
#define THREAD_RECEIVING    1
#define THREAD_SENDING      2
#define THREAD_SENDING_RECEIVING      3

#define FREE_THREAD_ENTRY   0
#define FIRST_THREAD_ID     3


/**
 * Esta estructura representa el estado de un hilo del sistema
 */
class Thread
{
public:
	// registros de proposito general
	unsigned eax; // offset 0
	unsigned ebx; // offset 4
	unsigned ecx; // offset 8
	unsigned edx; // offset 12

	// registros para el trabajo con la pila
	unsigned esp; // offset 16
	unsigned ebp; // offset 20

	// registros indices
	unsigned esi; // offset 24
	unsigned edi; // offset 28

	// apuntador al codigo
	unsigned eip; // offset 32

	// registros selectores de segmento
	unsigned cs;  // offset 36
	unsigned ds;  // offset 40
	unsigned ss;  // offset 44
	unsigned es;  // offset 48
	unsigned fs;  // offset 52
	unsigned gs;  // offset 56

	// banderas
	unsigned flags;   // offset 60

	// directorio de paginas
	unsigned page_directory;  // offset 64


	char* kernelStack;			// offset 68
	// direccion del area utcb, es responsabilidad de la aplicacion mantenerla
	// integra, de lo contrario, ese hilo no funciona,
	// el kernel solo lo mata si hace algo malo
	void* utcbArea;				// offset 72


	// A partir de aqui aparece lo que no es obligatorio
	unsigned status;

	// identificador del hilo
	TID tid;

	// es diferente de FREE_THREAD_ENTRY cuando el hilo esta REcibiendo
	TID waitingFor;

	// lista de los quedesean enviarme algo
	Queue* wanToSendMe;

	// indica si el hilo ha sido interrumpido por el hardware
	bool interrupted;


	Thread();
};

typedef Thread * pThread;


class ThreadControlSystem
{
private:
	int numero;
	/**
	* La lista de hilos
	*/
	Queue* threads;
	
	pThread idle_thread;
	
	unsigned threadCount;
	unsigned id_counter;

	pThread Locate(TID tid);

	bool EstaEnviando(pThread src, pThread dst);

	void CopyToAddressSpace_OLD(unsigned value,void * address,pThread thread);
	void CopyMessage_OLD(pThread src, pThread dst);
	void CopyMessage(pThread src, pThread dst);
	
	void dispatch(pThread thread);

public:
	void InitThreadTable();
	void CreateThread(unsigned eip, unsigned esp , unsigned utcb, unsigned page_directory_address);
	void shedule();
	bool SendHardwareIPC(TID tid, unsigned char number);
	bool ipc(unsigned action, TID tid, pThread thread);

};

extern pThread activeThread;
extern ThreadControlSystem threadControl;




#endif
