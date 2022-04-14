/**************************************************************
 *
 * userprog/ksyscall.h
 *
 * Kernel interface for systemcalls
 *
 * by Marcus Voelp  (c) Universitaet Karlsruhe
 *
 **************************************************************/

#ifndef __USERPROG_KSYSCALL_H__
#define __USERPROG_KSYSCALL_H__

#include "kernel.h"
#include <stdlib.h>
#include <math.h>
#include "synchconsole.h"

extern FileSystem  *fileSystem;


void SysHalt()
{
    kernel->interrupt->Halt();
}

int SysAdd(int op1, int op2)
{
    return op1 + op2;
}

int SysReadNum()
{
    // -2147483648 - 2147483647
    int rtnNumber = 0;
    int len = 0;
    bool isNegative = false, isValid = true;

    // Dummy init value
    char curr = '\0';

    curr = kernel->synchConsoleIn->GetChar();
    while (curr != '\0' && curr != '\n')
    {
        // If it > 10 or 11 with negative
        // Then it not valid so continue until '\0' or '\n'
        if ((!isNegative && len >= 10) || (isNegative && len >= 11))
            isValid = false;
        else if (len == 0 && curr == '-')
            isNegative = true;
        else if (len == 0 && curr == '0')
        {
            // Do nothing
        }
        else if (curr >= '0' && curr <= '9')
        {
            // Check if overflow
            if (((rtnNumber * 10 + (curr - '0')) < 0 && !isNegative && len == 9) || ((rtnNumber * 10 - (curr - '0')) > 0 && isNegative && len == 9))
            {
                isValid = false;
                len++;
            }
            // Negative in first char
            else if (len == 0 && isNegative)
            {
                rtnNumber = rtnNumber * 10 + (curr - '0');
                rtnNumber = -rtnNumber;
                len++;
            }
            else
            {
                if (isNegative)
                {
                    rtnNumber = rtnNumber * 10 - (curr - '0');
                    len++;
                }
                else
                {
                    rtnNumber = rtnNumber * 10 + (curr - '0');
                    len++;
                }
            }
        }
        else
            isValid = false;

        curr = kernel->synchConsoleIn->GetChar();
    }

    if (!isValid)
        return 0;

    return rtnNumber;
}

void SysPrintNum(int num)
{
    // -2147483648 - 2147483647
    char buffer[12];
    snprintf(buffer, 12, "%d", num);

    for (int i = 0; buffer[i] != '\0'; ++i)
        kernel->synchConsoleOut->PutChar(buffer[i]);
}

char SysReadChar()
{
    return kernel->synchConsoleIn->GetChar();
}

void SysPrintChar(char chr)
{
    kernel->synchConsoleOut->PutChar(chr);
}

int SysRandomNum()
{
    srand(time(NULL));
    return random();
}

void SysPrintAscii()
{
    for (int i = 32; i < 80; ++i)
    {
        SysPrintNum(i);
        SysPrintChar(':');
        SysPrintChar((char)i);
        SysPrintChar('\t');
        SysPrintNum(i + 47);
        SysPrintChar(':');
        SysPrintChar((char)(i + 47));
        SysPrintChar('\n');
    }
}


#endif /* ! __USERPROG_KSYSCALL_H__ */
