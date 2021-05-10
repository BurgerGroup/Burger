#ifndef SUDOKU_H
#define SUDOKU_H

#include <string>

std::string solveSudoku(const std::string& puzzle);
const int kCells = 81;
extern const char kNoSolution[];

#endif // SUDOKU_H