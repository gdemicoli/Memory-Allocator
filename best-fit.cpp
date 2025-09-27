#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stack>
#include <list>
#include <unistd.h>

class Allocation
{
public:
    size_t size;
    void *space;

    Allocation(size_t s, void *p) : size(s), space(p) {}
};
std::stack<void *> allocStack;
std::list<Allocation *> allocatedList;
std::list<Allocation *> freeList;

size_t roundUp(size_t requested)
{
    if (requested <= 32)
        return 32;
    else if (requested <= 64)
        return 64;
    else if (requested <= 128)
        return 128;
    else if (requested <= 256)
        return 256;
    else if (requested <= 512)
        return 512;
    else
        throw std::runtime_error("Request too large");
}

bool roundCheck(size_t requested)
{
    if (requested == 32 || requested == 64 || requested == 128 || requested == 256 || requested == 512)
    {
        return true;
    }
    return false;
}

void *alloc(std::size_t chunkSize)
{
    std::cout << "alloc" << " " << chunkSize << std::endl;
    Allocation *bestFit = nullptr;

    size_t roundedChunk = roundUp(chunkSize);

    for (Allocation *a : freeList)
    {
        if (a->size == roundedChunk)
        {
            freeList.remove(a);
            allocatedList.push_back(a);
            return a;
        }
        if (a->size > roundedChunk)
        {
            if (bestFit == nullptr || bestFit->size > a->size)
            {
                bestFit = a;
            }
        }
    }

    if (bestFit)
    {
        size_t size = bestFit->size;
        while (roundCheck(size / 2) && size / 2 >= roundedChunk)
        {
            size = size / 2;
        }

        if (size < bestFit->size)
        {
            size_t originalSize = bestFit->size;

            // void*
        }
        freeList.remove(bestFit);
        allocatedList.push_back(bestFit);
        return bestFit;
    }

    else
    {
        Allocation *a = new Allocation(roundedChunk, sbrk(chunkSize));
        allocatedList.push_back(a);
        return a;
    }
}
void dealloc(void *chunk)
{
    std::cout << "dealloc" << std::endl;
}

int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <datafile>\n";
        return 1;
    }

    std::string fileName = argv[1];

    std::ifstream infile(fileName);
    if (!infile)
    {
        std::cerr << "Error opening file: " << fileName << "\n";
        return 1;
    }

    std::string line;
    while (std::getline(infile, line))
    {
        std::istringstream iss(line);
        std::string command;
        iss >> command;

        if (command == "alloc:")
        {
            std::size_t amount;
            iss >> amount;
            allocStack.push(alloc(amount)); // check if this can be used here... since it is keeping track. can move it into alloc
        }

        else if (command == "dealloc")
        {
            if (allocStack.size() == 0)
            {
                std::cerr << "Fatal Error: cannot dealocate memory that hasn't been allocated. \n";
                return 1;
            }

            dealloc(allocStack.top());
            allocStack.pop();
        }
    }
}