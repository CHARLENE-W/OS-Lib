#include <iostream>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <winnt.h>
#include "Mmsystem.h"
using namespace std;
int main(int argc, char *argv[])
{
    int year, month, day, hour, minute, second, millisecond;
    SYSTEMTIME start, end;
    STARTUPINFO si;
    memset(&si, 0, sizeof(STARTUPINFO));
    si.wShowWindow = SW_SHOW;
    PROCESS_INFORMATION pi;
    if (argc == 2)
    {
        if (!CreateProcess(NULL, TEXT(argv[1]), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        {
            cout << "Create Fail" << endl;
            exit(0);
        }
    }
    else
    {
        char *str = strcat(argv[1], " ");
        str = strcat(str, argv[2]);
        if (!CreateProcess(NULL, TEXT(str), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        {
            cout << "Create Fail" << endl;
            exit(0);
        }
    }
    //load time
    GetSystemTime(&start);
    WaitForSingleObject(pi.hProcess, INFINITE);
    GetSystemTime(&end);

    //print data
    millisecond = end.wMilliseconds - start.wMilliseconds;
    year = end.wYear - start.wYear;
    month = end.wMonth - start.wMonth;
    day = end.wDay - start.wDay;
    hour = end.wHour - start.wHour;
    minute = end.wMinute - start.wMinute;
    second = end.wSecond - start.wSecond;
    if (millisecond < 0)
    {
        second--;
        millisecond += 1000;
    }
    if (second < 0)
    {
        minute--;
        second += 60;
    }
    if (minute < 0)
    {
        hour--;
        minute += 60;
    }
    if (hour < 0)
    {
        day--;
        hour += 24;
    }
    if (day < 0)
    {
        month--;
        day += 30;
    }
    if (month < 0)
    {
        year--;
        month += 12;
    }
    printf("��������ʱ�䣺");
    if (year > 0)
        printf("%d �� ", year);
    if (month > 0)
        printf("%d �� ", month);
    if (day > 0)
        printf("%d �� ", day);
    if (hour > 0)
        printf("%d ʱ ", hour);
    if (minute > 0)
        printf("%d �� ", minute);
    if (second > 0)
        printf("%d �� ", second);
    if (millisecond > 0)
        printf("%d ����", millisecond);
    printf("\n");
    return 0;
}
