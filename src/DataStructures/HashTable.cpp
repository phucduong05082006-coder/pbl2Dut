// ============================================================================
//  HashTable.cpp - phan KHONG phai template cua bang bam: ham bam da thuc
// ============================================================================
#include "HashTable.h"

// Ham bam da thuc (Polynomial Hash) - quy tac Horner
// Vi du "MV1": h = ((0*31 + 'M')*31 + 'V')*31 + '1'
// unsigned int tu dong "quay vong" khi tran so (modulo 2^32) nen khong loi.
unsigned int polynomialHash(const char* key)
{
    unsigned int hash = 0;
    for (int i = 0; key[i] != '\0'; i++)
        hash = hash * 31u + (unsigned char)key[i];
    return hash;
}
