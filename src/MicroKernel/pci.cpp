#include "includes/global.h"
#include "kernel.h"
#include "video.h"
#include "BasicMachine.h"
#include "pci.h"

#define PCI_ADDR_REG	0xcf8
#define PCI_DATA_REG	0xcfc

static int pci_read (int addr)
{
  BasicMachine::out_long (PCI_ADDR_REG,addr );
  return BasicMachine::in_long(PCI_DATA_REG);
}

static int pci_read_word (int addr)
{
  BasicMachine::out_long (PCI_ADDR_REG, addr & ~3 );
  return BasicMachine::in_word (PCI_DATA_REG + (addr & 3));
}

static int pci_read_byte (int addr)
{
  BasicMachine::out_long (PCI_ADDR_REG, addr & ~3);
  return BasicMachine::in_byte (PCI_DATA_REG + (addr & 3));
}

static void pci_write (int addr, int data)
{
  BasicMachine::out_long (PCI_ADDR_REG, addr);
  BasicMachine::out_long (PCI_DATA_REG, data);
}

static void pci_write_word (int addr, int data)
{
  BasicMachine::out_long (PCI_ADDR_REG, addr & ~3);
  BasicMachine::out_word (PCI_DATA_REG + (addr & 3), data);
}

static void pci_write_byte (int addr, int data)
{
  BasicMachine::out_long (PCI_ADDR_REG, addr & ~3);
  BasicMachine::out_byte (PCI_DATA_REG + (addr & 3), data);
}

static int pci_make_address (int bus, int device, int function, int reg)
{
  return (1 << 31) | (bus << 16) | (device << 11) | (function << 8) | (reg << 2);
}

struct pci_classname
{
  int clase;
  int subclass;
  const char *desc;
};

static const struct pci_classname grub_pci_classes[] =
  {
    { 0, 0, "" },
    { 1, 0, "SCSI Controller" },
    { 1, 1, "IDE Controller" },
    { 1, 2, "Floppy Controller" },
    { 1, 3, "IPI Controller" },
    { 1, 4, "RAID Controller" },
    { 1, 6, "SATA Controller" },
    { 1, 0x80, "Mass storage Controller" },
    { 2, 0, "Ethernet Controller" },
    { 2, 1, "Token Ring Controller" },
    { 2, 2, "FDDI Controller" },
    { 2, 3, "ATM Controller" },
    { 2, 4, "ISDN Controller" },
    { 2, 0x80, "Network controller" },
    { 3, 0, "VGA Controller" },
    { 3, 1, "XGA Controller" },
    { 3, 2, "3D Controller" },
    { 3, 0x80, "Display Controller" },
    { 4, 0, "Multimedia Video Device" },
    { 4, 1, "Multimedia Audio Device" },
    { 4, 2, "Multimedia Telephony Device" },
    { 4, 0x80, "Multimedia device" },
    { 5, 0, "RAM Controller" },
    { 5, 1, "Flash Memory Controller" },
    { 5, 0x80, "Memory Controller" },
    { 6, 0, "Host Bridge" },
    { 6, 1, "ISA Bridge" },
    { 6, 2, "EISA Bride" },
    { 6, 3, "MCA Bridge" },
    { 6, 4, "PCI-PCI Bridge" },
    { 6, 5, "PCMCIA Bridge" },
    { 6, 6, "NuBus Bridge" },
    { 6, 7, "CardBus Bridge" },
    { 6, 8, "Raceway Bridge" },
    { 6, 0x80, "Unknown Bridge" },
    { 7, 0x80, "Communication controller" },
    { 8, 0x80, "System hardware" },
    { 9, 0, "Keyboard Controller" },
    { 9, 1, "Digitizer" },
    { 9, 2, "Mouse Controller" },
    { 9, 3, "Scanner Controller" },
    { 9, 4, "Gameport Controller" },
    { 9, 0x80, "Unknown Input Device" },
    { 10, 0, "Generic Docking Station" },
    { 10, 0x80, "Unknown Docking Station" },
    { 11, 0, "80386 Processor" },
    { 11, 1, "80486 Processor" },
    { 11, 2, "Pentium Processor" },
    { 11, 0x10, "Alpha Processor" },
    { 11, 0x20, "PowerPC Processor" },
    { 11, 0x30, "MIPS Processor" },
    { 11, 0x40, "Co-Processor" },
    { 11, 0x80, "Unknown Processor" },
    { 12, 0x80, "Serial Bus Controller" },
    { 13, 0x80, "Wireless Controller" },
    { 14, 0, "I2O" },
    { 15, 0, "IrDA Controller" },
    { 15, 1, "Consumer IR" },
    { 15, 0x10, "RF-Controller" },
    { 15, 0x80, "Satellite Communication Controller" },
    { 16, 0, "Network Decryption" },
    { 16, 1, "Entertainment Decryption" },
    { 16, 0x80, "Unknown Decryption Controller" },
    { 17, 0, "Digital IO Module" },
    { 17, 0x80, "Unknown Data Input System" },
    { 0, 0, 0 },
  };

static const char *pci_get_class (int clase, int subclass)
{
  const struct pci_classname *curr = grub_pci_classes;

  while (curr->desc)
    {
      if (curr->clase == clase && curr->subclass == subclass)
					return curr->desc;
      curr++;
    }

  return 0;
}

static int hook (int bus, int dev, int func, int pciid)
{
	int clase;
	const char *sclass;
	int addr;

	//grub_printf ("%02x:%02x.%x %04x:%04x", bus, dev, func, pciid & 0xFFFF,pciid >> 16);
	addr = pci_make_address (bus, dev, func, 2);
	clase = pci_read (addr);

	/* Lookup the class name, if there isn't a specific one,
	retry with 0x80 to get the generic class name.  */
	sclass = pci_get_class (clase >> 24, (clase >> 16) & 0xFF);
	if (! sclass)
		sclass = pci_get_class (clase >> 24, 0x80);
	if (! sclass)
		return 0;//sclass = "";

	printf (" [%x] %s\n", (clase >> 16) & 0xffff, sclass);

	//grub_uint8_t pi = (class >> 8) & 0xff;
	//if (pi)
	//	grub_printf (" [PI %02x]", pi);

	//printf ("Device(%x,%x,%x,%x)\n",bus,dev,func,pciid);

	return 0;
}

void enumerate_pci_devices()
{
	int bus;
  int dev;
  int func;
  int addr;
  int id;
  int hdr;

	for (bus = 0; bus < 256; bus+=2)
	{
		for (dev = 0; dev < 32; dev++)
		{
			for (func = 0; func < 8; func++)
			{
				addr = pci_make_address (bus, dev, func, 0);
				id = pci_read (addr);

				/* Check if there is a device present.  */
				if (id >> 16 == 0xFFFF) continue;

				if (hook (bus, dev, func, id)) return;

				/* Probe only func = 0 if the device if not multifunction */
				if (func == 0)
				{
					addr = pci_make_address (bus, dev, func, 3);
					hdr = pci_read (addr);
					if (!(hdr & 0x800000)) break;
				}
			}
		}
	}
}