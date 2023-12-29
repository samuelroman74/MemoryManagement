#include <tuple>
#include <string>
#include <cstdlib>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstring>
#include <bitset>
#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <functional>
#include <iterator>
#include "MemoryManager.h"

MemoryManager::MemoryManager(unsigned wordSize_, std::function<int(int, void*)> allocator) {
wordSize = wordSize_;
alFunction = allocator;

}

//Tentative implementation
MemoryManager::~MemoryManager(){

    manager.clear();
    
    if(ogPoint != nullptr){
    delete[] (char*) ogPoint;
    }
    
}

void MemoryManager::initialize(size_t sizeInWords){

numOfWords = sizeInWords;

ogPoint = new char[numOfWords * wordSize];

manager[ogPoint] = std::make_tuple(0, numOfWords, false);

}

void* MemoryManager::allocate(size_t sizeInBytes) {

int allocateWords = sizeInBytes/wordSize;

//if empty, don't do work!
if(manager.empty()){
    return nullptr;
}

//Calculates how many words
if (sizeInBytes % wordSize != 0){
    allocateWords++;
}


//Gets list, sends it to appropriate function, then deletes list. Wordshift is where the hole starts
void* list = getList();
int wordShift = alFunction(allocateWords, list);
delete[] (short int*) list;

//If didn't find place to put it, then quit!
if(wordShift == -1){
    return nullptr;
}

for( auto& i : manager){
//If there's an error it's prolly this if statement lol
//Where the hole starts is already in the map! Just find it and acknowledge

//There is a chance things like the temporary pointers and tuples fall out of scope and mess up here. Look for problems here first!
if(std::get<0>(i.second) == wordShift){


    //If there will be extra space afterwards
    if(allocateWords != std::get<1>(i.second) - wordShift ){
        
        //Make new tuple that is now hole
        int newFirst = std::get<0>(i.second) + allocateWords;
        std::tuple<int, int, bool> temp = std::make_tuple(newFirst, std::get<1>(i.second), false);

        //Make a pointer that will point to it
        char* tempP = (char*) i.first;
        void* whatPointer = tempP + sizeInBytes;
        
        //Make a new entry in map
        manager[whatPointer] = temp;

        //And now change original so that it is not a hole and is accurate with bounds
        std::get<2>(i.second) = true;
        std::get<1>(i.second) = newFirst;
        return i.first;
    }
    else{
        //If it perfectly fits! Hooray!
        std::get<2>(i.second) = true;
        return i.first;

    }
}

}

return nullptr;

}

void MemoryManager::free(void* address) {

auto i = manager.find(address);
std::get<2>(i->second) = false;


auto j = manager.find(address);
auto k = manager.find(address);
if(i != manager.begin()){
j--;
}
if(i++ != manager.end()){
i--;
k++;
}

//If there is something behind it, check if it's a hole and extend if so
if(j->second != i->second){
    if(std::get<2>(j->second) == false){
    std::get<1>(j->second) = std::get<1>(i->second);
    }
}

//If there is something ahead of it, check if it's a hole and pre-extend (?) if so
if(k->second != i->second){
    if(std::get<2>(k->second) == false){
        std::get<0>(k->second) = std::get<1>(i->second);
    }
}


//If there's something behind and in front, and they're both holes (which means their respective end and start values would line up), combine and delete k
if(j != i && k != i){
if(std::get<2>(j->second) == false && std::get<2>(k->second) == false){
std::get<1>(j->second) = std::get<1>(k->second);
manager.erase(i->first);
manager.erase(k->first);
}
else if(std::get<2>(j->second) == false || std::get<2>(k->second) == false){
manager.erase(i->first);
}
}

}

void* MemoryManager::getList(){

//Makes new array
int totalSize = manager.size();
//Potential error here! Returning short int pointer and not void pointer, but maybe not a problem
short int* list;
list = new short int[(totalSize*2) + 1];

//Will count holes and make sure things get put into new array
short int holeCount = 0;
int keepTrack = 1;
list[0] = holeCount;

//If there's a hole (i.e. part of the tuple says false), then increment holeCount, make next thing in list where the hole starts, make other thing the difference 

for( auto& i: manager){
    if(std::get<2>(i.second) == false){
	list[0]++;
        list[keepTrack] = std::get<0>(i.second);
        keepTrack++;
        list[keepTrack] = std::get<1>(i.second) - std::get<0>(i.second);
        keepTrack++;
    }
}

return list;

}

void MemoryManager::setAllocator(std::function<int(int, void *)> allocator){

    alFunction = allocator;
}

unsigned MemoryManager::getWordSize(){
    return wordSize;
}

void* MemoryManager::getMemoryStart(){
    return ogPoint;
}

unsigned MemoryManager::getMemoryLimit(){
    return wordSize * numOfWords;
}

void* MemoryManager::getBitmap(){


    std::string bits = "";
    auto k = manager.end();

//Gets last hole
    while(k != manager.begin()){
        if(std::get<2>(k->second) != false){
            k--;
        }
        else{
            break;
        }
    }



//For every entry (until the last hole) add 1's if it's true and 0's if it's false

    for(auto& j: manager){
        if(std::get<2>(j.second) == true){
        for(int i = 0; i < std::get<1>(j.second) - std::get<0>(j.second); i++ ){
            bits = bits + "1";
        }
        }
        else{
            for(int i = 0; i < std::get<1>(j.second) - std::get<0>(j.second); i++){
                bits = bits + "0";
            }
        }

        //Make sure this comparison works. This is what breaks
        if(j == (*k)){
            break;
        }
    }


    //Fills the rest with 0's
    for(int i = 0; i < bits.length() % 8; i++) {
        bits = bits + "0";
    }


    int tempPosition = 0;
    uint8_t* bitMap = new uint8_t[(bits.length()/8) + 2];
   

    for(int i = 2; i < (bits.length() / 8) + 2; i++){
        std::string changeToInt = bits.substr(tempPosition, 8);
        tempPosition = tempPosition + 8;

        std::string reverse = "";
	int end = changeToInt.length() - 1;

        for (int ugh = 0; ugh < 8; ugh++){
            int digit = std::stoi(changeToInt.substr(end, 1));
            reverse = reverse + std::to_string(digit);
            end--;
        }
	

	bitMap[i] = std::stoi(reverse, nullptr, 2);
	

	}	
        std::bitset<16> numHoles(bits.length() / 8);
        std::string reverseAgain = numHoles.to_string();

        bitMap[1] = std::stoi(reverseAgain.substr(0, 8), nullptr, 2);
        bitMap[0] = std::stoi(reverseAgain.substr(8, 8), nullptr, 2);
    

//Adds an extra word if necessary and then adds two to capture all the words in it
//But first divides bit map size by 8 because that's how many bits a byte has

    return bitMap;


}

void MemoryManager::shutdown(){


        delete[] (char*) ogPoint;
        ogPoint = nullptr;
	manager.clear();

}

int MemoryManager::dumpMemoryMap(char* filename){


   void* list = getList();
   std::string memMap = "";
   memMap += "[";

   

   for (int i = 1; i < ((short int*)list)[0] * 2; i += 2){
	memMap += std::to_string(((short int*)list)[i]);
	memMap += ", ";
	memMap += std::to_string(((short int*)list)[i+1]);
	memMap += "]";

	if(i != (((short int*)list)[0] * 2) - 1){
	memMap += " - [";
} 
}

   char* killMe = new char [memMap.length() + 1];  
   std::strcpy(killMe, memMap.c_str());
   int fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0777);
   
   
   write(fd, killMe, memMap.length());
   delete[] (short int*) list;   
   close(fd);
   delete[] killMe;
   return 0;
}

int bestFit(int sizeInWords, void* list){

    //Gets max elements and initializes stuff
    int maxElements = ((short int*)list)[0];
    maxElements = (maxElements * 2) + 1;
    int placeReturn = -1;

    //If maxElements = 1 (i.e. there's no holes), then don't do work!
    if(maxElements == 1) {
        return -1;
    }

    //min offset initialized with first value
    int minOffset = ((short int*)list)[2] - sizeInWords;


    //for loop checks size of every hole, calculates whether it can fit sizeInWords, and if it can (and it's the smallest space so far), tells me where the hole starst
    for(int i = 2; i < maxElements; i += 2){
        int currentOffset = ((short int*)list)[i] - sizeInWords;
        if(minOffset >= currentOffset && currentOffset >= 0){
            minOffset = currentOffset;
            placeReturn = ((short int*)list)[i-1];
        }
    }

    return placeReturn;

}

int worstFit(int sizeInWords, void* list){

    //Gets max elements and initializes stuff
    int maxElements = ((short int*)list)[0];
    maxElements = (maxElements * 2) + 1;
    int placeReturn = -1;

    //If maxElements = 1 (i.e. there's no holes), then don't do work!
    if(maxElements == 1) {
        return -1;
    }

    //min offset initialized with first value
    int maxOffset = ((short int*)list)[2] - sizeInWords;


    //for loop checks size of every hole, calculates whether it can fit sizeInWords, and if it can (and it's the smallest space so far), tells me where the hole starst
    for(int i = 2; i < maxElements; i += 2){
        int currentOffset = ((short int*)list)[i] - sizeInWords;
        if(maxOffset <= currentOffset && currentOffset >= 0){
            maxOffset = currentOffset;
            placeReturn = ((short int*)list)[i-1];
        }
    }

    return placeReturn;

}

