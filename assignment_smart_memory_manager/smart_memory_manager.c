#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>

#define MIN_POOL_SIZE 1
#define MAX_POOL_SIZE 1000000
#define INITIAL_CAPACITY 4

typedef struct {
    int start;
    int size;
    int allocationId; 
    int isFree;
} Block;

unsigned char *memoryPool = NULL;
Block *blocks = NULL;
int poolSize = 0;
int blockCount = 0;
int capacity = 0;
long long nextAllocationId = 1;
int allocationStrategy = 1;

int readInteger(const char *message, int *value) {
    char input[100];
    char *end;
    while (1) {
        printf("%s", message);
        if(fgets(input, sizeof(input), stdin) == NULL) {
            return 0;
        }

        errno = 0;
        long result = strtol(input, &end, 10);
        while (isspace((unsigned char)*end)) {
            end++;
        }
        if(end == input || *end != '\0' || errno == ERANGE || result < INT_MIN || result > INT_MAX) {
            printf("Invalid input. Please enter an integer.\n");
            continue;
        }
        *value = (int)result;
        return 1;
    }
}

int ensureCapacity() {
    if(blockCount < capacity) {
        return 1;
    }
    Block *temp = realloc(blocks, capacity * 2 * sizeof(Block));
    if(temp == NULL) {
        printf("Unable to expand metadata array.\n");
        return 0;
    }

    blocks = temp;
    capacity *= 2;
    return 1;
}

int insertBlock(int index, Block b) {
    if(!ensureCapacity()) {
        return 0;
    }

    for(int i = blockCount; i > index; i--) {
        blocks[i] = blocks[i - 1];
    }
    blocks[index] = b;
    blockCount++;
    return 1;
}

void removeBlock(int index) {
    for (int i = index; i < blockCount - 1; i++) {
        blocks[i] = blocks[i + 1];
    }
    blockCount--;
}

int findById(int allocationId) {
    for(int i = 0; i < blockCount; i++) {
        if(!blocks[i].isFree && blocks[i].allocationId == allocationId) {
            return i;
        }
    }
    return -1;
}

void mergeFreeBlocks() {
    int i = 0;
    while(i < blockCount - 1) {
        if(blocks[i].isFree && blocks[i + 1].isFree) {
            blocks[i].size += blocks[i + 1].size;
            removeBlock(i + 1);
        } 
        else {
            i++;
        }
    }
}

int assignBlock(int index, int size, int allocationId) {
    int leftover = blocks[index].size - size;

    if (leftover > 0) {
        Block rest = { blocks[index].start + size, leftover, -1, 1 };
        if(!insertBlock(index + 1, rest)) {
            return 0;     
        }
    }

    blocks[index].size = size;
    blocks[index].allocationId = allocationId;
    blocks[index].isFree = 0;
    return 1;
}

int initializePool() {
    while (1) {
        if(!readInteger("Enter memory pool size: ", &poolSize)) {
            return 0;
        }
        if(poolSize >= MIN_POOL_SIZE && poolSize <= MAX_POOL_SIZE) {
            break;
        }
        printf("Invalid pool size. Enter between 1 and 1000000.\n");
    }

    memoryPool = malloc(poolSize);
    capacity = INITIAL_CAPACITY;
    blocks = malloc(capacity * sizeof(Block));

    if(memoryPool == NULL || blocks == NULL) {
        printf("Memory allocation failed.\n");
        free(memoryPool);
        free(blocks);
        return 0;
    }

    memset(memoryPool, 0, poolSize);

    blockCount = 1;
    blocks[0] = (Block){ 0, poolSize, -1, 1 };
    printf("Memory pool initialized successfully.\n");
    return 1;
}

int firstFit(int requestedSize) {
    for(int i = 0; i < blockCount; i++) {
        if(blocks[i].isFree && blocks[i].size >= requestedSize) {
            return i;
        }
    }
    return -1;
}

int bestFit(int requestedSize) {
    int best = -1;
    for (int i = 0; i < blockCount; i++) {
        if (blocks[i].isFree && blocks[i].size >= requestedSize &&
            (best == -1 || blocks[i].size < blocks[best].size)) {
            best = i;
        }
    }
    return best;
}

int worstFit(int requestedSize) {
    int worst = -1;
    for (int i = 0; i < blockCount; i++) {
        if (blocks[i].isFree && blocks[i].size >= requestedSize &&
            (worst == -1 || blocks[i].size > blocks[worst].size)) {
            worst = i;
        }
    }
    return worst;
}

int findSuitableBlock(int requestedSize) {
    if (allocationStrategy == 1) return firstFit(requestedSize);
    if (allocationStrategy == 2) return bestFit(requestedSize);
    return worstFit(requestedSize);
}


void checkFragmentation(int requestedSize) {
    int totalFree = 0, largest = 0;
    for(int i = 0; i < blockCount; i++) {
        if(blocks[i].isFree) {
            totalFree += blocks[i].size;
            if (blocks[i].size > largest) {
                largest = blocks[i].size;
            }
        }
    }

    if(totalFree >= requestedSize && largest < requestedSize) {
        printf("\nExternal Fragmentation Detected!\n");
        printf("Requested size: %d bytes\n", requestedSize);
        printf("Total free memory: %d bytes\n", totalFree);

        printf("Individual free blocks: ");
        for(int i = 0; i < blockCount; i++) {
            if (blocks[i].isFree) {
                printf("%d ", blocks[i].size);
            }
        }
        printf("\nLargest contiguous free block: %d bytes\n", largest);
        printf("Free memory is sufficient in total, "
               "but divided into smaller blocks.\n");
    }
}

void allocateMemory(int requestedSize) {
    if(requestedSize <= 0 || requestedSize > poolSize) {
        printf("Invalid allocation size.\n");
        return;
    }
    if(nextAllocationId > INT_MAX) {
        printf("Allocation ID limit reached.\n");
        return;
    }

    int index = findSuitableBlock(requestedSize);
    if(index == -1) {
        printf("Allocation failed: no contiguous free block is large enough.\n");
        checkFragmentation(requestedSize);
        return;
    }

    int id = (int)nextAllocationId;
    if(!assignBlock(index, requestedSize, id)) {
        return;
    }
    memset(memoryPool + blocks[index].start, id, requestedSize);
    nextAllocationId++;
    printf("\nAllocation successful!\n");
    printf("ID: %d | Start: %d | Size: %d bytes\n", id, blocks[index].start, requestedSize);
}

void releaseMemory(int allocationId) {
    int index = (allocationId > 0) ? findById(allocationId) : -1;

    if(index == -1) {
        printf("Invalid ID or allocation already released.\n");
        return;
    }
    blocks[index].isFree = 1;
    blocks[index].allocationId = -1;
    mergeFreeBlocks();
    printf("Allocation ID %d released successfully.\n", allocationId);
}


void resizeMemory(int allocationId, int newSize) {
    if(allocationId <= 0 || newSize <= 0 || newSize > poolSize) {
        printf("Invalid allocation ID or size.\n");
        return;
    }

    int index = findById(allocationId);
    if(index == -1) {
        printf("Allocation ID not found.\n");
        return;
    }
    int oldStart = blocks[index].start;
    int oldSize = blocks[index].size;
    int hasFreeNext = (index + 1 < blockCount && blocks[index + 1].isFree);

    if (newSize == oldSize) {
        printf("Allocation already has the requested size.\n");
        return;
    }

    if (newSize < oldSize) {
        int extra = oldSize - newSize;

        if(hasFreeNext) {
            blocks[index + 1].start = oldStart + newSize;
            blocks[index + 1].size += extra;
        } 
        else {
            Block freed = { oldStart + newSize, extra, -1, 1 };
            if(!insertBlock(index + 1, freed)) {
                return;
            }
        }
        blocks[index].size = newSize;
        printf("Allocation resized successfully.\nNew size: %d bytes\n", newSize);
        return;
    }

    int extra = newSize - oldSize;

    if(hasFreeNext && blocks[index + 1].size >= extra) {
        if (blocks[index + 1].size == extra) {
            removeBlock(index + 1);
        } 
        else {
            blocks[index + 1].start += extra;
            blocks[index + 1].size -= extra;
        }

        blocks[index].size = newSize;
        memset(memoryPool + oldStart + oldSize, allocationId, extra);
        printf("Allocation expanded in place successfully.\nNew size: %d bytes\n",
               newSize);
        return;
    }

    int newIndex = findSuitableBlock(newSize);
    if(newIndex == -1) {
        printf("Resize failed: insufficient contiguous free memory.\n");
        printf("The original allocation is unchanged.\n");
        checkFragmentation(newSize);
        return;
    }

    int newStart = blocks[newIndex].start;
    if(!assignBlock(newIndex, newSize, allocationId)) {
        printf("The original allocation is unchanged.\n");
        return;
    }

    memcpy(memoryPool + newStart, memoryPool + oldStart, oldSize);
    memset(memoryPool + newStart + oldSize, allocationId, extra);
    for(int i = 0; i < blockCount; i++) {
        if (blocks[i].start == oldStart) {
            blocks[i].isFree = 1;
            blocks[i].allocationId = -1;
            break;
        }
    }
    mergeFreeBlocks();

    printf("Allocation moved and resized successfully.\n");
    printf("Allocation ID: %d\nNew start: %d\nNew size: %d bytes\n", allocationId, newStart, newSize);
}

void displayMemoryMap() {
    printf("\n========== MEMORY MAP ==========\n");
    printf("Memory Pool: %d bytes\n\n", poolSize);
    for (int i = 0; i < blockCount; i++) {
        if(blocks[i].isFree) {
            printf("[Free: %d bytes, Start: %d]\n", blocks[i].size, blocks[i].start);
        }
        else {
            printf("[Allocated #%d: %d bytes, Start: %d]\n", blocks[i].allocationId, blocks[i].size, blocks[i].start);
        }
    }
    printf("================================\n");
}

void displayStatistics() {
    int allocated = 0, free = 0, allocBlocks = 0, freeBlocks = 0, largest = 0;

    for(int i = 0; i < blockCount; i++) {
        if(blocks[i].isFree) {
            free += blocks[i].size;
            freeBlocks++;
            if (blocks[i].size > largest) {
                largest = blocks[i].size;
            }
        }
        else {
            allocated += blocks[i].size;
            allocBlocks++;
        }
    }
    printf("\n======= MEMORY STATISTICS =======\n");
    printf("Total memory: %d bytes\n", poolSize);
    printf("Allocated: %d bytes\n", allocated);
    printf("Free: %d bytes\n", free);
    printf("Allocated blocks: %d\n", allocBlocks);
    printf("Free blocks: %d\n", freeBlocks);
    printf("Largest free block: %d bytes\n", largest);
    printf("=================================\n");
}

int changeStrategy() {
    const char *names[] = { "First Fit", "Best Fit", "Worst Fit" };
    int choice;

    printf("\nSelect Allocation Strategy:\n");
    printf("1. First Fit\n2. Best Fit\n3. Worst Fit\n");
    do {
        if(!readInteger("Choice: ", &choice)) {
            return 0;
        }
        if(choice < 1 || choice > 3) {
            printf("Invalid strategy. Select 1, 2, or 3.\n");
        }
    } while (choice < 1 || choice > 3);

    allocationStrategy = choice;
    printf("Strategy set to %s.\n", names[choice - 1]);
    return 1;
}

int main() {
    int choice = 0, size, allocationId;
    if(!initializePool()) {
        return 1;
    }

    if(changeStrategy()) {
        do {
            printf("\n========== SMART MEMORY MANAGER ==========\n");
            printf("1. Allocate Memory\n");
            printf("2. Release Memory\n");
            printf("3. Resize Allocation\n");
            printf("4. Display Memory Map\n");
            printf("5. Display Memory Statistics\n");
            printf("6. Change Allocation Strategy\n");
            printf("7. Exit\n");

            if(!readInteger("Enter your choice: ", &choice)) {
                break;
            }

            switch (choice) {
                case 1:
                    if (readInteger("Enter requested size: ", &size)) {
                        allocateMemory(size);
                    }
                    break;
                case 2:
                    if (readInteger("Enter allocation ID: ", &allocationId)) {
                        releaseMemory(allocationId);
                    }
                    break;
                case 3:
                    if (readInteger("Enter allocation ID: ", &allocationId) && readInteger("Enter new size: ", &size)) {
                        resizeMemory(allocationId, size);
                    }
                    break;
                case 4: displayMemoryMap();    
                    break;
                case 5: displayStatistics();   
                    break;
                case 6: changeStrategy();      
                    break;
                case 7: printf("Exiting Smart Memory Manager...\n"); 
                    break;
                default: printf("Invalid menu choice.\n");
            }
        } while (choice != 7);
    }
    free(memoryPool);
    free(blocks);
    memoryPool = NULL;
    blocks = NULL;
    printf("Memory resources released successfully.\n");
    return 0;
}