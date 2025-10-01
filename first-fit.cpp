#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stack>
#include <list>
#include <unistd.h>
#include <iomanip>

class Allocation
{
public:
    size_t size;
    void *space;
    size_t usedSize;

    Allocation(size_t s, void *p, size_t u = 0)
        : size(s), space(p), usedSize(std::min(u, s)) {}
};

std::stack<void *> allocStack;
std::list<Allocation *> allocatedList;
std::list<Allocation *> freeList;

void printLists()
{
    std::cout << "- Allocated Chunks -" << std::endl;
    std::cout << std::left
              << std::setw(20) << "Address"
              << std::setw(15) << "Used Size"
              << std::setw(15) << "Chunk Size" << std::endl;

    std::list<Allocation *>::iterator it;
    for (it = allocatedList.begin(); it != allocatedList.end(); ++it)
    {
        Allocation *a = *it;
        std::cout << std::left
                  << std::setw(20) << a->space
                  << std::setw(15) << a->usedSize
                  << std::setw(15) << a->size
                  << std::endl;
    }

    std::cout << std::endl;

    std::cout << "- Free Chunks -" << std::endl;
    std::cout << std::left
              << std::setw(20) << "Address"
              << std::setw(15) << "Used Size"
              << std::setw(15) << "Chunk Size" << std::endl;

    for (it = freeList.begin(); it != freeList.end(); ++it)
    {
        Allocation *f = *it;
        std::cout << std::left
                  << std::setw(20) << f->space
                  << std::setw(15) << f->usedSize
                  << std::setw(15) << f->size
                  << std::endl;
    }
}

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
        throw std::runtime_error("Error: Request too large");
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

    size_t roundedChunk = roundUp(chunkSize);

    auto firstFitIt = freeList.end();
    Allocation *firstFit = nullptr;

    for (auto i = freeList.begin(); i != freeList.end(); ++i)
    {
        Allocation *a = *i;

        if (a->size >= roundedChunk)
        {
            firstFit = a;
            firstFitIt = i;
            break;
        }
    }

    if (firstFit)
    {
        auto insertPos = firstFitIt;
        ++insertPos;                // position after bestFit
        freeList.erase(firstFitIt); // check if this connects adjacent nodes after removal
        size_t halfSize = firstFit->size;
        size_t size = firstFit->size;

        while (roundCheck(halfSize / 2) && halfSize / 2 >= roundedChunk)
        {

            halfSize = halfSize / 2;

            Allocation *leftover = new Allocation(size - halfSize, (char *)firstFit->space + halfSize);
            size = halfSize;
            freeList.insert(insertPos, leftover); // check the order of this is ok...
            insertPos--;
        }

        firstFit->size = size;
        firstFit->usedSize = chunkSize;
        allocatedList.push_back(firstFit);
        return firstFit->space;
    }

    else
    {
        void *ptr = sbrk(roundedChunk);
        if (ptr == (void *)-1)
        {
            throw std::runtime_error("Error: sbrk failed");
        }

        Allocation *a = new Allocation(roundedChunk, ptr, chunkSize); // check for sbrk failure
        allocatedList.push_back(a);
        return a->space;
    }
}

void dealloc(void *chunk)
{
    auto memoryNodeIt = freeList.end();
    Allocation *memoryNode = nullptr;

    for (auto i = allocatedList.begin(); i != allocatedList.end(); ++i)
    {
        Allocation *a = *i;

        if (a->space == chunk)
        {
            memoryNode = a;
            memoryNodeIt = i;
            break;
        }
    }

    if (memoryNode == nullptr)
    {
        throw std::runtime_error("Error: Memory must be allocated before being deallocated.");
    }

    allocatedList.erase(memoryNodeIt);
    memoryNode->usedSize = 0;
    auto insertPos = freeList.begin();
    while (insertPos != freeList.end() && (*insertPos)->space < memoryNode->space)
    {
        ++insertPos;
    }
    freeList.insert(insertPos, memoryNode);

    return;
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
        try
        {
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
            else
            {
                std::cerr << "Fatal Error: input file is incorrect \n";
                return 1;
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
            return 1;
        }
    }

    printLists();
}