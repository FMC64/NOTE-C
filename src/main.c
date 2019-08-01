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

static void strsonic_test(void)
{
	StrSonic sonic = StrSonic_init(NULL);

	StrSonic_add(&sonic, "ab", 0, NULL);
	StrSonic_add(&sonic, "cd", 0, NULL);
	StrSonic_add(&sonic, "ae", 0, NULL);
	StrSonic_add(&sonic, "str", 0, NULL);
	StrSonic_add(&sonic, "string", 0, NULL);
	StrSonic_add(&sonic, "string_test", 0, (void*)6);
	StrSonic_add(&sonic, "string_lol", 0, NULL);
	StrSonic_print(sonic);
	memcheck_stats();
	StrSonic_destroy(&sonic);
}

int AddIn_main(int isAppli, unsigned short OptionNum)
{
	if (setjmp(main_end) == 0) {
		malloc_unified_init();
		if (!CScope_keywords_init())
			return 0;
		if (!CStream_macro_init())
			return 0;

		//memcheck_test_fragmenting();
		CCompiler("\\\\crd0\\TEST.c");
		//CCompiler("\\\\fls0\\TEST.c");

		CStream_macro_quit();
		CScope_keywords_quit();
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

