#ifndef ATOMTABLE_H
#define ATOMTABLE_H

#include <vector>
#include "baseTypes.h"

namespace qmol{

/** AtomType: an entry of the atom table */
class AtomType{
public:
    Scalar spacefillRadius; // van der Wall (spacefill) radius
    Scalar covalentRadius; // covalent
    Col col;  // color

    /* constructor: use 0 in parameters for default values */
    AtomType(Scalar _spacefillRadius, Scalar _covalentRadius, const char* col);
    AtomType();

    /** conversion to html-style string (e.g. "FFFFAA") to Col , and viceversa */
    static Col string2col(const char* str);
    static char* col2string(const Col &c);

    /** read fata from a CSV (comma separated value) line */
    void fromCSVline(std::string input, char* code);
};


/** AtomType: a table specifying all data.
  * Each entry is identified by two letters (or spaces), eg, " C" or "Na". */

class AtomTable
{
public:
    std::vector< AtomType > data;

    AtomType& operator [] (const char* c) {
        return data[ twoCharsToIndex(c) ];
    }

    const AtomType& operator [] (const char* c) const {
        return data[ twoCharsToIndex(c) ];
    }

    /// loads the entire table from a XML file
    bool loadCSV(std::string filename) throw (std::wstring);

    /// clears all contents
    void clearAll();

    AtomTable();
private:
    static int charToIndex(char c);
    static int twoCharsToIndex(const char * c);
    enum { MAX_CHAR = 'z'-'a' + 2 };

};

class ChainTable: public std::vector<Col>{
public:
    Col operator[](int i) const;
    void setRandom(int size);
};

} // namespace qmol

#endif // ATOMTABLE_H
