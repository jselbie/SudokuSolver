/*
   Copyright 2011 John Selbie
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at
       http://www.apache.org/licenses/LICENSE-2.0
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#pragma once

// A CellSet is simply a set of 9 cells that are either a row, column, or square of the Sudoku Board
struct CellSet;

enum CELL_RELATIONSHIP
{
    CELL_NONE,
    CELL_ROW,
    CELL_COLUMN,
    CELL_SQUARE
};

extern const char *g_relationship_name[];

// A Cell represents an individual square on a Sukoku board.
// A Cell has a "value" (1-9) if a value has been set, otherwise 0 represents uninitilaized (m_value)
// A Cell has a candidate list of candidate values that have not been eliminated.  The candidate list is a bitmask (m_bitmask)
// A Cell has back pointers to the CellSets (row, column, square) that it belongs to
struct Cell
{
    int _value;  // the actual value of the cell, 0 means unknown
    uint16_t _bitmask; // candidate list as a bitmask.  LSB represents 1.  (next to last bit represents 2, etc...)

    CellSet *_square;  // pointer to the square this cell belongs to
    CellSet *_row;     // pointer to the row this cell belongs to
    CellSet *_column;  // pointer to the column this cell belongs to

    int _rowIndex;    // row index (0-8)  0 is top row
    int _colIndex;    // column index (0-8)  0 is far left column
    int _squareIndex; // square index (0-8)  0 is top left square.  8 is bottom right square
    int _squareCellIndex; // which index of the square this cell belongs to (0-8).  0 is top left.  8 is bottom right

    bool _isPermanent;  // means that this is one of the initial values of the puzzle

    int _cellIndex;    // overall cell index (0-80).  0 is top left cell.  80 is bottom right cell.

    Cell();
    void SetValue(int value);           // sets both value and bitmask
    void Reset(); // sets everything back to initialization values (value = 0, _bitmask = CELLINIT, etc..)
    bool IsOkToSetValue(int value);     // returns true if "value" is in this cell's candidate list
    bool ClearValueFromMask(int value); // removes a value from this cell's candidate list
    bool ClearBitmaskFromMask(uint16_t mask); // removes an entire bitmask from the the cell's candidate list

    // BitCount is a utility function that returns the number of bits set in w
    static int BitCount(uint16_t w);

    // These next two functions map a candidate list bitmask to the actual values they correspond to

    // GetCellValueFromBitmask returns an error if there is more than one bit set in wMask
    // otherwise, returns the value associated with wMask
    static int GetCellValueFromBitmask(uint16_t wMask);

    // Useful function for itereating over a bitmask with more than one bit set.
    // Returns the value associated with the least significant bit that is set and then clears it
    static int GetCellValueFromBitmaskAndClear(uint16_t &wMask);

    CELL_RELATIONSHIP GetRelationship(CellSet *set);

};

// a CellSet is a row, square, or column
struct CellSet  // set of 9 cells making up a row, column, or square
{
    std::vector<Cell*> _set;
    CellSet();
    void Reset();
};

const uint16_t CELLINIT = 0x01ff;  // 9 bits set





