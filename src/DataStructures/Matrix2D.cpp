// ============================================================================
//  Matrix2D.cpp
// ============================================================================
#include "Matrix2D.h"
#include <iostream>
#include <iomanip>

void Matrix2D::allocate(int r, int c, char fillValue)
{
    if (r <= 0 || c <= 0)
    {
        data = nullptr;
        rows = 0;
        cols = 0;
        return;
    }
    rows = r;
    cols = c;
    data = new char*[rows];                        // mang cac con tro hang
    for (int i = 0; i < rows; i++)
    {
        data[i] = new char[cols];                  // moi hang la mang char
        for (int j = 0; j < cols; j++) data[i][j] = fillValue;
    }
}

void Matrix2D::release()
{
    if (data != nullptr)
    {
        for (int i = 0; i < rows; i++) delete[] data[i];   // giai phong tung hang
        delete[] data;                                     // roi giai phong mang con tro
    }
    data = nullptr;
    rows = 0;
    cols = 0;
}

Matrix2D::Matrix2D(int r, int c, char fillValue) : data(nullptr), rows(0), cols(0)
{
    allocate(r, c, fillValue);
}

Matrix2D::Matrix2D(const Matrix2D& other) : data(nullptr), rows(0), cols(0)
{
    allocate(other.rows, other.cols, ' ');
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            data[i][j] = other.data[i][j];
}

Matrix2D& Matrix2D::operator=(const Matrix2D& other)
{
    if (this != &other)
    {
        release();
        allocate(other.rows, other.cols, ' ');
        for (int i = 0; i < rows; i++)
            for (int j = 0; j < cols; j++)
                data[i][j] = other.data[i][j];
    }
    return *this;
}

Matrix2D::~Matrix2D()
{
    release();
}

int Matrix2D::getRows() const { return rows; }

int Matrix2D::getCols() const { return cols; }

bool Matrix2D::isValidPosition(int r, int c) const
{
    return r >= 0 && r < rows && c >= 0 && c < cols;
}

char Matrix2D::get(int r, int c) const
{
    if (!isValidPosition(r, c)) return '\0';
    return data[r][c];
}

bool Matrix2D::set(int r, int c, char value)
{
    if (!isValidPosition(r, c)) return false;
    data[r][c] = value;
    return true;
}

void Matrix2D::fill(char value)
{
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            data[i][j] = value;
}

int Matrix2D::count(char value) const
{
    int total = 0;
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            if (data[i][j] == value) total++;
    return total;
}

void Matrix2D::display() const
{
    if (rows == 0)
    {
        std::cout << "(ma tran rong)\n";
        return;
    }
    std::cout << std::right;
    std::cout << "    ";
    for (int j = 0; j < cols; j++) std::cout << std::setw(3) << (j + 1);
    std::cout << "\n";
    for (int i = 0; i < rows; i++)
    {
        std::cout << "  " << (char)('A' + i) << " ";
        for (int j = 0; j < cols; j++) std::cout << std::setw(3) << data[i][j];
        std::cout << "\n";
    }
}
