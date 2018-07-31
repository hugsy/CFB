#include <Windows.h>

#include "stdafx.h"

#include "client.h"


static BOOLEAN g_DoRun = FALSE;


VOID RunInterpreter()
{
	while (g_DoRun)
	{
		wprintf(CLI_PROMPT);
		
	}

	return;
}