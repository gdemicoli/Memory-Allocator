#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stack>

class Allocation
{
public:
    size_t size;
    void *space;
};
std::stack<void *> allocStack;
Allocation *allocated_head = nullptr;
Allocation *free_head = nullptr;

void *alloc(std::size_t chunk_size)
{
    std::cout << "alloc" << " " << chunk_size << std::endl;
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