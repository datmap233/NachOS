#include "syscall.h"

void Sort(int *arr, int len, int direct)
{
    int tmp;
    int i, j;
    if (!direct)
    {
        for (i = 0; i < len - 1; i++)
        {
            for (j = 0; j < len - i - 1; j++)
            {
                if (arr[j] > arr[j + 1])
                {
                    tmp = arr[j];
                    arr[j] = arr[j + 1];
                    arr[j + 1] = tmp;
                }
            }
        }
    }
    else
    {
        for (i = 0; i < len - 1; i++)
        {
            for (j = 0; j < len - i - 1; j++)
            {
                if (arr[j] < arr[j + 1])
                {
                    tmp = arr[j];
                    arr[j] = arr[j + 1];
                    arr[j + 1] = tmp;
                }
            }
        }
    }
}

int main()
{
    int arr[100];
    int len = 0;
    int idx;
    int direct;

    PrintString("- Nhap chieu dai mang: ");
    len = ReadNum();
    while (len <= 0 || len > 100)
    {
        PrintString("! Nhap sai, nhap lai: ");
        len = ReadNum();
    }

    for (idx = 0; idx < len; ++idx)
    {
        PrintString("- Arr[");
        PrintNum(idx);
        PrintString("]: ");
        arr[idx] = ReadNum();
    }

    PrintString("\n- Nhap lua chon chieu tang dan hay giam dan(0 - tang, 1 - giam): ");
    direct = ReadNum();
    PrintString("Foo");
    while (direct < 0 || direct > 1)
    {
        PrintString("! Nhap sai, chi 0 hoac 1: ");
        direct = ReadNum();
    }

    Sort(arr, len, direct);

    PrintString("- Mang da duoc sap sep: ");
    for (idx = 0; idx < len; ++idx)
    {
        PrintNum(arr[idx]);
        PrintChar(' ');
    }

    PrintChar('\n');

    Halt();
    return 0;
}