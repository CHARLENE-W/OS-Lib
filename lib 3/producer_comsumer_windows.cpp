#include "Windows.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <time.h>
#define n_child_process 5
struct BUFFER //缓冲区
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
     //创建子进程
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
HANDLE CreateShareMemory() //创建文件共享区，返回文件映像对象句柄
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
     /*MapViewOfFile:将一个文件映射对象映射到当前应用程序的地址空间
     参数1：hFileMappingObject 为CreateFileMapping()或OpenFileMapping()返回的文件映像对象句柄。
     参数2：dwDesiredAccess 映射对象的文件数据的访问方式，而且同样要与CreateFileMapping()函数所设置的保护属性相匹配。 可取以下值：
             FILE_MAP_ALL_ACCESS 等价于CreateFileMapping的 FILE_MAP_WRITE|FILE_MAP_READ. 文件映射对象被创建时必须指定PAGE_READWRITE 选项. 
             FILE_MAP_COPY 可以读取和写入文件.写入操作会导致系统为该页面创建一份副本.在调用CreateFileMapping时必须传入PAGE_WRITECOPY保护属性.
             FILE_MAP_EXECUTE 可以将文件中的数据作为代码来执行.在调用CreateFileMapping时可以传入PAGE_EXECUTE_READWRITE或PAGE_EXECUTE_READ保护属性.
             FILE_MAP_READ 可以读取文件.在调用CreateFileMapping时可以传入PAGE_READONLY或PAGE_READWRITE保护属性.
             FILE_MAP_WRITE 可以读取和写入文件.在调用CreateFileMapping时必须传入PAGE_READWRITE保护属性.
     参数3：dwFileOffsetHigh 表示文件映射起始偏移的高32位.
     参数4：dwFileOffsetLow 表示文件映射起始偏移的低32位.(64KB对齐不是必须的)
     参数5：dwNumberOfBytes 指定映射文件的字节数.0表示映射整个文件
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

     printf("当前缓冲区：");
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

     // 将共享内存数据拷贝出来
     struct BUFFER *tmp = (struct BUFFER *)lpMap;
     HANDLE empty = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "EMPTY");
     HANDLE full = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "FULL");
     HANDLE mutex = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "MUTEX");

     for (int i = 0; i < 6; i++)
     {
          int random_time = rand() % 5000;
          Sleep(random_time);

          WaitForSingleObject(empty, INFINITE); //P(empty)

          WaitForSingleObject(mutex, INFINITE); //P(mutex) 上下不可交换，否则会一直阻塞

          int num_pro = rand() % 100;
          tmp->s[tmp->trail] = num_pro;
          tmp->trail = (tmp->trail + 1) % 4;
          tmp->IsEmpty = false;
          SYSTEMTIME time;
          GetLocalTime(&time);
          printf("%04d/%02d/%02d %02d:%02d:%02d  生产者 %d 放入缓存区  %d号 货物\n", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, nCloneID, num_pro);

          if ((tmp->trail + 1) % 4 == tmp->head)
               printf("缓存区已满\n");
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

     // 将共享内存数据拷贝出来
     struct BUFFER *tmp = (struct BUFFER *)lpMap;
     HANDLE empty = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "EMPTY");
     HANDLE full = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "FULL");
     HANDLE mutex = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "MUTEX");

     for (int i = 0; i < 4; i++)
     {
          int random_time = rand() % 5000;
          Sleep(random_time);

          WaitForSingleObject(full, INFINITE);  //P(full)
          WaitForSingleObject(mutex, INFINITE); //P(mutex) 上下不可交换，否则会一直阻塞

          SYSTEMTIME time;
          GetLocalTime(&time);
          printf("%04d/%02d/%02d  %02d:%02d:%02d  消费者 %d 从缓存区取出 %d号 货物 \n", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, nCloneID, tmp->s[tmp->head]);
          tmp->head = (tmp->head + 1) % 4;
          if (tmp->head == tmp->trail)
               tmp->IsEmpty = true;
          else
               tmp->IsEmpty = false;

          if (tmp->IsEmpty)
               printf("缓存区已空\n");
          else
               show(tmp);
          printf("\n");

          ReleaseSemaphore(mutex, 1, NULL); //V(mutex);
          ReleaseSemaphore(empty, 1, NULL); //V(empty);
     }

     //回收关闭
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
     if (argc > 1) //子进程
     {
          ::sscanf(argv[1], "%d", &nClone);
     }
     if (nClone == 0) //父进程
     {
          parent(nClone);
          return 0;
     }
     else if (nClone == 1 || nClone == 2) //nCloneID=1||2   生产者
     {
          producer(nClone);
     }
     else //消费者
     {
          comsumer(nClone);
     }

     return 0;
}
