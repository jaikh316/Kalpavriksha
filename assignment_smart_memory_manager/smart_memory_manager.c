#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>

#define MIN_POOL_SIZE 1
#define MAX_POOL_SIZE 1000000
#define INITIAL_CAPACITY 4
#define FIRST_FIT 1
#define BEST_FIT 2
#define WORST_FIT 3
#define MIN_STRATEGY_CHOICE FIRST_FIT
#define MAX_STRATEGY_CHOICE WORST_FIT
typedef struct {
    int start;
    int size;
    int allocationId; 
    int isFree;
} Block;

typedef struct {
    unsigned char *memoryPool;
    Block *blocks;
    int poolSize;
    int blockCount;
    int capacity;
    long long nextAllocationId;
    int allocationStrategy;
} MemoryManager;

MemoryManager manager = {NULL, NULL, 0, 0, 0, 1, FIRST_FIT};

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
        if(end == input) {
            printf("Invalid input. Please enter an integer.\n");
            continue;
        }
        while (isspace((unsigned char)*end)) {
            end++;
        }
        if(*end != '\0' || errno == ERANGE || result < INT_MIN || result > INT_MAX) {
            printf("Invalid input. Please enter an integer.\n");
            continue;
        }
        *value = (int)result;
        return 1;
    }
}

int ensureCapacity() {
    if(manager.blockCount < manager.capacity) {
        return 1;
    }
    Block *temp = realloc(manager.blocks, manager.capacity * 2 * sizeof(Block));
    if(temp == NULL) {
        printf("Unable to expand metadata array.\n");
        return 0;
    }

    manager.blocks = temp;
    manager.capacity *= 2;
    return 1;
}

int insertBlock(int index, Block b) {
    if(!ensureCapacity()) {
        return 0;
    }

    for(int blockIndex = manager.blockCount; blockIndex > index; blockIndex--) {
        manager.blocks[blockIndex] = manager.blocks[blockIndex - 1];
    }
    manager.blocks[index] = b;
    manager.blockCount++;
    return 1;
}

void removeBlock(int index) {
    for (int blockIndex = index; blockIndex < manager.blockCount - 1; blockIndex++) {
        manager.blocks[blockIndex] = manager.blocks[blockIndex + 1];
    }
    manager.blockCount--;
}

int findById(int allocationId) {
    for(int blockIndex = 0; blockIndex < manager.blockCount; blockIndex++) {
        if(!manager.blocks[blockIndex].isFree && manager.blocks[blockIndex].allocationId == allocationId) {
            return blockIndex;
        }
    }
    return -1;
}

void mergeFreeBlocks() {
    int blockIndex = 0;
    while(blockIndex < manager.blockCount - 1) {
        if(manager.blocks[blockIndex].isFree && manager.blocks[blockIndex + 1].isFree) {
            manager.blocks[blockIndex].size += manager.blocks[blockIndex + 1].size;
            removeBlock(blockIndex + 1);
        } 
        else {
            blockIndex++;
        }
    }
}

int assignBlock(int index, int size, int allocationId) {
    int leftover = manager.blocks[index].size - size;

    if (leftover > 0) {
        Block rest = { manager.blocks[index].start + size, leftover, -1, 1 };
        if(!insertBlock(index + 1, rest)) {
            return 0;     
        }
    }

    manager.blocks[index].size = size;
    manager.blocks[index].allocationId = allocationId;
    manager.blocks[index].isFree = 0;
    return 1;
}

int initializePool() {
    while (1) {
        if(!readInteger("Enter memory pool size: ", &manager.poolSize)) {
            return 0;
        }
        if(manager.poolSize >= MIN_POOL_SIZE && manager.poolSize <= MAX_POOL_SIZE) {
            break;
        }
        printf("Invalid pool size. Enter between 1 and 1000000.\n");
    }

    manager.memoryPool = malloc(manager.poolSize);
    manager.capacity = INITIAL_CAPACITY;
    manager.blocks = malloc(manager.capacity * sizeof(Block));

    if(manager.memoryPool == NULL || manager.blocks == NULL) {
        printf("Memory allocation failed.\n");
        free(manager.memoryPool);
        manager.memoryPool = NULL;
        free(manager.blocks);
        manager.blocks = NULL;
        return 0;
    }

    memset(manager.memoryPool, 0, manager.poolSize);

    manager.blockCount = 1;
    manager.blocks[0] = (Block){ 0, manager.poolSize, -1, 1 };
    printf("Memory pool initialized successfully.\n");
    return 1;
}

int firstFit(int requestedSize) {
    for(int blockIndex = 0; blockIndex < manager.blockCount; blockIndex++) {
        if(manager.blocks[blockIndex].isFree && manager.blocks[blockIndex].size >= requestedSize) {
            return blockIndex;
        }
    }
    return -1;
}

int bestFit(int requestedSize) {
    int best = -1;
    for (int blockIndex = 0; blockIndex < manager.blockCount; blockIndex++) {
        if (manager.blocks[blockIndex].isFree && manager.blocks[blockIndex].size >= requestedSize &&
            (best == -1 || manager.blocks[blockIndex].size < manager.blocks[best].size)) {
                
            best = blockIndex;
        }
    }
    return best;
}

int worstFit(int requestedSize) {
    int worst = -1;
    for (int blockIndex = 0; blockIndex < manager.blockCount; blockIndex++) {
        if (manager.blocks[blockIndex].isFree && manager.blocks[blockIndex].size >= requestedSize &&
            (worst == -1 || manager.blocks[blockIndex].size > manager.blocks[worst].size)) {
            worst = blockIndex;
        }
    }
    return worst;
}

int findSuitableBlock(int requestedSize) {
    if (manager.allocationStrategy == FIRST_FIT) return firstFit(requestedSize);
    if (manager.allocationStrategy == BEST_FIT) return bestFit(requestedSize);
    return worstFit(requestedSize);
}


void checkFragmentation(int requestedSize) {
    int totalFree = 0, largest = 0;
    for(int blockIndex = 0; blockIndex < manager.blockCount; blockIndex++) {
        if(manager.blocks[blockIndex].isFree) {
            totalFree += manager.blocks[blockIndex].size;

            if (manager.blocks[blockIndex].size > largest) {
                largest = manager.blocks[blockIndex].size;
            }
        }
    }

    if(totalFree >= requestedSize && largest < requestedSize) {
        printf("\nExternal Fragmentation Detected!\n");
        printf("Requested size: %d bytes\n", requestedSize);
        printf("Total free memory: %d bytes\n", totalFree);

        printf("Individual free blocks: ");
        for(int blockIndex = 0; blockIndex < manager.blockCount; blockIndex++) {
            if (manager.blocks[blockIndex].isFree) {
                printf("%d ", manager.blocks[blockIndex].size);
            }
        }
        printf("\nLargest contiguous free block: %d bytes\n", largest);
        printf("Free memory is sufficient in total, "
               "but divided into smaller blocks.\n");
    }
}

void allocateMemory(int requestedSize) {
    if(requestedSize <= 0 || requestedSize > manager.poolSize) {
        printf("Invalid allocation size.\n");
        return;
    }
    if(manager.nextAllocationId > INT_MAX) {
        printf("Allocation ID limit reached.\n");
        return;
    }

    int index = findSuitableBlock(requestedSize);
    if(index == -1) {
        printf("Allocation failed: no contiguous free block is large enough.\n");
        checkFragmentation(requestedSize);
        return;
    }

    int id = (int)manager.nextAllocationId;
    if(!assignBlock(index, requestedSize, id)) {
        return;
    }
    memset(manager.memoryPool + manager.blocks[index].start, id, requestedSize);
    manager.nextAllocationId++;
    printf("\nAllocation successful!\n");
    printf("ID: %d | Start: %d | Size: %d bytes\n", id, manager.blocks[index].start, requestedSize);
}


void releaseMemory(int allocationId) {
    if(allocationId <= 0) {
        printf("Invalid allocation ID. ID must be positive.\n");
        return;
    }
    int index = findById(allocationId);
    if(index == -1) {
        if (allocationId < manager.nextAllocationId) {
            printf("Release failed: Allocation ID %d has already been released.\n", allocationId);
        }
        else {
            printf("Release failed: Allocation ID %d does not exist.\n", allocationId);
        }
        return;
    }

    manager.blocks[index].isFree = 1;
    manager.blocks[index].allocationId = -1;
    mergeFreeBlocks();
    printf("Allocation ID %d released successfully.\n", allocationId);
}


int shrinkAllocation(int blockIndex, int newSize) {
    int oldStart = manager.blocks[blockIndex].start;
    int extraMemory = manager.blocks[blockIndex].size - newSize;

    if(blockIndex + 1 < manager.blockCount && manager.blocks[blockIndex + 1].isFree) {
        manager.blocks[blockIndex + 1].start = oldStart + newSize;
        manager.blocks[blockIndex + 1].size += extraMemory;
    }
    else {
        Block freedBlock = {oldStart + newSize, extraMemory, -1, 1};

        if(!insertBlock(blockIndex + 1, freedBlock)) {
            return 0;
        }
    }
    manager.blocks[blockIndex].size = newSize;
    printf("Allocation resized successfully.\n");
    printf("New size: %d bytes\n", newSize);
    return 1;
}


int expandAllocationInPlace(int blockIndex, int allocationId, int newSize) {
    int oldStart = manager.blocks[blockIndex].start;
    int oldSize = manager.blocks[blockIndex].size;
    int extraMemory = newSize - oldSize;

    if(blockIndex + 1 >= manager.blockCount || !manager.blocks[blockIndex + 1].isFree || manager.blocks[blockIndex + 1].size < extraMemory) {
        return 0;
    }

    if(manager.blocks[blockIndex + 1].size == extraMemory) {
        removeBlock(blockIndex + 1);
    }
    else {
        manager.blocks[blockIndex + 1].start += extraMemory;
        manager.blocks[blockIndex + 1].size -= extraMemory;
    }

    manager.blocks[blockIndex].size = newSize;

    memset(manager.memoryPool + oldStart + oldSize, allocationId, extraMemory);
    printf("Allocation expanded in place successfully.\n");
    printf("New size: %d bytes\n", newSize);
    return 1;
}


int moveAndResizeAllocation(int allocationId, int oldStart, int oldSize, int newSize) {
    int extraMemory = newSize - oldSize;
    int newBlockIndex = findSuitableBlock(newSize);

    if(newBlockIndex == -1) {
        printf("Resize failed: insufficient contiguous free memory.\n");
        printf("The original allocation is unchanged.\n");
        checkFragmentation(newSize);
        return 0;
    }

    int newStart = manager.blocks[newBlockIndex].start;

    if(!assignBlock(newBlockIndex, newSize, allocationId)) {
        printf("Unable to allocate a new block.\n");
        printf("The original allocation is unchanged.\n");
        return 0;
    }

    memcpy(manager.memoryPool + newStart, manager.memoryPool + oldStart, oldSize);
    memset(manager.memoryPool + newStart + oldSize, allocationId, extraMemory);

    for (int blockIndex = 0; blockIndex < manager.blockCount; blockIndex++) {
        if(manager.blocks[blockIndex].start == oldStart && manager.blocks[blockIndex].allocationId == allocationId) {
            manager.blocks[blockIndex].isFree = 1;
            manager.blocks[blockIndex].allocationId = -1;
            break;
        }
    }
    mergeFreeBlocks();
    printf("Allocation moved and resized successfully.\n");
    printf("Allocation ID: %d\n", allocationId);
    printf("New start: %d\n", newStart);
    printf("New size: %d bytes\n", newSize);
    return 1;
}


void resizeMemory(int allocationId, int newSize) {
    if(allocationId <= 0) {
        printf("Invalid allocation ID. ID must be positive.\n");
        return;
    }
    if(newSize <= 0 || newSize > manager.poolSize) {
        printf("Invalid resize size. Enter a size between 1 and %d.\n", manager.poolSize);
        return;
    }
    int blockIndex = findById(allocationId);
    if (blockIndex == -1) {
        if (allocationId < manager.nextAllocationId) {
            printf("Resize failed: Allocation ID %d has already been released.\n", allocationId);
        }
        else {
            printf("Resize failed: Allocation ID %d does not exist.\n", allocationId);
        }
        return;
    }

    int oldStart = manager.blocks[blockIndex].start;
    int oldSize = manager.blocks[blockIndex].size;
    if(newSize == oldSize) {
        printf("Allocation already has the requested size.\n");
        return;
    }

    if(newSize < oldSize) {
        shrinkAllocation(blockIndex, newSize);
        return;
    }
    if(expandAllocationInPlace(blockIndex, allocationId, newSize)) {
        return;
    }
    moveAndResizeAllocation(allocationId, oldStart, oldSize, newSize);
}


void displayMemoryMap() {
    printf("\n========== MEMORY MAP ==========\n");
    printf("Memory Pool: %d bytes\n\n", manager.poolSize);
    for (int blockIndex = 0; blockIndex < manager.blockCount; blockIndex++) {
        if(manager.blocks[blockIndex].isFree) {
            printf("[Free: %d bytes, Start: %d]\n", manager.blocks[blockIndex].size, manager.blocks[blockIndex].start);
        }
        else {
            printf("[Allocated #%d: %d bytes, Start: %d]\n", manager.blocks[blockIndex].allocationId, manager.blocks[blockIndex].size, manager.blocks[blockIndex].start);
        }
    }
    printf("================================\n");
}

void displayStatistics() {
    int allocated = 0, totalFree = 0, allocBlocks = 0, freeBlocks = 0, largest = 0;

    for(int blockIndex = 0; blockIndex < manager.blockCount; blockIndex++) {
        if(manager.blocks[blockIndex].isFree) {
            totalFree += manager.blocks[blockIndex].size;
            freeBlocks++;
            if (manager.blocks[blockIndex].size > largest) {
                largest = manager.blocks[blockIndex].size;
            }
        }
        else {
            allocated += manager.blocks[blockIndex].size;
            allocBlocks++;
        }
    }
    printf("\n======= MEMORY STATISTICS =======\n");
    printf("Total memory: %d bytes\n", manager.poolSize);
    printf("Allocated: %d bytes\n", allocated);
    printf("Free: %d bytes\n", totalFree);
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
        if(choice < MIN_STRATEGY_CHOICE  || choice > MAX_STRATEGY_CHOICE) {
            printf("Invalid strategy. Select 1, 2, or 3.\n");
        }
    } while (choice < MIN_STRATEGY_CHOICE  || choice > MAX_STRATEGY_CHOICE);

    manager.allocationStrategy = choice;
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
    free(manager.memoryPool);
    free(manager.blocks);
    manager.memoryPool = NULL;
    manager.blocks = NULL;
    printf("Memory resources released successfully.\n");
    return 0;
}