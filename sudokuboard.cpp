#include "stdafx.h"
#include "sudokuboard.h"
#include "cell.h"

SudokuBoard::SudokuBoard()
{
    Init();
}

bool SudokuBoard::Init()
{
    // Reset any existing state
    // ------------------------------------------
    for (int row = 0; row < 9; row++)
    {
        for (int col = 0; col < 9; col++)
        {
            m_board[row][col].Reset();
        }
    }

    for (int i = 0; i < 9; i++)
    {
        m_rows[i].Reset();
        m_cols[i].Reset();
        m_squares[i].Reset();
    }
    // ------------------------------------------

    // start of initialization
    for (int row = 0; row < 9; row++)
    {
        for (int col = 0; col < 9; col++)
        {
            m_board[row][col]._cellIndex = row * 9 + col;
        }
    }

    for (int row = 0; row < 9; row++)
    {
        for (int col = 0; col < 9; col++)
        {
            Cell *cell = &m_board[row][col];
            CellSet *colset = &m_cols[col];
            CellSet *rowset = &m_rows[row];
            int squareindex = 3*(row/3) + (col / 3);
            int squarecellindex = 3*(row%3) + (col %3);

            CellSet *square = &m_squares[squareindex];

            square->_set[squarecellindex] = cell;
            rowset->_set[col] = cell;
            colset->_set[row] = cell;

            cell->_row = rowset;
            cell->_column = colset;
            cell->_square = square;

            cell->_rowIndex = row;
            cell->_colIndex = col;
            cell->_squareIndex = squareindex;
            cell->_squareCellIndex = squarecellindex;
        }
    }


    return true;
}

bool SudokuBoard::LoadFromFile(const std::string& filename)
{
    std::ifstream infile(filename);

    char c;
    int row = 0;
    int col = 0;
    int value = 0;

    while (infile.is_open() && infile.good())
    {
        infile >> c;

        value = 0;
        if ((c >= '0') && (c <= '9'))
        {
            value = c - '0';
        }

        SetCellValue(row, col, value, true);

        col++;
        if (col == 9)
        {
            col = 0;
            row++;
        }
        if (row == 9)
        {
            break;
        }
    }

    if (row != 9)
    {
        Log("Error processing file!");
        return false;
    }

    return true;
}

bool SudokuBoard::Solve()
{
    bool fSolved = false;

    if (IsSolved())
        return true;

    int scancount = 0;
    std::string oldstate, state;
    state = GetBoardState();

    Log("\n");
    Dump();
    Log("\n");

    while(true)
    {
        oldstate = state;
        ScanForSolution();
        scancount++;
        state = GetBoardState();

        Log("\n");
        Dump();
        Log("\n");

        if (state == oldstate)
            break;


        fSolved = IsSolved();
        if (fSolved)
        {
            break;
        }
    }

    Log("Number of scans - %d", scancount);
    if (fSolved)
    {
        Log("Board has been solved");
    }
    else
    {
        Log("Board has not been solved");
        Log("");
    }

    bool fValid = IsValid();
    Log("%sBoard is%s valid", fValid?"":"WARNING - ", fValid?"":" NOT");

    return fSolved;
}



bool SudokuBoard::IsSolved()
{
    // it's solved if all 81 squares have a non-zero value and the board is valid

    for (int r = 0; r < 9; r++)
    {
        for (int c = 0; c < 9; c++)
        {
            if (m_board[r][c]._value == 0)
                return false;
        }
    }

    return true;
}





bool SudokuBoard::IsValid()
{
    // check for validity
    int squarecount[9][10] = {0};
    int rowcount[9][10] = {0};
    int colcount[9][10] = {0};

    for (int index = 0; index < 9; index++)
    {
        CellSet *square = &m_squares[index];
        CellSet *row = &m_rows[index];
        CellSet *col = &m_cols[index];

        for (int cellindex = 0; cellindex < 9; cellindex++)
        {
            int value;

            value = square->_set[cellindex]->_value;
            squarecount[index][value]++;

            value = row->_set[cellindex]->_value;
            rowcount[index][value]++;

            value = col->_set[cellindex]->_value;
            colcount[index][value]++;
        }
    }

    for (int index = 0; index < 9; index++)
    {
        for (int valueindex = 1; valueindex <= 9; valueindex++)
        {
            if (squarecount[index][valueindex] > 1)
                return false;

            if (rowcount[index][valueindex] > 1)
                return false;

            if (colcount[index][valueindex] > 1)
                return false;
        }
    }

    return true;
}





void SudokuBoard::LogWithoutLineBreak(const char *pwszFormat, ...)
{
    va_list args;
    va_start(args, pwszFormat);
    char szMsg[1024];

    vsnprintf(szMsg, 1024, pwszFormat, args);

    std::cout << szMsg;

    va_end(args);
}

void SudokuBoard::Log(const char *pwszFormat, ...)
{
    va_list args;
    va_start(args, pwszFormat);
    char szMsg[1024];

    vsnprintf(szMsg, 1024, pwszFormat, args);

    std::cout << szMsg << std::endl;

    va_end(args);
}


std::string SudokuBoard::GetBoardState()
{
    std::stringstream ss;

    for (int r = 0; r < 9; r++)
    {
        for (int c = 0; c < 9; c++)
        {
            ss << m_board[r][c]._value;
            ss << '.';
            ss << m_board[r][c]._bitmask;
            ss << '.';
        }
    }

    return ss.str();
}

void SudokuBoard::SetCellValue(int row, int col, int value, bool fPerm)
{
    assert(row >=0);
    assert(row < 9);
    assert(col >=0);
    assert(col < 9);
    assert(value >= 0);
    assert(value <= 9);

    Cell *cell = &m_board[row][col];
    SetCellValue(cell, value, fPerm);
}

void SudokuBoard::SetCellValue(Cell *cell, int value, bool fPerm)
{
    cell->SetValue(value);
    cell->_isPermanent = fPerm && (value != 0);

    // clear out the value bit from all the cells in the same row,col, and square
    CellSet* rowset = cell->_row;
    CellSet* square = cell->_square;
    CellSet* colset = cell->_column;

    for (int index = 0; index < 9; index++)
    {
        if (cell != rowset->_set[index])
        {
            rowset->_set[index]->ClearValueFromMask(value);
        }

        if (cell != colset->_set[index])
        {
            colset->_set[index]->ClearValueFromMask(value);
        }

        if (cell != square->_set[index])
        {
            square->_set[index]->ClearValueFromMask(value);
        }
    }

    return;

}


void SudokuBoard::ScanForSolution()
{
    // this function is the main loop that looks for a solution
    int value = 0;

    for (int r = 0; r < 9; r++)
    {
        for (int c = 0; c < 9; c++)
        {
            Cell *cell = &m_board[r][c];

            if (cell->_value != 0)  // cell is already solved
            {
                continue;
            }

            value = SimpleEliminate(cell, cell->_square);
            if (value == 0)
            {
                value = SimpleEliminate(cell, cell->_row);
            }
            if (value == 0)
            {
                value = SimpleEliminate(cell, cell->_column);
            }

            if (value != 0)
            {
                continue;
            }

            PairSearch(cell, cell->_square);
            PairSearch(cell, cell->_row);
            PairSearch(cell, cell->_column);

            TripleSearch(cell, cell->_square);
            TripleSearch(cell, cell->_row);
            TripleSearch(cell, cell->_column);

        }
    }


    for (int index = 0; index < 9; index++)
    {
        BoxLineReduction(&m_rows[index]);
        BoxLineReduction(&m_cols[index]);
    }


    for (int index = 0; index < 9; index++)
    {
        DoNumberClaiming(&m_squares[index]);
    }

    //FullDump();
    DoXWingSets(m_cols);
    DoXWingSets(m_rows);
}

// SimpleEliminate looks at the non-eliminated values at "cell" and compares it to all the
// other non-eliminated values from the {square,row,column} set.  If a non-eliminated value appears only once,
// then it
int SudokuBoard::SimpleEliminate(Cell *cell, CellSet *set)
{
    uint16_t wOtherMask = 0;
    int value = 0;

    if (cell->_value != 0)
    {
        return 0;
    }

    if (Cell::BitCount(cell->_bitmask) == 1)
    {
        // all other values have been eliminated for this cell, it must be "the one"
        value = Cell::GetCellValueFromBitmask(cell->_bitmask);
        SetCellValue(cell->_rowIndex, cell->_colIndex, value);
        LogWithoutLineBreak("SimpleEliminate - Single Bit match.  Setting %d for (r=%d c=%d)\n", value, cell->_rowIndex, cell->_colIndex);
        return value;
    }


    for (int index = 0; index < 9; index++)
    {
        Cell *othercell = set->_set[index];

        if (othercell != cell)
        {
            wOtherMask |= othercell->_bitmask;
        }
    }

    // subtract wOtherMask from cell->m_bitmask
    // if the result is something that has only 1 bit set, then it follows that there is exactly one value for that cell

    uint16_t wResult = (cell->_bitmask & ~wOtherMask);

    if (Cell::BitCount(wResult) == 1)
    {
        value = Cell::GetCellValueFromBitmask(wResult);

        SetCellValue(cell, value);

        CELL_RELATIONSHIP relate = cell->GetRelationship(set);

        const char *psz = g_relationship_name[relate];

        Log("SimpleEliminate - setting value of %d at (r=%d c=%d) [%s elimination]", value, cell->_rowIndex, cell->_colIndex, psz);
    }

    return value;
}

int SudokuBoard::BoxLineReduction(CellSet *set)
{

    bool placed[10] = {0};
    Cell *cell = NULL;
    Cell *comparecell = NULL;
    bool fCanDoBoxLineReduction;
    int reducecount = 0;
    
    for (int cellindex = 0; cellindex < 9; cellindex++)
    {
        int value = set->_set[cellindex]->_value;
        placed[value] = true;  // value[0] is irrelevant
    }

    for (int valueindex = 1; valueindex <= 9; valueindex++)
    {
        fCanDoBoxLineReduction = true;
        comparecell = NULL;

        if (placed[valueindex] == true)
        {
            continue;
        }

        // look at all the cells in this set that have index in the candidate list.   If all the cells of this set appear
        // in the same box, we can do boxline reduction

        for (uint16_t cellindex = 0; cellindex < 9; cellindex++)
        {
            cell = set->_set[cellindex];

            if (cell->_value != 0)
            {
                continue;
            }

            if (cell->IsOkToSetValue(valueindex))
            {
                // we found a cell that contains a missing value for this set it it's candidate list
                if (comparecell == NULL)
                {
                    comparecell = cell;
                }
                else
                {
                    if (comparecell->_square != cell->_square)
                    {
                        // "index" appears in m_bitmask in cells across different boxes - time to skip
                        fCanDoBoxLineReduction = false;
                        break;
                    }
                }
            }
        }

        cell = NULL;

        if (comparecell && fCanDoBoxLineReduction)
        {
            CellSet *square = comparecell->_square;

            // iterate over all the cells in the square that don't belong to "set" (row/column)
            for (uint16_t cellindex = 0; cellindex < 9; cellindex++)
            {
                cell = square->_set[cellindex];
                if  ((cell->_row == set) || (cell->_column == set))
                {
                    continue;
                }

                assert(cell != comparecell);

                assert(cell->_value != valueindex);

                // another cell in the same square that doesn't belong to "set", remove valueindex from the bitmask
                if (cell->ClearValueFromMask(valueindex))
                {
                    Log("BoxLineReduced %d from cell(r=%d c=%d)", valueindex, cell->_rowIndex, cell->_colIndex);
                    reducecount++;
                }
            }
        }
    }

    return reducecount;
}

int SudokuBoard::ClaimNumbers(uint16_t mask, CellSet *square, CellSet *set)
{
    // Remove all the values of "mask" from "set" that are not in "square"

    int value;
    int count = 0;
    
    value = Cell::GetCellValueFromBitmaskAndClear(mask);
    while (value != 0)
    {
        for (int index = 0; index < 9; index++)
        {
            Cell *cell = set->_set[index];
            if ((cell->_square != square) && (cell->_value == 0))
            {
                if (cell->IsOkToSetValue(value))
                {
                    Log("Number Claiming - removing %d from candidate list of cell at (r=%d c=%d)", value, cell->_rowIndex, cell->_colIndex);
                }

                cell->ClearValueFromMask(value);
                count++;
            }
        }

        value = Cell::GetCellValueFromBitmaskAndClear(mask);
    }

    return count;
}

int SudokuBoard::DoNumberClaiming(CellSet *square)
{
    int count  = 0;
    uint16_t maskPerRow[3] ={0};
    uint16_t maskPerCol[3] = {0};
    uint16_t wMaskRow;
    uint16_t wMaskCol;

    for (int r = 0; r < 3; r++)
    {
        wMaskRow = 0;
        for (int c = 0; c < 3; c++)
        {
            Cell *cell = square->_set[r*3+c];
            if (cell->_value == 0)
            {
                wMaskRow |= cell->_bitmask;
            }
        }
        maskPerRow[r] = wMaskRow;
    }

    for (int c = 0; c < 3; c++)
    {
        wMaskCol = 0;
        for (int r = 0; r < 3; r++)
        {
            Cell *cell = square->_set[r*3+c];
            if (cell->_value == 0)
            {
                wMaskCol |= cell->_bitmask;
            }
        }
        maskPerCol[c] = wMaskCol;
    }

    // for each row/col, look to see if there are any bits in the mask that are not in the other two row/cols

    for (int r = 0; r < 3; r++)
    {
        uint16_t wMaskedOut;
        int rx[3][3] = {{0,1,2}, {1,0,2}, {2,0,1}};
        wMaskedOut = (maskPerRow[rx[r][0]] & ~(maskPerRow[rx[r][1]] | maskPerRow[rx[r][2]]));

        if (wMaskedOut != 0)
        {
            Cell *refcell = square->_set[r*3];
            CellSet *row = refcell->_row;
            count += ClaimNumbers(wMaskedOut, square, row);
        }
    }

    for (int c = 0; c < 3; c++)
    {
        uint16_t wMaskedOut;
        int cx[3][3] = {{0,1,2}, {1,0,2}, {2,0,1}};
        wMaskedOut = (maskPerCol[cx[c][0]] & ~(maskPerCol[cx[c][1]] | maskPerCol[cx[c][2]]));

        if (wMaskedOut != 0)
        {
            Cell *refcell = square->_set[c];
            CellSet *col = refcell->_column;
            count += ClaimNumbers(wMaskedOut, square, col);
        }
    }

    return count;
}

int SudokuBoard::PairSearch(Cell *cell, CellSet *set)
{
    // scan the entire set.  If a pair of cells are found whereby both have the same bitmask of two
    // candidate values, then those candidate values can be erased from the rest of the cells in the set

    Cell *matchcell = NULL;

    if (cell->_value != 0)
        return 0;

    if (Cell::BitCount(cell->_bitmask) != 2)
    {
        return 0;
    }

    // scan the set looking for another cell with the same bitmask
    for (int cellindex = 0; cellindex < 9; cellindex++)
    {
        Cell *othercell = set->_set[cellindex];

        if (othercell == cell)
        {
            continue;
        }

        if (othercell->_value != 0)
        {
            continue;
        }

        if (othercell->_bitmask == cell->_bitmask)
        {
            matchcell = othercell;
            break;
        }
    }

    if (matchcell == NULL)
        return 0;

    uint16_t wBitmask = matchcell->_bitmask;

    int values[2];
    values[0] = Cell::GetCellValueFromBitmaskAndClear(wBitmask);
    values[1] = Cell::GetCellValueFromBitmaskAndClear(wBitmask);

    assert(values[0] != 0);
    assert(values[1] != 0);
    assert(wBitmask == 0);

    // remove the bitmask from all other cells in the set
    for (int cellindex = 0; cellindex < 9; cellindex++)
    {
        Cell *othercell = set->_set[cellindex];

        if ((othercell == matchcell) || (othercell == cell))
            continue;

        for (int x = 0; x < 2; x++)
        {
            if  (othercell->IsOkToSetValue(values[x]))
            {
                Log("PairSearch - %d removed from cell at (r=%d c=%d)", values[x], othercell->_rowIndex, othercell->_colIndex);
                othercell->ClearValueFromMask(values[x]);
            }
        }
    }

    return 0;
}

int SudokuBoard::TripleSearch(Cell *cell, CellSet *set)
{
    uint16_t wUnion;
    int value;

    if(cell->_value != 0)
    {
        return 0;
    }

    int bitcount = Cell::BitCount(cell->_bitmask);

    if ((bitcount < 2) || (bitcount > 3))
    {
        return 0;                
    }

    // look for two other cells that have an intersection in bitmasks with the cell passed in
    Cell *matches[2] = {0};
    int matchindex = 0;

    for (int index = 0; index < 9; index++)
    {
        Cell *othercell = set->_set[index];
        if (othercell == cell)
            continue;

        if (othercell->_value != 0)
            continue;

        if ((othercell->_bitmask & cell->_bitmask) == 0)
        {
            // no intersection
            continue;
        }

        wUnion = othercell->_bitmask | cell->_bitmask;

        if (Cell::BitCount(wUnion) > 3)
        {
            continue;
        }

        matches[matchindex] = othercell;
        matchindex++;
        if (matchindex == 2)
            break;
    }

    if (!(matches[0] && matches[1]))
    {
        return 0;
    }

    wUnion = matches[0]->_bitmask | matches[1]->_bitmask | cell->_bitmask;

    // something would be "off" if we had three cells only having a union of two or less bits
    // If the union bitmask is 4 or more, then my assertion that the loop above shouldn't do that is wrong
    //assert(Cell::BitCount(wUnion) == 3);

    if (Cell::BitCount(wUnion) != 3)
        return 0;

    // we have our three cells, let's pull these bits out of the other cells that aren't set

    for (int index = 0; index < 9; index++)
    {
        Cell *othercell = set->_set[index];
        if (othercell == cell)
            continue;

        if (othercell == matches[0])
            continue;

        if (othercell == matches[1])
            continue;

        if (othercell->_value != 0)
            continue;

        if (othercell->_bitmask & wUnion)
        {
            uint16_t wMask = wUnion;

            while (wMask)
            {
                value = Cell::GetCellValueFromBitmaskAndClear(wMask);
                if (othercell->IsOkToSetValue(value))
                {
                    othercell->ClearValueFromMask(value);
                    Log("TripleSearch - %d removed from cell at (r=%d c=%d)", value, othercell->_rowIndex, othercell->_colIndex);
                }
            }
        }
    }

    return 0;
}


int SudokuBoard::DoXWingSets(CellSet *sets)
{
    // look at every row where there are exactly two candidate cells for a particular value
    // If there is another row exactly two candidate cells for the same value, then it can be removed from the columns in the other rows

    int value = 0;
    uint16_t wBitMask = 0;
    int valuecounts[10][10] = {0}; // [row][value]
    int col1, col2;
    int changecount = 0;
    bool fRet;

    for  (int rowindex = 0; rowindex < 9; rowindex++)
    {
        CellSet *row = &sets[rowindex];

        for (int cellindex = 0; cellindex < 9; cellindex++)
        {
            Cell *cell = row->_set[cellindex];
            if (cell->_value != 0)
                continue;

            wBitMask = cell->_bitmask;

            while (wBitMask)
            {
                value = Cell::GetCellValueFromBitmaskAndClear(wBitMask);
                valuecounts[rowindex][value] = valuecounts[rowindex][value] + 1;
            }
        }
    }

    // for each row, look for instances where there are only two possible cells for a given instance

    for (int rowindex = 0; rowindex < 9; rowindex++)
    {
        for (int valueindex = 1; valueindex <= 9; valueindex++)
        {
            if (valuecounts[rowindex][valueindex] == 2)
            {

                fRet = XWing_FindColumnIndices(&sets[rowindex], valueindex, col1, col2);
                if (fRet == false)
                    continue;
              

                // find the matching row
                CellSet *matchrow = NULL;
                for (int nextrowindex = rowindex+1; nextrowindex < 9; nextrowindex++)
                {
                    if (valuecounts[nextrowindex][valueindex] == 2)
                    {
                        matchrow = &sets[nextrowindex];

                        Cell *c1 = matchrow->_set[col1];
                        Cell *c2 = matchrow->_set[col2];

                        if (!(c1->IsOkToSetValue(valueindex) && c2->IsOkToSetValue(valueindex)))
                        {
                            matchrow = NULL;
                        }
                        else
                        {
                            break;
                        }
                    }
                }


                if (matchrow != NULL)
                {
                    changecount = XWing_DoFilter(sets, &sets[rowindex], matchrow, valueindex, col1, col2);
                }
            }
        }
    }

    return changecount;
}

bool SudokuBoard::XWing_FindColumnIndices(CellSet *row, int value, int &col1, int &col2)
{
    col1 = -1;
    col2 = -1;

    for (int cellindex = 0; cellindex < 9; cellindex++)
    {
        Cell *cell = row->_set[cellindex];

        if (cell->IsOkToSetValue(value))
        {
            if (col1 == -1)
                col1 = cellindex;
            else if (col2 == -1)
                col2 = cellindex;
            else
                assert(false);  // a third instance found in this row???
        }
    }

    if ((col1 == -1) || (col2 == -1))
    {
        // this can happen if a previous loop in DoXWingSets removed one of the values
        return false;
    }
    return true;
}

int SudokuBoard::XWing_DoFilter(CellSet *sets, CellSet *firstrow, CellSet *matchrow, int value, int col1, int col2)
{
    int changecount = 0;

    bool fUsingColumns = (firstrow->_set[0]->_rowIndex == matchrow->_set[0]->_rowIndex);

    for (int rowindex = 0; rowindex < 9; rowindex++)
    {
        CellSet  *row = &sets[rowindex];

        if ((row == firstrow) || (row == matchrow))
        {
            continue;
        }

        // now look at col1 and col2 to see if value appears in the candidate list for these cells

        Cell *cells[2];
        
        cells[0] = row->_set[col1];
        cells[1] = row->_set[col2];

        for (int x = 0; x < 2; x++)
        {
            if (cells[x]->IsOkToSetValue(value))
            {
                Log("XWing - removing %d from cell at (r=%d c=%d)", value, cells[x]->_rowIndex, cells[x]->_colIndex);

                if (fUsingColumns == false)
                {
                    Log("   XWing cells are at (r=%d c=%d) (r=%d c=%d) (r=%d c=%d) (r=%d c=%d)", firstrow->_set[0]->_rowIndex, col1, firstrow->_set[0]->_rowIndex, col2, matchrow->_set[0]->_rowIndex, col1, matchrow->_set[0]->_rowIndex, col2);
                }
                else
                {
                    Log("   XWing cells are at (r=%d c=%d) (r=%d c=%d) (r=%d c=%d) (r=%d c=%d)", col1, firstrow->_set[0]->_colIndex, col1, matchrow->_set[0]->_colIndex, col2, firstrow->_set[0]->_colIndex, col2, matchrow->_set[0]->_colIndex);
                }


                cells[x]->ClearValueFromMask(value);
                changecount++;
            }
        }
    }

    return changecount;
}






void SudokuBoard::Dump()
{
    for (int row = 0; row < 9; row++)
    {
        for (int col = 0; col < 9; col++)
        {
            LogWithoutLineBreak("%c ", m_board[row][col]._value ? (m_board[row][col]._value + '0') : '?');

            if ((col % 3 == 2) && (col != 8))
                LogWithoutLineBreak("| ");
        }
        Log("");

        if (row % 3 == 2)
        {
            Log("---------------------");
        }
           
    }
}

void SudokuBoard::FullDump()
{
    int value;
    int count;

    for (int row = 0; row < 9; row++)
    {
        for (int col = 0; col < 9; col++)
        {
            value = m_board[row][col]._value;
            if (value != 0)
            {
                LogWithoutLineBreak("%c      ", value + '0');
            }
            else
            {
                LogWithoutLineBreak("{");
                uint16_t wMask = m_board[row][col]._bitmask;
                count = 0;
                while (wMask)
                {
                    value = Cell::GetCellValueFromBitmaskAndClear(wMask);
                    LogWithoutLineBreak("%c", value + '0');
                    count++;
                }

                int remainingspaces = 5 - count;
                LogWithoutLineBreak("}");
                while (remainingspaces > 0)
                {
                    LogWithoutLineBreak(" ");
                    remainingspaces--;
                }


            }

            if ((col % 3 == 2) && (col != 8))
                LogWithoutLineBreak("| ");
        }

        Log("");
        if (row % 3 == 2)
        {
            Log("------------------------------------------------------------------");
        }

    }
}



