#pragma once
#include <map>
#include <string>
#include <functional>

class MemoryManager {

private:
std::map<void*, std::tuple<int, int, bool>> manager;   
unsigned wordSize;
unsigned numOfWords;
std::function<int(int, void*)> alFunction;
void* ogPoint;

public:
//This NEEDS FUNCTION ADDED TO MEM MANAGER
MemoryManager(unsigned wordSize, std::function<int(int, void*)> allocator);
~MemoryManager();
void initialize(size_t sizeInWords);
void shutdown();
void* allocate(size_t sizeInBytes);
void free(void* address);
void setAllocator(std::function<int(int,void*)> allocator);
int dumpMemoryMap(char* filename);
void* getList();
void* getBitmap();
unsigned getWordSize();
void* getMemoryStart();
unsigned getMemoryLimit();
};

int bestFit(int sizeInWords, void* list);
int worstFit(int sizeInWords, void* list);
