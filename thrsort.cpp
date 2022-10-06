#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))


//Swap 2 values in an array, used within the sorting method
void swap(int* firstItem, int* secItem)
{
    int temp = *firstItem;
    *firstItem = *secItem;
    *secItem = temp;
}

//The real sort method. Handles the actual sorting of whatever list it is given in place.
void InPlaceSelectionSort(int *arr, int arrSize) {
    int indexSmallest = 0;
    for (int i = 0; i < arrSize - 1; ++i)
    {
        indexSmallest = i;
        for(int j = i + 1; j < arrSize; ++j)
        {
            if (arr[j] < arr[indexSmallest])
            {
                indexSmallest = j;
            }
        }
        swap(arr[i], arr[indexSmallest]);
    }
}

//Merging function. Will merge and sort the two input arrays. Assumes both arrays are already sorted.
void Merge(int *arr1, int *arr2, int arr1size, int arr2size, int* sortedList) {
    int iter1 = 0; int iter2 = 0; int iterMain = 0;

    //Main merge logic. Since inputs are assumed to be sorted already, simple comparative iteration is sufficient.
    while (iter1 < arr1size && iter2 < arr2size) {
        (arr1[iter1] < arr2[iter2]) ? sortedList[iterMain++] = arr1[iter1++] : sortedList[iterMain++] = arr2[iter2++];
    }

    //Leftovers handling. Properly loads final elements into sorted list.
    while (iter1 < arr1size) {
        sortedList[iterMain++] = arr1[iter1++];
    }
    while (iter2 < arr2size) {
        sortedList[iterMain++] = arr2[iter2++];
    }
}


//For threaded sort function input
typedef struct {
    int* original;
    int startIndex;
    int endIndex;
    int* results;
} SortArgs;

//For threaded merge function input
typedef struct {
    int* firstPart;
    int firstSize;
    int* secondPart;
    int secondSize;
    int* results;
} MergeArgs;

//Multi-threading sort method. Calls the actual sorting method within.
void* ThreadedSelectionSort(void* arg) {
    //Must type-cast back to its actual type
    SortArgs* myArgs = (SortArgs*) arg;

    InPlaceSelectionSort(myArgs->original + myArgs->startIndex, myArgs->endIndex - myArgs->startIndex);

    int* nextResult = myArgs->results;
    for (int i = myArgs->startIndex; i < myArgs->endIndex; ++i) {
        *nextResult++ = myArgs->original[i];
    }
    pthread_exit(0);
}

//Multi-threading Merge method. Calls the actual Merge method within.
void* ThreadedMerge(void* arg) {
    //Must type-cast back to its actual type
    MergeArgs* myArgs = (MergeArgs*) arg;

    Merge(myArgs->firstPart, myArgs->secondPart, myArgs->firstSize, myArgs->secondSize, myArgs->results);
}

//For printing out numerical arrays. Used in main and for testing
void printList(string label, int *list, int numElements) {
    cout << label << endl;
    for (int i = 0; i < numElements; ++i)
    {
        cout << list[i] << " ";
    }
    cout << endl;
}


//The real assignment, threading code. Used in main
int threadedCode(int *originalList, int size) {
    pthread_t threads[3];

    //Error tracking
    int err;

    //Relevant variables
    int numElements = size;
    int midpointIndex = (numElements + 1) / 2;
    int secondHalfElements = numElements - midpointIndex;

    //Creating the first half sorting inputs
    SortArgs firstHalfArgs;
    firstHalfArgs.original = originalList;
    firstHalfArgs.startIndex = 0;
    firstHalfArgs.endIndex = midpointIndex;

    //Creating the second half sorting inputs
    SortArgs secondHalfArgs;
    secondHalfArgs.original = originalList;
    secondHalfArgs.startIndex = midpointIndex;
    secondHalfArgs.endIndex = numElements;

    //Memory stuff. Automatically deallocates this memory once out of scope
    std::unique_ptr<int> firstHalfMem = std::unique_ptr<int> (new int[midpointIndex]);
    std::unique_ptr<int> secondHalfMem = std::unique_ptr<int> (new int[secondHalfElements]);
    std::unique_ptr<int> resultsMem = std::unique_ptr<int> (new int[numElements]);
    firstHalfArgs.results = firstHalfMem.get();
    secondHalfArgs.results = secondHalfMem.get();

    //Creating the merge halves inputs
    MergeArgs mergeArgs;
    mergeArgs.firstPart = firstHalfArgs.results;
    mergeArgs.secondPart = secondHalfArgs.results;
    mergeArgs.firstSize = midpointIndex;
    mergeArgs.secondSize = secondHalfElements;
    mergeArgs.results = resultsMem.get();

    //Thread creations and merges.
    err = pthread_create(&threads[0], NULL, ThreadedSelectionSort, &firstHalfArgs);
    if (err) {
        cout << "Thread 1 creation failed." << endl;
        return 1;
    }
    err = pthread_create(&threads[1], NULL, ThreadedSelectionSort, &secondHalfArgs);
    if (err) {
        cout << "Thread 2 creation failed." << endl;
        return 1;
    }
    for (int i = 0; i < 2; ++i) {
        err = pthread_join(threads[i], NULL);
    }
    if (err) {
        cout << "Thread joining failed." << endl;
        return 1;
    }

    err = pthread_create(&threads[2], NULL, ThreadedMerge, &mergeArgs);
    if (err) {
        cout << "Thread 3 creation failed." << endl;
        return 1;
    }

    err = pthread_join(threads[2], NULL);
    if (err) {
        cout << "Thread joining failed." << endl;
        return 1;
    }

    printList("Final List:", mergeArgs.results, numElements);
}

//Test case code that uses exact same sorting logic, just without the threads. Used for logic tests.
void simpleCode(int *originalList, int size) {
    int firstHalfSize = (size+1)/2;
    int firstHalf[firstHalfSize];
    for (int i = 0; i < firstHalfSize; ++i) {
        firstHalf[i] = originalList[i];
    }
    InPlaceSelectionSort(firstHalf, firstHalfSize);

    int secondHalfSize = size - firstHalfSize;
    int secondHalf[secondHalfSize];
    for (int i = 0; i < secondHalfSize; ++i) {
        secondHalf[i] = originalList[firstHalfSize + i];
    }
    InPlaceSelectionSort(secondHalf, secondHalfSize);

    int sortedList[size];

    Merge(firstHalf, secondHalf, firstHalfSize, secondHalfSize, sortedList);
    printList("Final List:", sortedList, size);
}

int main() {
    //Change this line to define the array to be sorted
    int originalList[] = {7,4,1,2,10,8,24,9,16,11,300};
    
    printList("Initial List:", originalList, ARRAY_SIZE(originalList));

    threadedCode(originalList, ARRAY_SIZE(originalList));

    return 0;

}



