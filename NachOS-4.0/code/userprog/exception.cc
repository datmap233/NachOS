// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "syscall.h"
#include "ksyscall.h"

#include <stdlib.h>

#define MaxFileLength 32
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions
//	is in machine.h.
//----------------------------------------------------------------------
void IncreasePC()
{
    // PrevPCReg, PCReg, NextPCReg
    // 4 byte each instruction

    /* set previous programm counter (debugging only)*/
    kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

    /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
    kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

    /* set next programm counter for brach execution */
    kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
}

// Copy buffer from User memory space to System memory space
// Buffer (char*)
char *User2System(int virtualAddr, int limit)
{
    int oneChar;
    char *kernelBuf = new char[limit + 1];
    if (kernelBuf == NULL)
        return kernelBuf;
    memset(kernelBuf, 0, limit + 1);

    for (int i = 0; i < limit; ++i)
    {
        kernel->machine->ReadMem(virtualAddr + i, 1, &oneChar);
        kernelBuf[i] = (char)oneChar;

        if (oneChar == 0)
            break;
    }
    return kernelBuf;
}
// Copy buffer from System memory space to User memory space
// Return: number of bytes copied (int)
int System2User(int virtualAddr, int len, char *buffer)
{
    if (len < 0)
        return -1;

    if (len == 0)
        return len;

    int i = 0;
    int oneChar = 0;

    do
    {
        oneChar = (int)buffer[i];
        kernel->machine->WriteMem(virtualAddr + i, 1, oneChar);
        i++;
    } while (i < len && oneChar != 0);

    return i;
}

void ExceptionHandler(ExceptionType which)
{
    int type = kernel->machine->ReadRegister(2);

    DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

    switch (which)
    {
    case NoException:
    {
        DEBUG(dbgSys, "Switch back to System\n");

        kernel->interrupt->setStatus(SystemMode);

        return;
    }
    case SyscallException:
    {
        // printf("The exception is %d\n", type);
        switch (type)
        {
        case SC_Halt:
        {
            DEBUG(dbgSys, "Shutdown, initiated by user program.\n");

            SysHalt();

            ASSERTNOTREACHED();
            break;
        }
        case SC_ReadNum:
        {
            DEBUG(dbgSys, "Syscall ReadNum\n");

            kernel->machine->WriteRegister(2, SysReadNum());

            IncreasePC();
            break;
        }
        case SC_PrintNum:
        {
            DEBUG(dbgSys, "Syscall ReadNum\n");

            SysPrintNum(kernel->machine->ReadRegister(4));

            IncreasePC();
            break;
        }
        case SC_ReadChar:
        {
            DEBUG(dbgSys, "Syscall ReadChar\n");

            kernel->machine->WriteRegister(2, (int)SysReadChar());

            IncreasePC();
            break;
        }
        case SC_PrintChar:
        {
            DEBUG(dbgSys, "Syscall PrintChar\n");

            SysPrintChar(kernel->machine->ReadRegister(4));

            IncreasePC();
            break;
        }
        case SC_RandomNum:
        {
            DEBUG(dbgSys, "Syscall RamdonNum\n");

            kernel->machine->WriteRegister(2, SysRandomNum());

            IncreasePC();
            break;
        }
        case SC_ReadString:
        {
            DEBUG(dbgSys, "Syscall ReadString\n");

            int virtAddrStr = kernel->machine->ReadRegister(4);
            int length = kernel->machine->ReadRegister(5);
            char *bufferInSystem = User2System(virtAddrStr, length);

            if (bufferInSystem == NULL)
                break;

            for (int i = 0; i < length; ++i)
                bufferInSystem[i] = kernel->synchConsoleIn->GetChar();

            System2User(virtAddrStr, length, bufferInSystem);

            delete bufferInSystem;

            IncreasePC();
            break;
        }
        case SC_PrintString:
        {
            DEBUG(dbgSys, "Syscall PrintString\n");

            int virtAddrStr = kernel->machine->ReadRegister(4);
            char *bufferInSystem = User2System(virtAddrStr, 255);

            if (bufferInSystem == NULL)
            {
                IncreasePC();
                break;
            }

            for (int i = 0; bufferInSystem[i] != '\0'; ++i)
                kernel->synchConsoleOut->PutChar(bufferInSystem[i]);

            delete bufferInSystem;

            IncreasePC();
            break;
        }
        case SC_PrintAscii:
        {
            DEBUG(dbgSys, "Syscall PrintAscii\n");
            SysPrintAscii();

            IncreasePC();
            break;
        }
        case SC_Add:
        {
            DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");

            /* Process SysAdd Systemcall*/
            int result;
            result = SysAdd(/* int op1 */ (int)kernel->machine->ReadRegister(4),
                            /* int op2 */ (int)kernel->machine->ReadRegister(5));

            DEBUG(dbgSys, "Add returning with " << result << "\n");
            /* Prepare Result */
            kernel->machine->WriteRegister(2, (int)result);

            /* Modify return point */
            IncreasePC();

            return;

            ASSERTNOTREACHED();

            break;
        }
        case SC_CreateFile:
        {
            // Input: Dia chi tu vung nho user cua ten file
            // Output: -1 = Loi, 0 = Thanh cong
            // Chuc nang: Tao ra file voi tham so la ten file
            int virtAddr;
            char *filename;
            DEBUG('a', "\n SC_CreateFile call ...");
            DEBUG('a', "\n Reading virtual address of filename");

            virtAddr = kernel->machine->ReadRegister(4); // Doc dia chi cua file tu thanh ghi R4
            DEBUG('a', "\n Reading filename.");

            // Sao chep khong gian bo nho User sang System, voi do dang toi da la (32 + 1) bytes
            filename = User2System(virtAddr, MaxFileLength + 1);
            if (strlen(filename) == 0)
            {
                printf("\n File name is not valid");
                DEBUG('a', "\n File name is not valid");
                kernel->machine->WriteRegister(2, -1); // Return -1 vao thanh ghi R2
                IncreasePC();
                break;
            }

            if (filename == NULL) // Neu khong doc duoc
            {
                printf("\n Not enough memory in system");
                DEBUG('a', "\n Not enough memory in system");
                kernel->machine->WriteRegister(2, -1); // Return -1 vao thanh ghi R2
                delete filename;
                IncreasePC();
                break;
            }
            DEBUG('a', "\n Finish reading filename.");

            if (!kernel->fileSystem->Create(filename)) // Tao file bang ham Create cua fileSystem, tra ve ket qua
            {
                // Tao file that bai
                printf("\n Error create file '%s'", filename);
                kernel->machine->WriteRegister(2, -1);
                delete filename;
                IncreasePC();
                break;
            }

            // Tao file thanh cong
            kernel->machine->WriteRegister(2, 0);
            delete filename;
            IncreasePC(); // Day thanh ghi lui ve sau de tiep tuc ghi
            break;
        }
        case SC_Open:
        {
            // OpenFileId Open(char *name);

            int virtAddr = kernel->machine->ReadRegister(4);
            char *fileName = User2System(virtAddr, MaxFileLength + 1);

            // if file name is NULL
            if (fileName == NULL)
            {
                DEBUG(dbgSys, "\nFilename is NULL !!!");
                cout << "\nFilename is NULL !!!" << endl; // Ouput error
                kernel->machine->WriteRegister(2, -1);    // return -1 to user space
                delete[] fileName;
                IncreasePC();
                break;
            }

            bool isExist = false;
            // Check if file is opened
            for (int i = 2; i < 20; ++i)
            {
                if (kernel->fileSystem->fileTable[i] != NULL)
                {
                    if (strcmp(kernel->fileSystem->fileTable[i]->name, fileName) == 0)
                    {
                        DEBUG(dbgSys, "\nFile is opened");
                        kernel->machine->WriteRegister(2, i);
                        delete[] fileName;
                        isExist = true;
                        IncreasePC();
                        break;
                    }
                }
            }
            if (isExist)
                break;

            int freeSlot = kernel->fileSystem->findEmptySlot(); // find empty slot from file table
            if (freeSlot == -1)                                 // if file table is full
            {
                DEBUG(dbgSys, "\nFull of file list");
                cout << "Full of file list" << endl;   // Output error
                kernel->machine->WriteRegister(2, -1); // return -1 to user space
                delete[] fileName;
                IncreasePC();
                break;
            }
            kernel->fileSystem->fileTable[freeSlot] = kernel->fileSystem->Open(fileName); // push OpenFile* into file table

            // if not found file
            if (!kernel->fileSystem->fileTable[freeSlot])
            {
                DEBUG(dbgSys, "\nError: File is not exist");
                cout << "Error: File is not exist" << endl;
                kernel->machine->WriteRegister(2, -1);
            }
            else
            {
                kernel->fileSystem->fileTable[freeSlot]->name = new char[MaxFileLength];
                strcpy(kernel->fileSystem->fileTable[freeSlot]->name, fileName); // add file name into file table
                DEBUG(dbgSys, "\nOpen successfully !!!");
                kernel->machine->WriteRegister(2, freeSlot);
            }
            delete[] fileName;
            IncreasePC();
            break;
        }
        case SC_Close:
        {
            /* Close the file, we're done reading and writing to it.
             * Return 1 on success, negative error code on failure
             */
            // int Close(OpenFileId id);
            int id = kernel->machine->ReadRegister(4);

            // if OpenFileId is outsize in range [0, 19]
            if (id < 0 || id >= 20)
            {
                cout << "OpenFileId is invalid" << endl;
                kernel->machine->WriteRegister(2, -1);
                IncreasePC();
                break;
            }

            // if file is not opened
            if (!kernel->fileSystem->fileTable[id])
            {
                cout << "Error close file..." << endl;
                kernel->machine->WriteRegister(2, -1);
            }
            else
            // if file is opened
            {
                delete kernel->fileSystem->fileTable[id]; // deallocated memory OpenFile* from file table
                kernel->fileSystem->fileTable[id] = NULL;
                kernel->machine->WriteRegister(2, 1); // Return 1 on success to user space
            }
            IncreasePC();
            break;
        }
        case SC_Read:
        {
            /* Read "size" bytes from the open file into "buffer".
             * Return the number of bytes actually read -- if the open file isn't
             * long enough, or if it is an I/O device, and there aren't enough
             * characters to read, return whatever is available (for I/O devices,
             * you should always wait until you can return at least one character).
             */
            // int Read(char *buffer, int size, OpenFileId id);
            // Console IO: Input(OpenFileID = 0), Output(OpenFileID = 1)
            int virtAddr = kernel->machine->ReadRegister(4);
            int size = kernel->machine->ReadRegister(5);
            int id = kernel->machine->ReadRegister(6);

            // if OpenFileId is outsize in range [0, 19]
            if (id < 0 || id >= 20)
            {
                cout << "OpenFileId is invalid" << endl;
                kernel->machine->WriteRegister(2, -1);
                IncreasePC();
                break;
            }

            // if file is not openend
            if (kernel->fileSystem->fileTable[id] == NULL)
            {
                cout << "Not to found this file in file table" << endl; // Output error
                kernel->machine->WriteRegister(2, -1);                  // return -1 to user space
                IncreasePC();
                break;
            }

            char *buffer = User2System(virtAddr, size);

            // if OpenFileID is Console Input ID or Console Output ID
            if (id == 0 || id == 1)
            {
                if (id == ConsoleIn) // if OpenFileId is ConsoleInput (id = 0)
                {
                    // read input value from keyboard and output into console
                    int numbytes = kernel->ReadConsole(buffer, size); // read input value from keyboard
                    kernel->machine->WriteRegister(2, numbytes);      // return bytes actually read to user space
                    System2User(virtAddr, size, buffer);              // return input value to user
                    IncreasePC();
                    delete[] buffer;
                    break;
                }
                // if OpenFileID is ConsoleOuput (id = 1): Output error and return -1 to user space
                cout << "Not to read console output" << endl;
                kernel->machine->WriteRegister(2, -1);
                IncreasePC();
                delete[] buffer;
                break;
            }

            // if OpenFileID >= 2
            int sizeBytes = kernel->fileSystem->fileTable[id]->Read(buffer, size); // read file content, add content into buffer and return bytes actually read
            if (sizeBytes >= 0)
            {
                System2User(virtAddr, sizeBytes, buffer);
                kernel->machine->WriteRegister(2, sizeBytes);
            }
            else
            {
                cout << "\nError reading file..." << endl;
                kernel->machine->WriteRegister(2, -1);
            }
            delete[] buffer;
            IncreasePC();
            break;
        }

        case SC_Write:
        {
            /* Write "size" bytes from "buffer" to the open file.
             * Return the number of bytes actually read on success.
             * On failure, a negative error code is returned.
             */
            // int Write(char *buffer, int size, OpenFileId id);
            int virtAddr = kernel->machine->ReadRegister(4);
            int size = kernel->machine->ReadRegister(5);
            int id = kernel->machine->ReadRegister(6);

            // if OpenFileId is outsize in range [0, 19]
            if (id < 0 || id >= 20)
            {
                cout << "OpenFileId is invalid " << endl;
                kernel->machine->WriteRegister(2, -1);
                IncreasePC();
                break;
            }

            // if file is not openend
            if (kernel->fileSystem->fileTable[id] == NULL)
            {
                cout << "File is not exist in file table" << endl;
                kernel->machine->WriteRegister(2, -1);
                IncreasePC();
                break;
            }
            char *buffer = User2System(virtAddr, size); // get char* buffer from user space

            // if OpenFileID is Console Input ID or Console Output ID

            if (id == 0 || id == 1)
            {
                if (id == ConsoleOut) // if OpenFileID is ConsoleOutput (id = 1)
                {
                    // Output buffer into console
                    if (buffer == NULL)
                        buffer = "";
                    int numbytes = kernel->WriteConsole(buffer, size);
                    kernel->machine->WriteRegister(2, numbytes);
                    delete[] buffer;
                    IncreasePC();
                    break;
                }
                // if OpenFileID is ConsoleInput (id = 0)
                cout << "Cannot write console input" << endl;
                kernel->machine->WriteRegister(2, -1);
                delete[] buffer;
                IncreasePC();
                break;
            }

            int numbytes = kernel->fileSystem->fileTable[id]->Write(buffer, size); // write content from char* buffer into opened file and return bytes actually write
            if (numbytes > 0)
                kernel->machine->WriteRegister(2, numbytes);
            else
            {
                cout << "Error write file" << endl;
                kernel->machine->WriteRegister(2, -1);
            }
            delete[] buffer;
            IncreasePC();
            break;
        }

        case SC_Seek:
        {
            /* Set the seek position of the open file "id"
             * to the byte "position".
             */
            // int Seek(int position, OpenFileId id);
            int position = kernel->machine->ReadRegister(4);
            int id = kernel->machine->ReadRegister(5);
            // if OpenFileId is outsize in range [0, 19]
            if (id < 0 || id >= 20)
            {
                cout << "OpenFileId is invalid" << endl;
                kernel->machine->WriteRegister(2, -1);
                IncreasePC();
                break;
            }
            // if OpenFileID is Console Input ID or Console Output ID

            if (id == 0 || id == 1)
            {
                cout << "Cannot seek into console input or console output" << endl;
                kernel->machine->WriteRegister(2, -1);
                IncreasePC();
                break;
            }

            // if file is not openend
            if (kernel->fileSystem->fileTable[id] == NULL)
            {
                cout << "File is not exist" << endl;
                kernel->machine->WriteRegister(2, -1);
                IncreasePC();
                break;
            }

            // if position = -1 => user want to move file cursor in the end of file
            if (position == -1)
                position = kernel->fileSystem->fileTable[id]->Length();

            // if position is invalid
            if (position > kernel->fileSystem->fileTable[id]->Length() || position < 0)
            {
                cout << "Position to seek is invalid" << endl;
                kernel->machine->WriteRegister(2, -1);
            }
            else
            {
                kernel->fileSystem->fileTable[id]->Seek(position); // Seek file cursor
                kernel->machine->WriteRegister(2, position);       // Return position to user space
            }
            IncreasePC();
            break;
        }

        case SC_Remove:
        {
            /* Remove a Nachos file, with name "name" */
            // int Remove(char *name);
            int virtAddr = kernel->machine->ReadRegister(4);
            char *name = User2System(virtAddr, MaxFileLength + 1);
            bool isOpening = false;

            // if filename is NULL
            if (name == NULL)
            {
                DEBUG(dbgSys, "\nFilename is NULL");
                kernel->machine->WriteRegister(2, -1);
                delete[] name;
                IncreasePC();
                break;
            }

            // check if file is opened
            for (int i = 2; i < 20; ++i)
            {
                if (kernel->fileSystem->fileTable[i] != NULL)
                {
                    if (strcmp(kernel->fileSystem->fileTable[i]->name, name) == 0)
                    {
                        DEBUG(dbgSys, "\nFile is opening...");
                        kernel->machine->WriteRegister(2, -1);
                        delete[] name;
                        IncreasePC();
                        isOpening = true;
                        break;
                    }
                }
            }

            if (isOpening == true)
                break;

            bool result = kernel->fileSystem->Remove(name); // remove file from filename
            if (result)
            {
                cout << "\nRemove " << name << " successfully !!!]\n";
                DEBUG(dbgSys, "\nRemove " << name << " successfully !!!");
                kernel->machine->WriteRegister(2, 0);
            }
            else
            {
                cout << "\nRemove " << name << " failed !!!\n";
                DEBUG(dbgSys, "\nRemove " << name << " failed !!!");
                kernel->machine->WriteRegister(2, -1);
            }
            delete[] name;
            IncreasePC();
            break;
        }
        default:
            cerr << "Unexpected system call " << type << "\n";
            break;
        }

        break;
    }
    case PageFaultException:
    {
        DEBUG(dbgSys, "No valid translaton found");
        SysHalt();

        break;
    }
    case ReadOnlyException:
    {
        DEBUG(dbgSys, "Write attempted to page marked read-only");
        SysHalt();

        break;
    }
    case BusErrorException:
    {
        DEBUG(dbgSys, "Translation resulted in on invalid physical address");
        SysHalt();

        break;
    }
    case AddressErrorException:
    {
        DEBUG(dbgSys, "Unaligned reference or one that was beyond the end of the address space");
        SysHalt();

        break;
    }
    case OverflowException:
    {
        DEBUG(dbgSys, "Integer overflow in add or sub");
        SysHalt();

        break;
    }
    case IllegalInstrException:
    {
        DEBUG(dbgSys, "Unimplemented or reserved instr");
        SysHalt();

        break;
    }
    case NumExceptionTypes:
    {
        SysHalt();

        break;
    }
    default:
    {
        cerr << "Unexpected user mode exception" << (int)which << "\n";
        break;
    }
    }
}
