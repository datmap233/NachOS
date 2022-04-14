#include "syscall.h"

int main()
{
    // Alocating on stack, not using dynamic allocate
    // Cant using malloc or new
    char strBuffer[255];
    int length = 0;

    do
    {
        PrintString("Input the length of string (0 < length < 255): ");
        length = ReadNum();
    } while (length <= 0 || length >= 255);

    PrintString("Input string: ");
    ReadString(strBuffer, length);

    PrintString("String is: ");
    PrintString(strBuffer);
    PrintString("\n");
    Halt();
    return 0;
}