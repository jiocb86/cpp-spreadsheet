#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <unordered_set>
#include <stack>

class Sheet;

class Cell : public CellInterface {
public:
    Cell(Sheet& sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    bool IsReferenced() const;

private:
    class Impl;
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;
    
    std::unique_ptr<Impl> impl_;
    
    bool CheckCyclicDependency(const Impl& new_impl) const; // Проверка цикличности
    void UpdateDependencies(bool force = false);            // Обновление зависимостей
    void UpdateReferences(std::unique_ptr<Impl> new_impl);    
    
    bool HasCycle(const std::unordered_set<const Cell*>& referenced, 
                  std::unordered_set<const Cell*>& visited, 
                  std::stack<const Cell*>& to_visit) const;    
    
    Sheet& sheet_;                                          // Ссылка на таблицу
    std::unordered_set<Cell*> dependent_сells_;             // Зависимые ячейки
    std::unordered_set<Cell*> referenced_сells_;            // Связанные ячейки
};