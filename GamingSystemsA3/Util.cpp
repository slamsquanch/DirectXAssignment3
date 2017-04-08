#include "Headers.h"

/*
Prints the formatted error to the debug output.

@param szFormat - The error string to be output with standard formatting
				  characters allowed
@param ... - The values to replace fields in the szFormat string
*/
void SetError(TCHAR* szFormat, ...) {
	TCHAR szBuffer[1024];
	va_list pArgList;

	va_start(pArgList, szFormat);

	_vsntprintf_s(szBuffer, sizeof(szBuffer) / sizeof(char), szFormat, pArgList);

	va_end(pArgList);

	OutputDebugString(szBuffer);
	OutputDebugString(TEXT("\n"));
}