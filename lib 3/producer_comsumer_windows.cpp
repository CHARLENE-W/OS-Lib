#include "Windows.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <time.h>
#define n_child_process 5
struct BUFFER //������
{
     int s[4];
     int head;
     int trail;
     bool IsEmpty;
};

PROCESS_INFORMATION StartClone(int nCloneID)
{
     TCHAR szFilename[MAX_PATH];
     TCHAR szCMDLine[MAX_PATH];
     ::GetModuleFileName(NULL, szFilename, MAX_PATH);
     ::sprintf(szCMDLine, "\"%s\" %d", szFilename, nCloneID);
     STARTUPINFO si;
     ::ZeroMemory(reinterpret_cast<void *>(&si), sizeof(si));
     si.cb = sizeof(si);
     PROCESS_INFORMATION pi;
     //�����ӽ���
     bool flag = CreateProcess(szFilename, szCMDLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
     //BOOL bCreatOK=CreateProcess(szFilename,szCmdLine,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);
     if (flag == false)
     {
          printf("create clone_process error\n");
          exit(0);
     }
     else
     {
          printf("CloneID:%d is created\n", nCloneID);
          return pi;
     }
}
HANDLE CreateShareMemory() //�����ļ��������������ļ�ӳ�������
{
     HANDLE hMapFile = CreateFileMapping(
         INVALID_HANDLE_VALUE,
         NULL,
         PAGE_READWRITE,
         0,
         sizeof(struct BUFFER),
         "buffer");
     if (!hMapFile)
     {
          printf("Create file mapping error:%d\n", GetLastError());
          exit(0);
     }

     LPVOID lpMap = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
     /*MapViewOfFile:��һ���ļ�ӳ�����ӳ�䵽��ǰӦ�ó���ĵ�ַ�ռ�
     ����1��hFileMappingObject ΪCreateFileMapping()��OpenFileMapping()���ص��ļ�ӳ���������
     ����2��dwDesiredAccess ӳ�������ļ����ݵķ��ʷ�ʽ������ͬ��Ҫ��CreateFileMapping()���������õı���������ƥ�䡣 ��ȡ����ֵ��
             FILE_MAP_ALL_ACCESS �ȼ���CreateFileMapping�� FILE_MAP_WRITE|FILE_MAP_READ. �ļ�ӳ����󱻴���ʱ����ָ��PAGE_READWRITE ѡ��. 
             FILE_MAP_COPY ���Զ�ȡ��д���ļ�.д������ᵼ��ϵͳΪ��ҳ�洴��һ�ݸ���.�ڵ���CreateFileMappingʱ���봫��PAGE_WRITECOPY��������.
             FILE_MAP_EXECUTE ���Խ��ļ��е�������Ϊ������ִ��.�ڵ���CreateFileMappingʱ���Դ���PAGE_EXECUTE_READWRITE��PAGE_EXECUTE_READ��������.
             FILE_MAP_READ ���Զ�ȡ�ļ�.�ڵ���CreateFileMappingʱ���Դ���PAGE_READONLY��PAGE_READWRITE��������.
             FILE_MAP_WRITE ���Զ�ȡ��д���ļ�.�ڵ���CreateFileMappingʱ���봫��PAGE_READWRITE��������.
     ����3��dwFileOffsetHigh ��ʾ�ļ�ӳ����ʼƫ�Ƶĸ�32λ.
     ����4��dwFileOffsetLow ��ʾ�ļ�ӳ����ʼƫ�Ƶĵ�32λ.(64KB���벻�Ǳ����)
     ����5��dwNumberOfBytes ָ��ӳ���ļ����ֽ���.0��ʾӳ�������ļ�
     */
     if (!lpMap)
          printf("MapViewOfFile  error:%d\n", GetLastError());
     else
          ZeroMemory(lpMap, sizeof(struct BUFFER));
     UnmapViewOfFile(lpMap);
     return hMapFile;
}
void show(struct BUFFER *tmp)
{

     printf("��ǰ��������");
     for (int i = tmp->head; i != tmp->trail;)
     {
          printf("%d ", tmp->s[i]);
          i++;
          if (i == 4)
               i = i % 4;
     }
     printf("\n");
}
void producer(int nCloneID)
{

     HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "buffer");
     if (!hMapFile)
          printf("Openfile mapping error:%d\n", GetLastError());

     LPVOID lpMap = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
     if (!lpMap)
          printf("MapView file  error:%d\n", GetLastError());

     // �������ڴ����ݿ�������
     struct BUFFER *tmp = (struct BUFFER *)lpMap;
     HANDLE empty = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "EMPTY");
     HANDLE full = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "FULL");
     HANDLE mutex = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "MUTEX");

     for (int i = 0; i < 6; i++)
     {
          int random_time = rand() % 5000;
          Sleep(random_time);

          WaitForSingleObject(empty, INFINITE); //P(empty)

          WaitForSingleObject(mutex, INFINITE); //P(mutex) ���²��ɽ����������һֱ����

          int num_pro = rand() % 100;
          tmp->s[tmp->trail] = num_pro;
          tmp->trail = (tmp->trail + 1) % 4;
          tmp->IsEmpty = false;
          SYSTEMTIME time;
          GetLocalTime(&time);
          printf("%04d/%02d/%02d %02d:%02d:%02d  ������ %d ���뻺����  %d�� ����\n", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, nCloneID, num_pro);

          if ((tmp->trail + 1) % 4 == tmp->head)
               printf("����������\n");
          show(tmp);
          printf("\n");

          ReleaseSemaphore(mutex, 1, NULL); //V(mutex);
          ReleaseSemaphore(full, 1, NULL);  //V(mutex);
     }

     UnmapViewOfFile(lpMap);
     lpMap = NULL;
     CloseHandle(hMapFile);
     CloseHandle(empty);
     CloseHandle(full);
     CloseHandle(mutex);
}

void comsumer(int nCloneID)
{
     HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "buffer");
     if (!hMapFile)
          printf("Openfile mapping error:%d\n", GetLastError());

     LPVOID lpMap = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
     if (!lpMap)
          printf("MapView file  error:%d\n", GetLastError());

     // �������ڴ����ݿ�������
     struct BUFFER *tmp = (struct BUFFER *)lpMap;
     HANDLE empty = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "EMPTY");
     HANDLE full = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "FULL");
     HANDLE mutex = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "MUTEX");

     for (int i = 0; i < 4; i++)
     {
          int random_time = rand() % 5000;
          Sleep(random_time);

          WaitForSingleObject(full, INFINITE);  //P(full)
          WaitForSingleObject(mutex, INFINITE); //P(mutex) ���²��ɽ����������һֱ����

          SYSTEMTIME time;
          GetLocalTime(&time);
          printf("%04d/%02d/%02d  %02d:%02d:%02d  ������ %d �ӻ�����ȡ�� %d�� ���� \n", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, nCloneID, tmp->s[tmp->head]);
          tmp->head = (tmp->head + 1) % 4;
          if (tmp->head == tmp->trail)
               tmp->IsEmpty = true;
          else
               tmp->IsEmpty = false;

          if (tmp->IsEmpty)
               printf("�������ѿ�\n");
          else
               show(tmp);
          printf("\n");

          ReleaseSemaphore(mutex, 1, NULL); //V(mutex);
          ReleaseSemaphore(empty, 1, NULL); //V(empty);
     }

     //���չر�
     UnmapViewOfFile(lpMap);
     lpMap = NULL;
     CloseHandle(hMapFile);
     CloseHandle(empty);
     CloseHandle(full);
     CloseHandle(mutex);
}

void parent(int nClone)
{
     HANDLE hMapFile = CreateShareMemory();
     if (!hMapFile)
          printf("Create file mapping error:%d\n", GetLastError());

     HANDLE hOpenMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "buffer");
     if (!hOpenMapFile)
          printf("OpenFileMapping error:%d\n", GetLastError());

     LPVOID lpMap = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
     if (!lpMap)
          printf("MapViewOfFile  error:%d\n", GetLastError());

     struct BUFFER *tmp = (struct BUFFER *)lpMap;
     tmp->head = 0;
     tmp->trail = 0;
     tmp->IsEmpty = true;
     HANDLE empty = CreateSemaphore(NULL, 3, 3, "EMPTY");
     HANDLE full = CreateSemaphore(NULL, 0, 3, "FULL");
     HANDLE mutex = CreateSemaphore(NULL, 1, 1, "MUTEX");
     UnmapViewOfFile(lpMap);
     lpMap = NULL;
     CloseHandle(hMapFile);
     PROCESS_INFORMATION pis[n_child_process];
     for (int i = 0; i < n_child_process; i++)
     {
          pis[i] = StartClone(++nClone);
     }
     for (int i = 0; i < n_child_process; i++)
     {
          WaitForSingleObject(pis[i].hProcess, INFINITE);
     }
     printf("press any key to close the window:\n");
     if (getchar())
     {
          for (int i = 0; i < n_child_process; i++)
          {
               CloseHandle(pis[i].hProcess);
          }
     }
}
int main(int argc, char *argv[])
{
     int nClone = 0;
     if (argc > 1) //�ӽ���
     {
          ::sscanf(argv[1], "%d", &nClone);
     }
     if (nClone == 0) //������
     {
          parent(nClone);
          return 0;
     }
     else if (nClone == 1 || nClone == 2) //nCloneID=1||2   ������
     {
          producer(nClone);
     }
     else //������
     {
          comsumer(nClone);
     }

     return 0;
}
