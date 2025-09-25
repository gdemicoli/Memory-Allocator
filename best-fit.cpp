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

void *alloc(std::size_t chunkSize)
{
    std::cout << "alloc" << " " << chunkSize << std::endl;
    Allocation *bestFit = nullptr;

    for (Allocation *a : freeList)
    {
        if (a->size == chunkSize)
        {
            freeList.remove(a);
            allocatedList.push_back(a);
            return a;
        }
        if (a->size > chunkSize)
        {
            if (bestFit == nullptr || bestFit->size > a->size)
            {
                bestFit = a;
            }
        }
    }

    if (bestFit)
    {
        freeList.remove(bestFit);
        allocatedList.push_back(bestFit);
        return bestFit;
    }

    else
    {
        Allocation *a = new Allocation(chunkSize, sbrk(chunkSize));
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
    }
}