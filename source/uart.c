/*****************************************************************
*       uart.c
*       by Zhiyi Huang, hzy@cs.otago.ac.nz
*       University of Otago
*
********************************************************************/



#include "types.h"
#include "defs.h"
#include "memlayout.h"
#include "traps.h"
#include "arm.h"

#define BASE 0xFE200000

#define GPFSEL0			(BASE + 0x0)//0xFE200000
#define GPFSEL1			(BASE + 0x4)//0xFE200004
#define GPFSEL2			(BASE + 0x8)//0xFE200008
#define GPFSEL3			(BASE + 0xC)//0xFE20000C
#define	GPFSEL4			(BASE + 0x10)//0xFE200010
#define	GPFSEL5			(BASE + 0x14)//0xFE200014
#define GPSET0  		(BASE + 0x1C)//0xFE20001C
#define GPSET1			(BASE + 0x20)//0xFE200020
#define GPCLR0  		(BASE + 0x28)//0xFE200028
#define GPCLR1			(BASE + 0x2C)//0xFE20002C
#define GPPUD       		(BASE + 0x94)//0xFE200094
#define GPPUDCLK0   		(BASE + 0x98)//0xFE200098
#define GPPUDCLK1		(BASE + 0x9c)//0xFE20009C

#define AUX_IRQ			(BASE + 0x15000)//0xFE215000
#define AUX_ENABLES     	(BASE + 0x15004)//0xFE215004
#define AUX_MU_IO_REG   	(BASE + 0x15040)//0xFE215040
#define AUX_MU_IER_REG  	(BASE + 0x15044)//0xFE215044
#define AUX_MU_IIR_REG  	(BASE + 0x15048)//0xFE215048
#define AUX_MU_LCR_REG  	(BASE + 0x1504C)//0xFE21504C
#define AUX_MU_MCR_REG  	(BASE + 0x15050)//0xFE215050
#define AUX_MU_LSR_REG  	(BASE + 0x15054)//0xFE215054
#define AUX_MU_MSR_REG  	(BASE + 0x15058)//0xFE215058
#define AUX_MU_SCRATCH  	(BASE + 0x1505C)//0xFE21505C
#define AUX_MU_CNTL_REG 	(BASE + 0x15060)//0xFE215060
#define AUX_MU_STAT_REG 	(BASE + 0x15064)//0xFE215064
#define AUX_MU_BAUD_REG 	(BASE + 0x15068)//0xFE215068

void
setgpioval(uint func, uint val)
{
	uint sel, ssel, rsel;

	if(func > 53) return;
	sel = func >> 5;
	ssel = GPSET0 + (sel << 2);
	rsel = GPCLR0 + (sel << 2);
	sel = func & 0x1f;
	if(val == 0) outw(rsel, 1<<sel);
	else outw(ssel, 1<<sel);
}


void
setgpiofunc(uint func, uint alt)
{
	uint sel, data, shift;

	if(func > 53) return;
	sel = 0;
	while (func > 10) {
	    func = func - 10;
	    sel++;
	}
	sel = (sel << 2) + GPFSEL0;
	data = inw(sel);
	shift = func + (func << 1);
	data &= ~(7 << shift);
	data |= alt << shift;
	outw(sel, data);
}


void 
uartputc(uint c)
{
	if(c=='\n') {
		while(1) if(inw(AUX_MU_LSR_REG) & 0x20) break;
		outw(AUX_MU_IO_REG, 0x0d); // add CR before LF
	}
	while(1) if(inw(AUX_MU_LSR_REG) & 0x20) break;
	outw(AUX_MU_IO_REG, c);
}

static int
uartgetc(void)
{
	if(inw(AUX_MU_LSR_REG)&0x1) return inw(AUX_MU_IO_REG);
	else return -1;
}

void 
enableirqminiuart(void)
{
        intctrlregs *ip;

        ip = (intctrlregs *) INT_REGS_BASE;
        ip->gpuenable[0] |= (1 << 29);   // enable the miniuart through Aux
}


void
miniuartintr(void)
{
  consoleintr(uartgetc);
}

void 
uartinit(void)
{
	outw(AUX_ENABLES, 1);
	outw(AUX_MU_CNTL_REG, 0);
	outw(AUX_MU_LCR_REG, 0x3);
	outw(AUX_MU_MCR_REG, 0);
	outw(AUX_MU_IER_REG, 0x1);
	outw(AUX_MU_IIR_REG, 0xC7);
	outw(AUX_MU_BAUD_REG, 270); // (250,000,000/(115200*8))-1 = 270

	setgpiofunc(14, 2); // gpio 14, alt 5
	setgpiofunc(15, 2); // gpio 15, alt 5

	outw(GPPUD, 0);
	delay(10);
	outw(GPPUDCLK0, (1 << 14) | (1 << 15) );
	delay(10);
	outw(GPPUDCLK0, 0);

	outw(AUX_MU_CNTL_REG, 3);
	enableirqminiuart();
}
