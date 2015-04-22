/*
 * Author: thirdstormofcythraul@outlook.com
 */
#include "print.h"

#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "memory_debug.h"

HANDLE mutex = 0;

void Print::myprintf(char *format, ...) {

	va_list pa;
	int n, myint;
	char *s, c;
	wchar_t *w;
	float f;
	FILE * pFile = 0;
	errno_t err;

	if (mutex == 0){
		 mutex = CreateMutex(NULL, FALSE, NULL);
	}
	WaitForSingleObject( mutex , INFINITE); // acquérir le mutex
	bool tryToWrite = true;
	if( (err  = fopen_s (&pFile, "\\windows\\temp\\log", "a")) != 0 ) {
		tryToWrite = false;
	}

	va_start(pa, format);
	while (*format != '\0') {
		if ( *format == '%' ) {
			switch (*++format) {
			case '%' : putchar('%'); break;
			case 'c' : /* affichage d'un caractère */ 
				c = va_arg(pa, char);
				putchar(c);
				if (tryToWrite)
					fprintf(pFile,  "%c", c);
				break;
			case 'x' : /* affichage d'un caractère */ 
				c = va_arg(pa, char);
				printf("%x", c);
				if (tryToWrite)
					fprintf(pFile,  "%x", c);
				break;
			case 'd' : /* affichage d'un entier */ 
				n = va_arg(pa, int);
				printf("%d", n); 
				if (tryToWrite)
					fprintf(pFile,  "%d", n);
				break;
			case 'u' : /* affichage d'un entier */ 
				n = va_arg(pa, long);
				printf("%u", n); 
				if (tryToWrite)
					fprintf(pFile,  "%u", n);
				break;
			case 'f' : /* affichage d'un float */ 
				f = (float)va_arg(pa, double);    /* !!!!! */
				printf("%f", f); 
				if (tryToWrite)
					fprintf(pFile,  "%f", f);
				break;
			case '0' : /* affichage d'un float */ 
				format++;
				format++;
				myint = va_arg(pa, unsigned char);
				printf("%002X", myint); 
				if (tryToWrite)
					fprintf(pFile,  "%002X", myint);
				break;
			case 's' : /* affichage d'une chaîne */ 
				s = va_arg(pa, char *);
				for ( ; *s != '\0'; s++ ) {
					putchar( *s );
					if (tryToWrite)
						fprintf(pFile,  "%c", *s);
				}
				break;
			case 'w' : /* affichage d'une wide chaîne */ 
				w = va_arg(pa, wchar_t *);
				for ( ; *w != L'\0'; w++ ) {
					putwchar( *w );
					if (tryToWrite)
						fwprintf(pFile,  L"%c", *w);
				}
				break;
			} /* end switch */
		}
		else {
			putchar( *format );
			if (tryToWrite)
					fprintf(pFile,  "%c", *format);
		}
		format++;
	}   
	va_end(pa);

	if (pFile)
		fclose(pFile);
	ReleaseMutex( mutex ); // libérer le mutex

} 

void Print::printBufferAsHexa(const unsigned char* buffer, int size){
#ifdef LOG
	for (int i = 0; i < size; i = i + 1 )
	{
		Print::myprintf("%x ", buffer[i]);
	}
	Print::myprintf( "\n");
#endif
}

void Print::printBuffer(const unsigned char* buffer, int size){
#ifdef LOG
	for (int i = 0; i < size; i = i + 1 )
	{
		Print::myprintf("%c", buffer[i]);
	}
	Print::myprintf( "\n");
#endif
}

void Print::printBuffer(const char* buffer, int size){
#ifdef LOG
	for (int i = 0; i < size; i = i + 1 )
	{
		Print::myprintf("%c", buffer[i]);
	}
	Print::myprintf( "\n");
#endif
}

bool Print::saveBuffer(const char* buffer, int size, const char* name){
#ifdef LOG
	FILE * pFile;
	errno_t err;
	if((err  = fopen_s (&pFile, name, "w")) != 0 ) {
		printf("Can't open file !\n");
		return false;
	}
	for (int i = 0; i < size; i = i + 2 )
	{
		fprintf(pFile,  "%c", buffer[i]);
	}
	Print::myprintf( "\n");
	fclose(pFile);
#endif
	return true;
}