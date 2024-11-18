#pragma once

#include "cell.h"
#include "common.h"

#include <functional>

class Sheet : public SheetInterface {
public:
    struct Hasher {
        size_t operator()(const Position& pos) const {
            return std::hash<int>()(pos.row) + 37 * std::hash<int>()(pos.col);
        }
    };

    struct Comparator {
        bool operator()(const Position& lhs, const Position& rhs) const {
            return lhs == rhs;
        }
    };
    
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;
    
    void Print(std::ostream&, std::function<std::string(const Cell&)>) const;
    
    const Cell* GetCellPtr(Position pos) const;
    Cell* GetCellPtr(Position pos);    

private:
    std::unordered_map<Position, std::unique_ptr<Cell>, Hasher, Comparator> cells_;  
};