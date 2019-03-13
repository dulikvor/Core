#include "Allocator.h"
#include <algorithm>
#include <cstdint>

namespace core{
    
    const unsigned int BuddyTree::NON = std::numeric_limits<unsigned int>::max();
    
    unsigned int BuddyCell::CellsUpUntilCurrentLevel(unsigned int cellLevel)
    {
        return std::max(static_cast<unsigned int>(pow(2, cellLevel) - 1), static_cast<unsigned int>(0));//Geometric progression
    }
    
    unsigned int BuddyCell::CellsPerLevel(unsigned int cellLevel)
    {
        return static_cast<unsigned int>(pow(2, cellLevel));
    }
    
    char* BuddyCell::CalculateCellAddress(char *base, unsigned int cellLevel, unsigned int logarithmVal, unsigned int cell)
    {
		auto cellSize = static_cast<std::size_t>(pow(2, logarithmVal));
        unsigned int cellOffset = cell - CellsUpUntilCurrentLevel(cellLevel);
        return base + cellOffset * cellSize;
    }
    
    unsigned int BuddyCell::AddressToCellIdx(unsigned int cellLevel, unsigned int logarithmVal, char* base, char *address)
    {
		auto cellSize = static_cast<std::size_t>(pow(2, logarithmVal));
        std::uintptr_t addressDiff = reinterpret_cast<std::uintptr_t>(address) - reinterpret_cast<std::uintptr_t>(base);
        unsigned int cellIdx = addressDiff % cellSize == 0 ? addressDiff / cellSize + BuddyCell::CellsUpUntilCurrentLevel(cellLevel) :
                std::numeric_limits<unsigned int>::max();
        return cellIdx;
    }
    
    unsigned int BuddyCell::CalculateCellToLevel(unsigned int cell)
    {
		return static_cast<unsigned int>(floor(log2(cell + 1)));
    }
    
    BuddyTree::BuddyTree(unsigned int numCellsLevel, char* const buffer)
        :m_topCellLevel(numCellsLevel), m_buffer(buffer)
        {
            unsigned int cellsCount = BuddyCell::CellsUpUntilCurrentLevel(m_topCellLevel) + static_cast<unsigned int>(pow(2, m_topCellLevel));
            VERIFY(cellsCount < std::numeric_limits<unsigned int>::max(), "unsigned maximum value is being used as a flag, too many cells are requested");
            m_buddyTree.reset(new Symbolset<BLOCK_STATUS_BIT_COUNT>(cellsCount, NILL));
            (*m_buddyTree)[0] = NotAllocated;
            m_availableCells.insert(std::make_pair(0, 1));
        }
        
    char* BuddyTree::Allocate(unsigned int logarithmVal)
    {
        unsigned int cellLevel = m_topCellLevel - logarithmVal, targetCellLevel = m_topCellLevel - logarithmVal;
        unsigned int cell = NON;
        while (cellLevel >= 0) {
            if (m_availableCells.find(cellLevel) != m_availableCells.end())//Cells are counted in inverse
            {
                cell = ParitionCells(cellLevel, targetCellLevel);
                break;
            }
            cellLevel--;
        }
        if (cell == NON)
            throw std::bad_alloc();

        (*m_buddyTree)[cell] = Allocated;
        DecreaseCellLevel(targetCellLevel, 1);
        return BuddyCell::CalculateCellAddress(m_buffer, targetCellLevel, logarithmVal, cell);
    }
    
    void BuddyTree::Deallocate(char *address)
    {
        for(unsigned int cellLevelIdx = 0; cellLevelIdx <= m_topCellLevel; cellLevelIdx++)
        {
            unsigned int cell = BuddyCell::AddressToCellIdx(cellLevelIdx, m_topCellLevel - cellLevelIdx, m_buffer, address);
            if(cell != NON &&
               BuddyCell::CalculateCellAddress(m_buffer, cellLevelIdx, m_topCellLevel - cellLevelIdx, cell) == address &&
               static_cast<int>((*m_buddyTree)[cell]) == Allocated)
            {
                (*m_buddyTree)[cell] = NotAllocated;
                IncreaseCellLevel(cellLevelIdx, 1);
                MergeCells(cell);
                return;
            }
        }
        throw std::bad_alloc();
    }
    
    void BuddyTree::MergeCells(unsigned int startingCell)
    {
        unsigned int cell = startingCell;
        while(cell != 0)
        {
            bool isLeftChild = cell % 2 != 0;
            unsigned int siblingCell = isLeftChild ? cell + 1 : cell - 1;
            if(static_cast<int>((*m_buddyTree)[siblingCell]) == NotAllocated && static_cast<int>((*m_buddyTree)[cell]) == NotAllocated)
            {
                (*m_buddyTree)[siblingCell] = NILL;
                (*m_buddyTree)[cell] = NILL;
                cell = isLeftChild ? cell / 2 : cell / 2 - 1; //Calculating parent cell
                (*m_buddyTree)[cell] = NotAllocated; //Updating parent cell
                
                unsigned int cellLevel = BuddyCell::CalculateCellToLevel(cell);
                DecreaseCellLevel(cellLevel + 1, 2);
                IncreaseCellLevel(cellLevel, 1);
            }
            else
                return;
        }
    }
    
    unsigned int BuddyTree::ParitionCells(unsigned int startingCellLevel, unsigned int requestedCellLevel)
    {
        std::list<unsigned int> candidateCells = {0}; //Always start with the largest chunk
        bool mayPartition = false;
        while(candidateCells.empty() == false)
        {
            unsigned int currentCell = *candidateCells.begin();
            candidateCells.pop_front();
            unsigned int currentCellLevel = BuddyCell::CalculateCellToLevel(currentCell);
            if(currentCellLevel == requestedCellLevel &&
                    static_cast<int>((*m_buddyTree)[currentCell]) == NotAllocated)
                return currentCell;
            
            if(currentCellLevel >= startingCellLevel)
                mayPartition = true;
            else
                mayPartition = false;
            
             if(currentCellLevel > requestedCellLevel)
                 continue;
            
            if(static_cast<int>((*m_buddyTree)[currentCell]) == NotAllocated && mayPartition)
            {
                unsigned int leftChildCell = currentCell * 2 + 1, rightChildCell = currentCell * 2 + 2;
                (*m_buddyTree)[leftChildCell] = NotAllocated;
                (*m_buddyTree)[rightChildCell] = NotAllocated;
                (*m_buddyTree)[currentCell] = NILL;
                candidateCells.push_front(leftChildCell);
                candidateCells.push_front(rightChildCell);
                
                DecreaseCellLevel(currentCellLevel, 1);
                IncreaseCellLevel(currentCellLevel + 1, 2);
            }
            else if(static_cast<int>((*m_buddyTree)[currentCell]) == NILL)
            {
                unsigned int leftChildCell = currentCell * 2 + 1, rightChildCell = currentCell * 2 + 2;
                candidateCells.push_front(leftChildCell);
                candidateCells.push_front(rightChildCell);
            }
        }
        return NON;
    }
    
    void BuddyTree::DecreaseCellLevel(unsigned int cellLevel, int count)
    {
        m_availableCells[cellLevel] -= count;
        if(m_availableCells[cellLevel] == 0)
            m_availableCells.erase(cellLevel);
    }
    
    void BuddyTree::IncreaseCellLevel(unsigned int cellLevel, int count)
    {
        if(m_availableCells.find(cellLevel) == m_availableCells.end())
            m_availableCells.insert(std::make_pair(cellLevel, count));
        else
            m_availableCells[cellLevel] += count;
    }
}
