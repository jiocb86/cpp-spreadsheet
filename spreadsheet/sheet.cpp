#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
#include <sstream>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }
    
    const auto& cell = cells_.find(pos);

    if (cell == cells_.end()) {
        cells_.emplace(pos, std::make_unique<Cell>(*this));
    }
    cells_.at(pos)->Set(std::move(text));
}

const CellInterface* Sheet::GetCell(Position pos) const {
    return GetCellPtr(pos);
}

CellInterface* Sheet::GetCell(Position pos) {
    return GetCellPtr(pos);
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }

    const auto& cell = cells_.find(pos);
    if (cell != cells_.end() && cell->second != nullptr) {
        cell->second->Clear();
        if (!cell->second->IsReferenced()) {
            cell->second.reset();
        }
    }
}

Size Sheet::GetPrintableSize() const {
    Size result{0, 0};
    
    for (auto it = cells_.begin(); it != cells_.end(); ++it) {
        if (it->second != nullptr) {
            const int col = it->first.col;
            const int row = it->first.row;
            result.rows = std::max(result.rows, row + 1);
            result.cols = std::max(result.cols, col + 1);
        }
    }

    return { result.rows, result.cols };
}

void Sheet::Print(std::ostream& output, std::function<std::string(const Cell&)> getValue) const {
    Size printable_size = GetPrintableSize();
    
    for (int row = 0; row < printable_size.rows; ++row) {
        for (int col = 0; col < printable_size.cols; ++col) {
            Position pos = {row, col};
            if (col > 0) {
                output << "\t";  
            }
            auto it = cells_.find(pos);
            if (it != cells_.end() && it->second != nullptr) { 
                output << getValue(*it->second); 
            }
        }
        output << "\n"; 
    }
}

void Sheet::PrintValues(std::ostream& output) const {
    Print(output, [](const Cell& cell) {
        auto value = cell.GetValue();
        std::ostringstream oss;
        std::visit([&oss](auto&& arg) { oss << arg; }, value);
        return oss.str();
    });
}

void Sheet::PrintTexts(std::ostream& output) const {
    Print(output, [](const Cell& cell) {
        return cell.GetText();
    });
}

const Cell* Sheet::GetCellPtr(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }

    auto it = cells_.find(pos);
    if (it != cells_.end()) {
        return it->second.get(); 
    }
    
    return nullptr;
}

Cell* Sheet::GetCellPtr(Position pos) {
    return const_cast<Cell*>(static_cast<const Sheet&>(*this).GetCellPtr(pos));
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}