/*****************************************************************/
/*                                                               */
/*   CASIO fx-9860G SDK Library                                  */
/*                                                               */
/*   File name : [ProjectName].c                                 */
/*                                                               */
/*   Copyright (c) 2006 CASIO COMPUTER CO., LTD.                 */
/*                                                               */
/*****************************************************************/

#include "headers.h"

//****************************************************************************
//  AddIn_main (Sample program main function)
//
//  param   :   isAppli   : 1 = This application is launched by MAIN MENU.
//                        : 0 = This application is launched by a strip in eACT application.
//
//              OptionNum : Strip number (0~3)
//                         (This parameter is only used when isAppli parameter is 0.)
//
//  retval  :   1 = No error / 0 = Error
//
//****************************************************************************

static status = 1;
static jmp_buf main_end;

// Thanks to Sentaro21 for this
static void * HiddenRAM(void){    // Check HiddenRAM
	volatile unsigned int *NorRAM = (volatile unsigned int*)0xA8000000;    // Nomarl RAM TOP (no cache area)
	volatile unsigned int *HidRAM = (volatile unsigned int*)0x88040000;    // Hidden RAM TOP (cache area)
	int a, b;
	int K55 = 0x55555555;
	int KAA = 0xAAAAAAAA;
	char *HidAddress=NULL;

	a = *NorRAM;
	b = *HidRAM;
	*NorRAM = K55;
	*HidRAM = KAA;
	if ( *NorRAM != *HidRAM ) {
		HidAddress = (char*)HidRAM;    // Hidden RAM Exist
	}
	*NorRAM = a;
	*HidRAM = b;
	return HidAddress;
}

static void hidden_ram_test(void)
{
	size_t i;
	char *ptr = HiddenRAM();


	if (ptr == NULL) {
		terminal_flush();

		printf_term("No additionnal RAM found.\n");

		terminal_show();
		exit(0);
	}
	for (i = 0; i < 256000; i++)
		ptr[i] = 70;
	for (i = 0; i < 256000; i++)
		if (ptr[i] != 70) {
			terminal_flush();

			printf_term("Unreliable additionnal RAM.\n");

			terminal_show();
			exit(0);
		}
}

int AddIn_main(int isAppli, unsigned short OptionNum)
{
	if (setjmp(main_end) == 0) {
		hidden_ram_test();
		//CCompiler("\\\\crd0\\TEST.c");
		CCompiler("\\\\fls0\\TEST.c");

		#ifdef MEMCHECK
		memcheck_recap();
		#endif
	}
	return status;
}

void exit(int code)
{
	status = code;
	longjmp(main_end, 1);
}


//****************************************************************************
//**************                                              ****************
//**************                 Notice!                      ****************
//**************                                              ****************
//**************  Please do not change the following source.  ****************
//**************                                              ****************
//****************************************************************************


#pragma section _BR_Size
unsigned long BR_Size;
#pragma section


#pragma section _TOP

//****************************************************************************
//  InitializeSystem
//
//  param   :   isAppli   : 1 = Application / 0 = eActivity
//              OptionNum : Option Number (only eActivity)
//
//  retval  :   1 = No error / 0 = Error
//
//****************************************************************************
int InitializeSystem(int isAppli, unsigned short OptionNum)
{
    return INIT_ADDIN_APPLICATION(isAppli, OptionNum);
}

#pragma section

