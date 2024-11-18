#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>
#include <cmath>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

namespace {
class Formula : public FormulaInterface {
public:
    explicit Formula(std::string expression)
        try : ast_(ParseFormulaAST(expression)) {}
        catch (const std::exception& e) {
            throw FormulaException(e.what());
        }

    Value Evaluate(const SheetInterface& sheet) const override {
        const std::function<double(Position)> getValue = [&](const Position& p) -> double {
            if (!p.IsValid()) {
                throw FormulaError(FormulaError::Category::Ref);
            }                

            const auto* cell = sheet.GetCell(p);
            if (!cell) {
                return 0.0;
            }

            return ExtractValueFromCell(cell);
        };

        try {
            double result = ast_.Execute(getValue);
        
            if (std::isinf(result)) {
                throw FormulaError(FormulaError::Category::Arithmetic);
            }

            return result;

        } catch (const FormulaError& e) {
            return e;
        }
    }

    std::vector<Position> GetReferencedCells() const override {
        std::forward_list<Position> cell_list = ast_.GetCells();
        std::vector<Position> cells(cell_list.begin(), cell_list.end());
        auto end_unique = std::unique(cells.begin(), cells.end());
        cells.erase(end_unique, cells.end());
        return cells;
    }
    
    std::string GetExpression() const override {
        std::ostringstream out;
        ast_.PrintFormula(out);
        return out.str();
    } 

private:
    double ExtractValueFromCell(const CellInterface* cell) const {
        if (std::holds_alternative<double>(cell->GetValue())) {
            return std::get<double>(cell->GetValue());
        }

        if (std::holds_alternative<std::string>(cell->GetValue())) {
            std::string value = std::get<std::string>(cell->GetValue());
            if (value.empty()) {
                return 0.0;
            }

            double result;
            std::istringstream in(value);
            if (!(in >> result) || !in.eof()) {
                throw FormulaError(FormulaError::Category::Value);
            }
            return result;
        }

        throw FormulaError(std::get<FormulaError>(cell->GetValue()));
    }

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}