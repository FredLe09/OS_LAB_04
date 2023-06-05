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

#define UNDEFINED -1

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

void genProcess(int numberOfProcess, PCB *processArr[]);
void inputProcess(int numberOfProcess, PCB *processArr[]);
void printProcess(int numberOfProcess, PCB *processArr[]);
void exportGanttChart(int numberOfProcess, PCB *processArr[]);
void pushProcess(int *numberOfProcess, PCB *processArr[], int index, PCB *process);
void removeProcess(int *numberOfProcess, PCB *processArr[], int index);
int compareProcess(PCB *processA, PCB *processB, int iCriteria);
void swapProcess(PCB **processA, PCB **processB);
int checkPCBStart(PCB *process);
void setPCBStart(PCB *process, int timePoint);
void setPCBFinish(PCB *process, int timePoint);
int partition(PCB *processArr[], int low, int high, int iCriteria);
void quickSort(PCB *processArr[], int low, int high, int iCriteria);
void calculateAWT(int numberOfProcess, PCB *processArr[]);
void calculateATaT(int numberOfProcess, PCB *processArr[]);

int main(int argc, char *argv[])
{
    printf("===== SJF Scheduling =====\n");
    srand(time(NULL));
    // Initialize array of PCB
    PCB *InputArray[MAX_PCB];
    for (int i = 0; i < MAX_PCB; i++)
        InputArray[i] = NULL;

    PCB *ReadyQueue[MAX_PCB];
    for (int i = 0; i < MAX_PCB; i++)
        ReadyQueue[i] = NULL;

    PCB *TerArray[MAX_PCB];
    for (int i = 0; i < MAX_PCB; i++)
        TerArray[i] = NULL;

    // Input number of process
    // If no argument, input from keyboard
    // And manual input processes
    // Else, generate random number of process
    // And auto generate random number of process
    int iNumberOfProcess;
    if (argc < 2)
    {
        printf("Please input number of Process: ");
        scanf("%d", &iNumberOfProcess);
        inputProcess(iNumberOfProcess, InputArray);
    }
    else
    {
        iNumberOfProcess = atoi(argv[1]);
        genProcess(iNumberOfProcess, InputArray);
    }

    // Initialize number of InputArray, ReadyQueue, TerArray
    int iRemain = iNumberOfProcess, iReady = 0, iTer = 0;

    // Sort input array by arrival time
    // And print input array
    quickSort(InputArray, 0, iNumberOfProcess - 1,
              BY_ARRIVAL);
    printf("===== SJF Scheduling =====\n");
    printf("\nInput Array:\n");
    printProcess(iNumberOfProcess, InputArray);

    int step = 0;
    int timePoint = 0;

    // Check if any process is not terminated
    while (iTer < iNumberOfProcess)
    {
        // Check if ready queue is empty
        if (iReady == 0)
        {
            // timePoint is skipped to the early arrival time
            timePoint = InputArray[0]->iArrival;

            // Push all process that have same arrival time
            // into ready queue
            while (iRemain > 0 && InputArray[0]->iArrival <= timePoint)
            {
                pushProcess(&iReady, ReadyQueue,
                            iReady, InputArray[0]);
                removeProcess(&iRemain, InputArray, 0);
            }
        }
        // Else if ready queue is not empty
        else if (iReady > 0)
        {
            // Make sure that the first proecess in ready queue is running
            setPCBStart(ReadyQueue[0], timePoint);

            // Increase time point by burst time of running process
            // And set the running process to terminated
            timePoint += ReadyQueue[0]->iBurst;
            setPCBFinish(ReadyQueue[0], timePoint);

            // Push all process that have same arrival time
            // into ready queue
            while (iRemain > 0 && InputArray[0]->iArrival <= timePoint)
            {
                pushProcess(&iReady, ReadyQueue,
                            iReady, InputArray[0]);
                removeProcess(&iRemain, InputArray, 0);
            }

            // Move the terminated process to terminated queue
            pushProcess(&iTer, TerArray,
                        iTer, ReadyQueue[0]);
            removeProcess(&iReady, ReadyQueue, 0);
        }

        // Sort all process in ready queue by burst time
        quickSort(ReadyQueue, 0, iReady - 1,
                  BY_BURST);

        // Show the step by step with the following code
        // printf("\nStep %d at time: %d\n", ++step, timePoint);

        // printf("Ready Queue:\n");
        // printProcess(iReady, ReadyQueue);

        // printf("Terminated Queue:\n");
        // printProcess(iTer, TerArray);
    }

    printf("In Result\nTerminated Queue:\n");
    printProcess(iTer, TerArray);

    printf("\n===== SJF Scheduling =====\n");

    // Export Gantt Chart by TerArray
    exportGanttChart(iTer, TerArray);

    // Calculate Average Waiting Time and Average Turnaround Time
    calculateAWT(iTer, TerArray);
    calculateATaT(iTer, TerArray);

    // Free all allocated memory
    for (int i = 0; i < iRemain; i++)
        free(InputArray[i]);

    for (int i = 0; i < iReady; i++)
        free(ReadyQueue[i]);

    for (int i = 0; i < iTer; i++)
        free(TerArray[i]);

    return 0;
}

void genProcess(int n, PCB *arr[])
{
    for (int i = 0; i < n; i++)
    {
        arr[i] = (PCB *)malloc(sizeof(PCB));
        arr[i]->iPID = i + 1;
        arr[i]->iArrival = rand() % (MAX_ARRIVAL - MIN_ARRIVAL + 1) +
                           MIN_ARRIVAL;
        arr[i]->iBurst = rand() % (MAX_BURST - MIN_BURST + 1) +
                         MIN_BURST;
        arr[i]->iStart = UNDEFINED;
        arr[i]->iFinish = UNDEFINED;
        arr[i]->iWaiting = UNDEFINED;
        arr[i]->iResponse = UNDEFINED;
        arr[i]->iTaT = UNDEFINED;
    }
}

void inputProcess(int n, PCB *arr[])
{
    for (int i = 0; i < n; i++)
    {
        arr[i] = (PCB *)malloc(sizeof(PCB));
        printf("Process %d\n", i + 1);
        printf("Arrival Time: ");
        scanf("%d", &arr[i]->iArrival);
        printf("Burst Time: ");
        scanf("%d", &arr[i]->iBurst);
        arr[i]->iPID = i + 1;
        arr[i]->iStart = UNDEFINED;
        arr[i]->iFinish = UNDEFINED;
        arr[i]->iWaiting = UNDEFINED;
        arr[i]->iResponse = UNDEFINED;
        arr[i]->iTaT = UNDEFINED;
    }
}

void printProcess(int n, PCB *arr[])
{
    printf("------\n");
    printf("PID\tArrival\tBurst\tStart\tFinish\tResponse\tWaiting\tTaT\n");
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
    printf("------\n");
}

void writeLog(int *n, int arr[][2], int t, PCB *p)
{
    arr[*n][0] = t;
    arr[*n][1] = (p ? p->iPID : UNDEFINED);
    (*n)++;
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
    arr[*n - 1] = NULL;
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

int checkPCBStart(PCB *p)
{
    if (!p)
        return -1;
    else
        return (p->iStart == -1 ? 0 : 1);
}

void setPCBStart(PCB *p, int t)
{
    p->iStart = t;
    p->iResponse = p->iStart - p->iArrival;
}

void setPCBFinish(PCB *p, int t)
{
    p->iFinish = t;
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

void calculateAWT(int n, PCB *arr[])
{
    int sum = 0;
    for (int i = 0; i < n; i++)
        sum += arr[i]->iWaiting;

    printf("Average Waiting Time: %.2f\n", (float)sum / n);
}

void calculateATaT(int n, PCB *arr[])
{
    int sum = 0;
    for (int i = 0; i < n; i++)
        sum += arr[i]->iTaT;

    printf("Average Turnaround Time: %.2f\n", (float)sum / n);
}