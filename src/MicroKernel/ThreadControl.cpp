#include "kernel.h"
#include "ThreadControl.h"
#include "memory.h"
#include "video.h"
#include "BasicMachine.h"

ThreadControlSystem threadControl;

/**
 * Un apuntador al hilo activo
 */
pThread activeThread;


Thread::Thread()
{
	cs = USER_CODE_SELECTOR;
	ds = USER_DATA_SELECTOR;
	ss = USER_DATA_SELECTOR;
	es = USER_DATA_SELECTOR;
	fs = USER_DATA_SELECTOR;	
	gs = UTCB_SEGMENT;	// este porque me da la gana

	eax = 0;
	ebx = 0;
	ecx = 0;
	edx = 0;
	edi = 0;
	esi = 0;

	ebp = 0;

	eip = 0;
		
	utcbArea = 0;
	kernelStack = 0;

	page_directory = 0;

	status = THREAD_READY; 
		
	interrupted = false;
	waitingFor = FREE_THREAD_ENTRY;
	wanToSendMe = new Queue(5);

	// solo me quedan el eflags y esp
	unsigned normalFlags = 0x202 | 0x3000; // puedo hacer cli y out
	//normalFlags = normalFlags | 0x100; // con esta linea comienzo a debuguear
	flags = normalFlags;   // TODO : Ver este valor
	
	esp = 0;
}

char pila_Idle [28];

/**
 * \brief
 */
void ThreadControlSystem::InitThreadTable()
{
	threads = new Queue(64); 
	
	idle_thread = new Thread();
	idle_thread->tid = FIRST_THREAD_ID - 1;
	idle_thread->cs = IDLE_CODE_SELECTOR;
	idle_thread->ds = IDLE_DATA_SELECTOR;
	idle_thread->ss = IDLE_DATA_SELECTOR;
	idle_thread->es = IDLE_DATA_SELECTOR;
	idle_thread->gs = IDLE_DATA_SELECTOR;
	idle_thread->fs = IDLE_DATA_SELECTOR;
	
	idle_thread->eip = (unsigned int)&idle_thread_entry;
	idle_thread->esp = (unsigned)&pila_Idle[28];
	idle_thread->page_directory = _hwParameters.kernel_page_directory;

	id_counter = FIRST_THREAD_ID;
	threadCount = 0;
}

/**
 * \brief
 * \param eip
 * \param esp
 * \param page_directory_address
 */
void ThreadControlSystem::CreateThread(unsigned eip, unsigned esp, unsigned utcb, unsigned page_directory_address)
{

	pThread newThread = new Thread();

	printf("hilo nuevo %x\n", newThread);

	if (!newThread) {
		// TODO : Agregar tratamiento de errores
		// malo, malo
		printf("=================================================================");
		printf("=================================================================");
		printf("=================================================================");
		printf("=================================================================");
		printf("=================================================================");
	} 
	else 
	{
		newThread->ebx = utcb;
		newThread->eip = eip;
		newThread->esp = esp;
		newThread->utcbArea = new int[64];//(void*)utcb;
		newThread->kernelStack = new char[256];
		newThread->page_directory = page_directory_address;

		newThread->tid = id_counter++;
		newThread->status = THREAD_READY; 


		// ahora debo ponerlo en la lista
		threads->Enqueue(newThread);
		
		threadCount ++;
		// si es el primer hilo ese sera el activo
		if (threadCount == 1)
			dispatch(newThread);
	}
}

/**
 * */
void ThreadControlSystem::shedule()
{
	
	unsigned i = 0;
	bool b = false;
	pThread temp ;
	
	// el hilo activo es el que esta al frente de la cola
	//printf("El hilo activo es %d\n",activeThread->tid);
	//return;
	
	if (activeThread != idle_thread)
	{
		temp = (pThread)threads->Dequeue();
		threads->Enqueue(temp);
	}
		
	temp = (pThread)threads->Front();
	i = 0;
	while (i < threadCount && temp->status != THREAD_READY)
	{
		temp =  (pThread)threads->Dequeue();
		//printf("Hilo JOJJO %d\n",temp->tid);
		threads->Enqueue(temp);
		temp = (pThread)threads->Front();
		i++;
	}
	if (i == threadCount) {
		//printf("Pero si todavia no hay hilo idle %s:%d\n",__FILE__,__LINE__);
		// activamos el hilo idle porque ningun otro esta listo
		
		dispatch(idle_thread);
		
		//while(true);

	} 
	else dispatch(temp);
}

/**
 * \brief Activa un hilo para su ejecucion
 * \param thread
 */
void ThreadControlSystem::dispatch(pThread thread)
{
	activeThread = thread;
	
	if (activeThread != idle_thread){
		BasicMachine::set_utcb_area((unsigned)activeThread->utcbArea);
	}
	//printf("Hilo activo : %d ",activeThread->tid);
}

/**
 * \brief Al producirse una interrupcion de hardware se llama este metodo para informarle
 * al hilo correspondiente
 * \param tid - Id del hilo al que debe informarse
 * \param number - Numero de la interrupcion ocurrida
 * \return 
 */
bool ThreadControlSystem::SendHardwareIPC(TID tid, unsigned char number)
{
	//busco el hilo con ese numero, lento!!!!!!!!!
	Iterator * ite = threads->iterator();
	bool b = false;
	pThread hilo;
	while (ite->HasMore() && !b)
	{
		hilo = (pThread)ite->Current();
		b = hilo->tid == tid;
		if (!b)
			ite->Next();
	}
	// elimino el iterador
	delete ite;
	if (b) {
		
		bool handle = false;
		if (hilo->status == THREAD_RECEIVING)
		{
			switch (hilo->waitingFor)
			{
				case HARDWARE_SOURCE:
				case ANY_SOURCE:
					hilo->status = THREAD_READY;
					hilo->waitingFor = FREE_THREAD_ENTRY;
					hilo->interrupted = false;
					// copio la informacion en el area utcb
					int* utcb = (int*)hilo->utcbArea;
					utcb[0] = HARDWARE_SOURCE;
					utcb[1] = 3;
					utcb[2] = number;
					handle = true;
				break;
			}
		}
		if (!handle)
			hilo->interrupted = true;
		else if (activeThread == idle_thread)
			shedule();
		
		return true;
	}
	return false;
}

/**
 * \brief Realiza las acciones correspondientes a la llamada ipc
 * \param action - SEND, RECEIVE o SEND_RECEIVE
 * \param tid	- A quien enviar o de quien recibir
 * \param thread - Que hilo desea realizar la accion
 * \return True si se logra mandar la ayuda
 */
bool ThreadControlSystem::ipc(unsigned action, TID tid, pThread thread)
{
	
	pThread who;
	bool done = false;
	
	//printf("Action(%d), Actor(%d) with(%d) , ",action,thread->tid,tid);
	
	switch (action)
	{
		case THREAD_SEND: // quiere enviar // ESTA PARTE YA
			who = Locate(tid);
			// no existe el hilo
			if (who== NULL) return false; 
			
			if (who->status == THREAD_RECEIVING 
							&& (who->waitingFor == thread->tid || who->waitingFor == ANY_SOURCE))
			{
				// si el hilo esta esperando y ademas por mi, envio el mensaje
				CopyMessage(thread,who);
				who->status = THREAD_READY;
				who->waitingFor = FREE_THREAD_ENTRY;
			}
			else
			{
				// debo ponerme en cola para enviar si el hilo no esta esperando por mi
				thread->status = THREAD_SENDING;
				who->wanToSendMe->Enqueue(thread);
				shedule();
			}
		break;
		case THREAD_RECEIVE: // quiere recibir
			switch (tid)
			{
				case ANY_SOURCE: // ESTA PARTE YA
					if (thread->interrupted)
					{
						// si esta esperando de cualquiera y hay una interrupcion ocurrida
						// la transmito
						thread->interrupted = false;
						// copio la informacion en el area utcb
						// la fuente
						int* utcb = (int*)thread->utcbArea;
						utcb[0] = HARDWARE_SOURCE;
						utcb[1] = 3;
						utcb[2] = 1; // TODO: Arreglar este numero
						//CopyToAddressSpace_OLD(HARDWARE_SOURCE,thread->utcbArea,thread);
						//CopyToAddressSpace_OLD(3,thread->utcbArea + 4,thread); // la longitud del mensaje
						// Todo: Arreglar este numero
						//CopyToAddressSpace_OLD(1,thread->utcbArea + 8, thread);  // la interrupcion que ocurrio
						done = true;
					} 
					else if (!thread->wanToSendMe->Empty()) // si alguien quiere enviarme
					{
						who = (pThread)thread->wanToSendMe->Dequeue();
						// si yo recibo de cualquiera y hay cualquiera, entonces ipc
						CopyMessage(who,thread);
						who->status = (who->status == THREAD_SENDING)? THREAD_READY: THREAD_RECEIVING;
						done = true;
					}
				break;
				case HARDWARE_SOURCE: // ESTA PARTE YA
					// si espero del hardware y ocurrio una interrupcion
					// la transmito
					if (thread->interrupted)
					{
						// si esta esperando de cualquiera y hay una interrupcion ocurrida
						// la transmito
						thread->waitingFor = FREE_THREAD_ENTRY;
						thread->interrupted = false;
						// copio la informacion en el area utcb
						int* utcb = (int*)thread->utcbArea;
						utcb[0] = HARDWARE_SOURCE;
						utcb[1] = 3;
						utcb[2] = 1; // Todo: Arreglar este numero
						//CopyToAddressSpace_OLD(HARDWARE_SOURCE,thread->utcbArea,thread);
						//CopyToAddressSpace_OLD(3,thread->utcbArea + 4,thread); // la longitud del mensaje
						//CopyToAddressSpace_OLD(1,thread->utcbArea + 8, thread);  // la interrupcion que ocurrio
						done = true;
					}
				break;
				default:
					who = Locate(tid);
					if (EstaEnviando(who,thread))
					{
						// lo saco del lista de los que me envian y copio
						CopyMessage(who,thread);
						who->status = (who->status == THREAD_SENDING)? THREAD_READY: THREAD_RECEIVING;
						who = (pThread)thread->wanToSendMe->Dequeue();
						while (who->tid != tid)
						{
							thread->wanToSendMe->Enqueue(who);
							who = (pThread)thread->wanToSendMe->Dequeue();
						}
						// aqui lo encontre y lo elimine ya de la cola
						done = true;
					}
				break;
			}
			if (!done)
			{
				// no se logro recibir, tengo que poner el hilo a esperar
				//printf("JOJOJOJOJOJOOJOJOJOJJOJOO\n");
				// no me envia el que yo quiero, me pongo a esperar
				thread->status = THREAD_RECEIVING;
				thread->waitingFor = tid;
				shedule();
			}
			
		break;
		case THREAD_SND_RCV: // quiere enviar y recibir
			who = Locate(tid);
			// no existe el hilo
			if (who== NULL) return false; 
			
			if (who->status == THREAD_RECEIVING 
							&& (who->waitingFor == thread->tid || who->waitingFor == ANY_SOURCE))
			{
				// si el hilo esta esperando y ademas por mi, envio el mensaje
				CopyMessage(thread,who);
				who->status = THREAD_READY;
				who->waitingFor = FREE_THREAD_ENTRY;
				// ya envie, debo ponerme a esperar la respuesta
				thread->status = THREAD_RECEIVING;
			}
			else
			{
				// debo ponerme en cola para enviar si el hilo no esta esperando por mi
				thread->status = THREAD_SENDING_RECEIVING;
				who->wanToSendMe->Enqueue(thread);
				
			}
			thread->waitingFor = tid;
			shedule();
		break;
		default:
			// TODO: error
		break;
	}
}

/**
 * \brief Busca un hilo dado su id
 * \param tid
 * \return El hilo con ese id o NULL si ningun hilo tiene ese id
 */
pThread ThreadControlSystem::Locate(TID tid)
{
	//busco el hilo con ese numero
	Iterator * ite = threads->iterator();
	bool b = false;
	pThread hilo;
	while (ite->HasMore() && !b)
	{
		hilo = (pThread)ite->Current();
		b = hilo->tid == tid;
		if (!b)
			ite->Next();
	}
	// elimino el iterador
	delete ite;
	return (b)?hilo:NULL;
}

/**
 * \brief Indica si un hilo esta intentando enviarle a otro
 * \param src Hilo que se desea ver si intenta enviar
 * \param dst posible hilo al que se envia
 * \return True si el hilo src esta intentando enviar al hilo dst
 */
bool ThreadControlSystem::EstaEnviando(pThread src, pThread dst)
{
	//busco el hilo con ese numero
	Iterator * ite = dst->wanToSendMe->iterator();
	bool b = false;
	pThread hilo;
	//printf("Buscando si %d envia a %d\n",src->tid,dst->tid);
	while (ite->HasMore() && !b)
	{
		hilo = (pThread)ite->Current();
		b = hilo->tid == src->tid;
		if (!b)
			ite->Next();
	}
	// elimino el iterador
	delete ite;
	//printf("\nEcnontrado\n");
	return b;
	
}

/**
 * \brief Copia un entero sin signo en una direccion dada de un espacio de direcciones
 * \param value Valor a copiar
 * \param address Direccion en el espacio de direcciones objetivo
 * \param thread Hilo del espacio de direcciones objetivo
 */
void ThreadControlSystem::CopyToAddressSpace_OLD(unsigned value, void* address, pThread thread)
{
	//address += 0XC00000;
	// calculamos la direccion en el espacio de direcciones del kernel
	int* add = (int*)thread->page_directory;
	
	// tomamos los 10 bits mas significativos, la entrada de directorios correspondiente
	int i1 = ((unsigned)address) >> 22;
	// tomamos los otros 10 bits mas significativos, la entrada de paginas correspondiente
	int i2 = (((unsigned)address) << 10) >> 22;
	int i3 = ((unsigned)address) & 0xFFF;
	
	
	add = (int*)(add[i1] & 0xFFFFF000);
	
	add = (int*)(add[i2] & 0xFFFFF000);
	
	//printf("%d %d %d %x\n",i1,i2,i3,add);
	
	char * place = (char*)add;
	//place[i3] = 12;
	place = &place[i3];
	add = (int*)place;
	*add = value;
}

/**
 * \brief 
 * \param addr
 * \param pDirectory
 * \return 
 */
unsigned * PhysicalAddress(unsigned addr, unsigned pDirectory)
{
	unsigned * a = (unsigned*)pDirectory;
	int i1 = (addr) >> 22;
	int i2 = ((addr) << 10) >> 22;
	int i3 = (addr) & 0xFFF;
	//printf("%d %d %d\n",i1,i2,i3);
	
	a = (unsigned*)(a[i1] & 0xFFFFF000);
	a = (unsigned*)(a[i2] & 0xFFFFF000);
	
	char * place = (char*)a;
	//printf("Virtual %x Fisica %x\n",addr,(unsigned*)&place[i3]);
	return (unsigned*)&place[i3];
}

/**
 * \brief Copia el mensaje que se dea enviar de src a dst
 * \param src
 * \param dst
 */
void ThreadControlSystem::CopyMessage_OLD(pThread src, pThread dst)
{
	unsigned addressSrc = (unsigned)src->utcbArea ;//+ 0XC00000;
	unsigned addressDts = (unsigned)dst->utcbArea ;//+ 0XC00000;
	
	//printf("%d=>>%d ",src->tid,dst->tid);
	
	addressSrc += 4;
	
	unsigned * addrDst = PhysicalAddress(addressDts,dst->page_directory);
	*addrDst = src->tid; // pongo de donde viene el mensaje
	unsigned * addrSrc = PhysicalAddress(addressSrc,src->page_directory);
	// tomo la lontitud
	int count = *addrSrc;
	
	addressDts+=4;
	// copio la longitud
	addrDst = PhysicalAddress(addressDts,dst->page_directory);
	*addrDst =  count; // por cierto, esta cantidad es en palabras de 32 bits
	// copio el resto del mensaje
	addressSrc += 4;
	addressDts+=4;
	for (int i = 0 ; i < count - 2 ; i++,addressDts+=4,addressSrc += 4)
	{
		addrDst = PhysicalAddress(addressDts,dst->page_directory);
		addrSrc = PhysicalAddress(addressSrc,src->page_directory);
		*addrDst = *addrSrc; 
	}
}
void ThreadControlSystem::CopyMessage(pThread src, pThread dst)
{
	int* addressSrc = (int*)src->utcbArea ;
	int* addressDts = (int*)dst->utcbArea ;
	
	addressDts[0] = src->tid;
	int count = addressSrc[1];
	addressDts[1] = count;
	
	for(int index=0; index<count; index++)
		addressDts[index + 2] = addressSrc[index + 2];
}
