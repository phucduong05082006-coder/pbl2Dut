// ============================================================================
//  Matrix2D.h - Ma tran ky tu 2 chieu cap phat dong (char**)
//  Dung cho so do ghe: 'O' = ghe trong, 'X' = ghe da dat
//  Cai dat: Matrix2D.cpp (khong phai template nen bien dich rieng duoc)
// ============================================================================
#ifndef MATRIX2D_H
#define MATRIX2D_H

class Matrix2D
{
private:
    char** data;
    int rows;
    int cols;

    void allocate(int r, int c, char fillValue);   // new char*[r], moi hang new char[c]
    void release();                                // delete[] tung hang, roi delete[] data

public:
    Matrix2D(int r = 0, int c = 0, char fillValue = ' ');
    Matrix2D(const Matrix2D& other);               // Rule of Three
    Matrix2D& operator=(const Matrix2D& other);
    ~Matrix2D();

    int getRows() const;
    int getCols() const;

    bool isValidPosition(int r, int c) const;
    char get(int r, int c) const;                  // '\0' neu vi tri sai
    bool set(int r, int c, char value);            // false neu vi tri sai
    void fill(char value);
    int count(char value) const;                   // dem so o co gia tri = value

    // In ma tran voi tieu de cot 1..cols va nhan hang A, B, C...
    void display() const;
};

#endif
