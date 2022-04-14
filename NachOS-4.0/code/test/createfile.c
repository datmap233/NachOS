#include "syscall.h"
#define MAX_LENGTH 32

int main()
{
    int length = 0;
    char fileName[MAX_LENGTH];
    PrintString("\nCREATE FILE\n");
    do
    {
        PrintString("Input the length of File Name (0 < length < 32): ");
        length = ReadNum();
    } while (length <= 0 || length >= MAX_LENGTH);
    
    PrintString("Input File Name: ");
    ReadString(fileName, length);
    // CreateFile(fileName);
    if (CreateFile(fileName) == 0) // Goi ham CreateFile
        PrintString(" -> Create file successful.\n\n");
    else
        PrintString(" -> Create file failed.\n\n");

    Halt();
    return 0;
}
