#include "analysis/spreadsheet/Table.h"

#include <cassert>

namespace tester {

void MapTable::addCell(const std::string &name, tester::CellPtr cell) {
  // Sanity checks.
  assert(cells.size() == 2 && "More than two rows in map table.");
  assert(colByName.count(name) == 0 && "Adding col that exists already in map table.");

  // Add this column to the name register.
  colByName.emplace(name, cells[0].size());

  // Add the name and the count.
  cells[0].emplace_back(new StringCell(name));
  cells[1].emplace_back(std::move(cell));
}

void CrossTable::reserve(const std::vector<std::string> &students) {
  // Sanity check.
  assert(!isReserved && "Reserving a cross table twice.");

  // Reserve the space. Adding one for the name column on top.
  size_t dim = students.size();
  cells.reserve(dim + 1);

  // Fill the space. Adding one for the name column on the left.
  for (size_t i = 0; i < dim + 1; ++i)
    cells.emplace_back(dim + 1);

  // Now fill the slots with names while building student to index map.
  for (size_t i = 0; i < students.size(); ++i) {
    // Get the index into the table, it's the same for row/col. One extra to offset the extra name
    // row/col.
    size_t idx = i + 1;

    // Save the name and index in the map.
    const std::string &name = students[i];
    idxByName.emplace(name, idx);

    // Put the name in the defender row and attacker column. Using reset to replace what's there
    // already.
    cells[0][idx].reset(new StringCell(name));
    cells[idx][0].reset(new StringCell(name));
  }

  // Put a string into the top left corner.
  cells[0][0].reset(new StringCell("defender\\attacker"));

  // Mark as already reserved.
  isReserved = true;
}

const Cell &CrossTable::getCrossCell(const std::string &defender, const std::string &attacker) {
  // Sanity check.
  assert(isReserved && "Getting cell from unreserved cross table.");
  CellPtr &cell = cells[idxByName[defender]][idxByName[attacker]];
  assert(cell && "Accessing cross cell that isn't yet filled.");

  // Return the cell.
  return *cell;
}

void CrossTable::addCrossCell(const std::string &defender, const std::string &attacker,
                              tester::CellPtr toAdd) {
  // Sanity checks.
  assert(isReserved && "Adding cell to unreserved cross table.");
  CellPtr &cell = cells[idxByName[defender]][idxByName[attacker]];
  assert(!cell && "Filling cross table cell that is already filled.");

  // The actual add.
  cell = std::move(toAdd);
};

void TestCountTable::addTestCount(std::string name, size_t count) {
  addCell(name, CellPtr(new IntCell<size_t>(count)));
}

void OffensivePointsTable::addAttacker(std::string name, const tester::Cell &pointRangeMin,
                                       const tester::Cell &pointRangeMax,
                                       const tester::Cell &nameRangeMin,
                                       const tester::Cell &nameRangeMax) {
  // Create the cell and add conditions to it.
  CountIfsCell *cell = new CountIfsCell();
  cell->addCondition(pointRangeMin, pointRangeMax, "\"1\"");
  cell->addCondition(nameRangeMin, nameRangeMax, "<>" + name);

  addCell(name, CellPtr(cell));
};

void TestPassRateTable::addPassRate(const std::string &defender, const std::string &attacker,
                                    size_t passCount, const tester::Cell &maxCount) {
  addCrossCell(defender, attacker, CellPtr(new RateCell(passCount, maxCount)));
}

void TestSummaryTable::addSummary(const std::string &defender, const std::string &attacker,
                                  const std::vector<tester::CellRef> &addends) {
  addCrossCell(defender, attacker, CellPtr(new AverageCell(addends)));
}

} // End namespace tester