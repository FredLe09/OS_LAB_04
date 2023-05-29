#include <stdio.h>
#include <stdlib.h>
#include <time.h>

enum Criteria
{
    BY_PID,
    BY_ARRIVAL,
    BY_BURST,
    BY_START
};

#define MAX_PCB 10

#define MIN_ARRIVAL 0
#define MAX_ARRIVAL 20
#define MIN_BURST 2
#define MAX_BURST 12

typedef struct
{
    int iPID;
    int iArrival, iBurst;
    int iStart, iFinish, iWaiting, iResponse, iTaT;
} PCB;

void inputProcess(int numberOfProcess, PCB *processArr[]);
void printProcess(int numberOfProcess, PCB *processArr[]);
void exportGanttChart(int numberOfProcess, PCB *processArr[]);
void pushProcess(int *numberOfProcess, PCB *processArr[], int index, PCB *process);
void removeProcess(int *numberOfProcess, PCB *processArr[], int index);
int compareProcess(PCB *processA, PCB *processB, int iCriteria);
void swapProcess(PCB **processA, PCB **processB);
void setPCBFinish(PCB *process, int startTime);
int partition(PCB *processArr[], int low, int high, int iCriteria);
void quickSort(PCB *processArr[], int low, int high, int iCriteria);
void calculateAWT(int numberOfProcess, PCB *processArr[]);
void calculateATaT(int numberOfProcess, PCB *processArr[]);

int main(int argc, char *argv[])
{
    printf("===== FCFS Scheduling =====\n");
    srand(time(NULL));
    PCB *InputArray[MAX_PCB];
    PCB *ReadyQueue[MAX_PCB];
    PCB *TerArray[MAX_PCB];

    int iNumberOfProcess;
    if (argc < 2)
    {
        printf("Please input number of Process: ");
        scanf("%d", &iNumberOfProcess);
    }
    else
        iNumberOfProcess = atoi(argv[1]);

    int iRemain = iNumberOfProcess, iReady = 0, iTer = 0;

    inputProcess(iNumberOfProcess, InputArray);
    quickSort(InputArray, 0, iNumberOfProcess - 1,
              BY_ARRIVAL);
    printf("\nInput Array: ");
    printProcess(iNumberOfProcess, InputArray);

    int step = 0;

    while (iTer < iNumberOfProcess)
    {
        if (iReady == 0)
        {
            pushProcess(&iReady, ReadyQueue,
                        0, InputArray[0]);
            removeProcess(&iRemain, InputArray, 0);

            printf("\nStep %d", ++step);
            printf("\nReady Queue: ");
            printProcess(iReady, ReadyQueue);

            printf("\nTerminated Array: ");
            printProcess(iTer, TerArray);

            continue;
        }
        else if (iTer == 0 || ReadyQueue[0]->iArrival >= TerArray[iTer - 1]->iFinish)
            setPCBFinish(ReadyQueue[0], ReadyQueue[0]->iArrival);
        else
            setPCBFinish(ReadyQueue[0], TerArray[iTer - 1]->iFinish);

        while (iRemain > 0 && InputArray[0]->iArrival <= ReadyQueue[0]->iFinish)
        {
            pushProcess(&iReady, ReadyQueue,
                        iReady, InputArray[0]);
            removeProcess(&iRemain, InputArray, 0);
        }

        quickSort(ReadyQueue, 1, iReady - 1,
                  BY_BURST);

        printf("\nStep %d", ++step);
        printf("\nReady Queue: ");
        printProcess(iReady, ReadyQueue);

        printf("\nTerminated Array: ");
        printProcess(iTer, TerArray);

        pushProcess(&iTer, TerArray,
                    iTer, ReadyQueue[0]);
        removeProcess(&iReady, ReadyQueue, 0);
    }

    printf("\nIn Result\nTerminated Queue: ");
    printProcess(iTer, TerArray);

    printf("\n===== FCFS Scheduling =====\n");

    exportGanttChart(iTer, TerArray);
    quickSort(TerArray, 0, iTer - 1,
              BY_PID);
    calculateAWT(iTer, TerArray);
    calculateATaT(iTer, TerArray);

    for (int i = 0; i < iRemain; i++)
        free(InputArray[i]);

    for (int i = 0; i < iReady; i++)
        free(ReadyQueue[i]);

    for (int i = 0; i < iTer; i++)
        free(TerArray[i]);

    return 0;
}

void inputProcess(int n, PCB *arr[])
{
    for (int i = 0; i < n; i++)
    {
        arr[i] = (PCB *)malloc(sizeof(PCB));
        arr[i]->iPID = i + 1;
        arr[i]->iArrival = rand() % (MAX_ARRIVAL - MIN_ARRIVAL + 1) +
                           MIN_ARRIVAL;
        arr[i]->iBurst = rand() % (MAX_BURST - MIN_BURST + 1) +
                         MIN_BURST;
        arr[i]->iStart = -1;
        arr[i]->iFinish = -1;
        arr[i]->iWaiting = -1;
        arr[i]->iResponse = -1;
        arr[i]->iTaT = -1;
    }
}

void printProcess(int n, PCB *arr[])
{
    printf("\nPID\tArrival\tBurst\tStart\tFinish\tResponse\tWaiting\tTaT\n");
    for (int i = 0; i < n; i++)
        printf("%d\t%d\t%d\t%d\t%d\t%d\t\t%d\t%d\n",
               arr[i]->iPID,
               arr[i]->iArrival,
               arr[i]->iBurst,
               arr[i]->iStart,
               arr[i]->iFinish,
               arr[i]->iResponse,
               arr[i]->iWaiting,
               arr[i]->iTaT);
}

void exportGanttChart(int n, PCB *arr[])
{
    printf("\nGantt Chart:\n");
    if (arr[0]->iStart > 0)
        printf("0\t");

    for (int i = 0; i < n - 1; i++)
    {
        printf("%d\t", arr[i]->iStart);
        if (arr[i]->iFinish < arr[i + 1]->iStart)
            printf("%d\t", arr[i]->iFinish);
    }
    printf("%d\t%d\n", arr[n - 1]->iStart, arr[n - 1]->iFinish);

    if (arr[0]->iStart > 0)
        printf("| IDLE\t");
    for (int i = 0; i < n - 1; i++)
    {
        printf("| P%d\t", arr[i]->iPID);
        if (arr[i]->iFinish < arr[i + 1]->iStart)
            printf("| IDLE\t");
    }
    printf("| P%d\t|\t\n", arr[n - 1]->iPID);

    printf("\n");
}

void pushProcess(int *n, PCB *arr[], int ind, PCB *p)
{
    for (int i = *n; i > ind; i--)
        arr[i] = arr[i - 1];

    arr[ind] = p;
    (*n)++;
}

void removeProcess(int *n, PCB *arr[], int ind)
{
    for (int i = ind; i < *n - 1; i++)
        arr[i] = arr[i + 1];

    (*n)--;
}

int compareProcess(PCB *a, PCB *b, int c)
{
    int result = 0;
    switch (c)
    {
    case BY_PID:
        result = a->iPID - b->iPID;
        break;
    case BY_ARRIVAL:
        result = a->iArrival - b->iArrival;
        break;
    case BY_BURST:
        result = a->iBurst - b->iBurst;
        break;
    }
    return (result == 0 ? compareProcess(a, b, c - 1) : result);
}

void swapProcess(PCB **a, PCB **b)
{
    PCB *temp = *a;
    *a = *b;
    *b = temp;
}

void setPCBFinish(PCB *p, int s)
{
    p->iStart = s;
    p->iFinish = p->iStart + p->iBurst;
    p->iResponse = p->iStart - p->iArrival;
    p->iTaT = p->iFinish - p->iArrival;
    p->iWaiting = p->iTaT - p->iBurst;
}

int partition(PCB *arr[], int l, int h, int c)
{
    int i = l - 1;
    for (int j = l; j < h; j++)
        if (compareProcess(arr[j], arr[h], c) < 0)
            swapProcess(&arr[++i], &arr[j]);

    swapProcess(&arr[i + 1], &arr[h]);
    return i + 1;
}

void quickSort(PCB *arr[], int l, int h, int c)
{
    if (l < h)
    {
        int pi = partition(arr, l, h, c);
        quickSort(arr, l, pi - 1, c);
        quickSort(arr, pi + 1, h, c);
    }
}

void calculateAWT(int numberOfProcess, PCB *arr[])
{
    int sum = 0;
    for (int i = 0; i < numberOfProcess; i++)
        sum += arr[i]->iWaiting;

    printf("Average Waiting Time: %.2f\n", (float)sum / numberOfProcess);
}

void calculateATaT(int numberOfProcess, PCB *arr[])
{
    int sum = 0;
    for (int i = 0; i < numberOfProcess; i++)
        sum += arr[i]->iTaT;

    printf("Average Turnaround Time: %.2f\n", (float)sum / numberOfProcess);
}