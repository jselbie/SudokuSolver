#include "stdafx.h"
#include "sudokuboard.h"


int main(int argc, char* argv[])
{
    SudokuBoard board;

    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " filename" << std::endl;
    }
    else
    {
        std::cout << "Loading: " << argv[1] << std::endl;
        bool loadresult = board.LoadFromFile(argv[1]);

        if (loadresult == false)
        {
            std::cout << "Failed to load board from file" << std::endl;
        }
        else
        {
            board.Solve();
        }
    }

	return 0;
}

