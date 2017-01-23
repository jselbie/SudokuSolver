#include "stdafx.h"
#include "cell.h"

// Must match up to CELL_RELATIONSHIP
const char *g_relationship_name[] = {
    "None",
    "Row",
    "Column",
    "Square"
};

CellSet::CellSet()
{
    Reset();
}

void CellSet::Reset()
{
    _set.clear();
    _set.resize(9);
}


Cell::Cell()
{
    Reset();
};

void Cell::Reset()
{
    _value = 0;
    _bitmask = CELLINIT;
    _square = nullptr;
    _row = nullptr;
    _column = nullptr;
    _rowIndex = -1;
    _colIndex = -1;
    _squareIndex = -1;
    _squareCellIndex = -1;
    _isPermanent = false;
    _cellIndex = 0;
}

void Cell::SetValue(int value)
{
    if (value <= 0)
    {
        if (_value > 0)
        {
            _bitmask = CELLINIT;
        }
        // don't clear bitmask if value is already 0
    }
    else
    {
        _value = value;
        _bitmask = 0x01 << (value - 1);
    }
}

bool Cell::IsOkToSetValue(int value)
{
    assert(value > 0);
    assert(value <= 9);
    uint16_t mask = 0x01 << (value-1);
    return ((mask & _bitmask) != 0);
}

bool Cell::ClearValueFromMask(int value)
{
    assert(value >= 0);
    assert(value <= 9);

    if (value <= 0)
        return false;

    uint16_t mask = 0x01 << (value - 1);
    mask = ~mask;  // invert the bits

    uint16_t newmask = _bitmask & mask;

    if (newmask == _bitmask)
    {
        return false;
    }

    _bitmask = newmask;
    return true;
}

bool Cell::ClearBitmaskFromMask(uint16_t mask)
{
    assert(mask != 0);

    assert((mask & ~CELLINIT) == 0);

    if (mask == 0)
        return false;

    mask = mask & CELLINIT;

    uint16_t newMask = _bitmask & ~mask;

    if (newMask == _bitmask)
    {
        return false;
    }

    _bitmask = newMask;

    return true;
}


//static
int Cell::BitCount(uint16_t w)
{
/*
    0000 -> 0
    0001 -> 1
    0010 -> 1
    0011 -> 2

    0100 -> 1
    0101 -> 2
    0110 -> 2
    0111 -> 3

    1000 -> 1
    1001 -> 2
    1010 -> 2
    1011 -> 3

    1100 -> 2
    1101 -> 3
    1110 -> 3
    1111 -> 4 
*/

    int bitcount[16] = {0,1,1,2, 1,2,2,3, 1,2,2,3, 2,3,3,4};
    int count = 0;

    for (int index = 0; index < 4; index++)
    {
        uint16_t mask = w & 0x000f;
        count += bitcount[mask];
        w = w >> 4;
    }

    return count;
}


//static
int Cell::GetCellValueFromBitmask(uint16_t mask)
{
    int value = 1;

    if (BitCount(mask) != 1)
    {
        assert(false);
        return 0;
    }

    while (mask)
    {
        if (mask & 0x01)
        {
            return value;
        }
        value++;
        mask = mask >> 0x01;
    }
    return value;
}


//static
int Cell::GetCellValueFromBitmaskAndClear(uint16_t &mask)
{
    unsigned int value = 0;

    if (mask == 0)
        return 0;

    while (mask)
    {
        value++;

        if (mask & (0x01 << (value-1)))
        {
            mask = mask & ~(0x01 << (value-1));
            return value;
        }
    }

    return 0;
}

CELL_RELATIONSHIP Cell::GetRelationship(CellSet *cellset)
{
    if (_square == cellset)
        return CELL_SQUARE;

    if (_row == cellset)
        return CELL_ROW;

    if (_column == cellset)
        return CELL_COLUMN;

    return CELL_NONE;
};
