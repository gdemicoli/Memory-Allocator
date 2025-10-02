#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stack>
#include <list>
#include <unistd.h>
#include <iomanip>
#include <stdexcept>

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
// Function for rounding requested chunk size up to standard piece
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
    return 0;
}
// Function for checking size is standard

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

    auto bestFitIt = freeList.end();
    Allocation *bestFit = nullptr;
    // Searches through free list for best fit
    for (auto i = freeList.begin(); i != freeList.end(); ++i)
    {
        Allocation *a = *i;

        if (a->size == roundedChunk)
        {
            bestFit = a;
            bestFitIt = i;
            break;
        }

        if (a->size > roundedChunk && (!bestFit || a->size < bestFit->size))
        {
            bestFit = a;
            bestFitIt = i;
        }
    }

    if (bestFit)
    {
        auto insertPos = bestFitIt;
        ++insertPos;
        freeList.erase(bestFitIt);
        size_t halfSize = bestFit->size;
        size_t size = bestFit->size;

        // If an adequate fit is found, we split the chunk if needed
        while (roundCheck(halfSize / 2) && halfSize / 2 >= roundedChunk)
        {

            halfSize = halfSize / 2;

            Allocation *leftover = new Allocation(size - halfSize, (char *)bestFit->space + halfSize);
            size = halfSize;
            freeList.insert(insertPos, leftover);
            insertPos--;
        }

        bestFit->size = size;
        bestFit->usedSize = chunkSize;
        allocatedList.push_back(bestFit);
        return bestFit->space;
    }
    // If no adequate chunk can be found we request memory from the OS
    else
    {
        void *ptr = sbrk(roundedChunk);
        if (ptr == (void *)-1)
        {
            throw std::runtime_error("Error: sbrk failed");
            return ptr;
        }

        Allocation *a = new Allocation(roundedChunk, ptr, chunkSize);
        allocatedList.push_back(a);
        return a->space;
    }
}

void dealloc(void *chunk)
{
    auto memoryNodeIt = freeList.end();
    Allocation *memoryNode = nullptr;

    // Search for chunk in list
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

    // insert freed chunk in a way that keeps the chunks contiguous (where free)
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
                allocStack.push(alloc(amount));
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