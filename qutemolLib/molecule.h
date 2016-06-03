#ifndef MOLECULE_H
#define MOLECULE_H

#include <vector>
#include "baseTypes.h"

namespace qmol {

class Shape;
class AtomTable;
class ChainTable;
struct ShapingOptions;
class MoleculeList;

class PdbToken;


/** qmol::Atom: */
class Atom{
public:
  char type[2]; // eg. "C" ('c', '\n') or "NA" ('n', 'a')
  Pos pos;

  bool isHetero;
  bool isHydrogen() const;
  bool isSelected;

  int chainId;
  int modelId;


  void fromPDB(const std::string &line); // reads from a line

  std::vector<int> bond; // indices of atoms this atom is connected to
};

typedef std::vector< std::string > StringVector;
typedef std::string String;

struct Date{
    int month;
    int day;
    int year;
    void fromString(const char* st);
    static int st2month(const char* st);
};

struct Publication{
    String authors;
    String title;
    String status;
    String journal;
    String editor; //* non journals
    String publisher; //* non journals
    String pubmedId;
    String doi; // like, e.g., "doi:10.1000/182"
    String issnId, essnId;

    void clear();

    bool fromPDB(const String &line);
    String toString() const;

    static Publication qutemolCiteware(); // returns reference to qutemol article!


};

class PdbMetaData{
public:
    // from header
    char idCode[5];
    Date date;
    String classification;
    String title;

    // group:
    String authors;

    // group:
    //String publication; // todo: structurize
    Publication publication;

    // group:
    String keywords;

    // group: "part of"
    String partOf;

    // group: "melecular complex info"
    String macromolecularInfo; // todo: structurize
    String sourceInfo; // todo: structurize

    // group: "about dataset"
    String modelType;
    String experimentalData;

    // group "log"
    String revisionHistory;
    String obsoleting; // this molecule is obsoleting!

    // group:
    String warnings;
    Date obsoletedSince; // when was this molecule obsolete
    String obsoletedBy; // this molecule is obsoleted by...!


    void clear();

    /* access methods */
    std::string urlToActivePdbKeeper() const;
    static std::string activePdbKeeperUrlBase;
    std::string urlToDoiKeeper() const;  // "http://search.crossref.org/?q=doi%3A10.1000%2F182"
    static std::string activeDoiUrlBase;

    /* import */
    bool fromPDB(const String &line, PdbToken t);
};

class Transformation{
public:
    Vec x,y,z,t;
    Pos apply( const Pos& p ) const;
    static Transformation identity();
    bool isIdentity() const;
    void fromPDB(const String &line);
};

/** qmol::Molecule: a collection of atoms and bonds */
class Molecule
{
public:
  // loads a molecule... throws a std::string exception if anything goes bad
  bool loadPDB(const std::wstring &filename , FeedbackFunctionT f = NULL  ) throw (std::wstring);

  std::vector<Atom> atom;

  bool makeShape(Shape& p, const ShapingOptions &opt, const AtomTable &table, const ChainTable &chainCols);
  bool makeSingleShape(Shape& p, const ShapingOptions &opt, const AtomTable &table, const ChainTable &chainCols, int modelID);

  int nHydro,nHetero,maxChain,maxModel; // stats

  Molecule();

  bool isEmpty() const { return atom.empty(); }
  int nModels() const; // include symmetries! but does not inlcude 0th model.
  int nChains() const;
  int nSymmetries() const;
  int ithModelIndex(int i);
  bool looksLikeAnAnimation() const; /* heuristic */

  PdbMetaData metadata;
  std::vector<Transformation> symmetry;

  void clear();

private:

  /// reads bonds from a "CONECT" line of a pdb file
  void addBondsFromPDB(const String &line);
  void addSymmetryFromPDB(const String &line);

  Pos center; Scalar radius; // bounding sphere
  void updateBoundingSphere();

  void compressSymmetryVector();
  void apply( const Transformation &t);

  void fillShape(Shape& p, const ShapingOptions &opt, const AtomTable &table, const ChainTable &chainCols, int modelBias);
  void fillShape(Shape& p, const ShapingOptions &opt, const AtomTable &table, const ChainTable &chainCols, int modelBias, int modelID);

friend class MoleculeList;
};

class MoleculeList : public std::vector< Molecule > {
public:
    bool makeShape(Shape& p, const ShapingOptions &opt, const AtomTable &table, const ChainTable &chainCols);

    // void clear(); // superclass one is just fine

    int loadPDBs(const std::vector< std::wstring > &filename, FeedbackFunctionT f =  NULL ) throw (std::wstring);


    bool isEmpty() const;
    int nHetero() const;
    int nHydro() const;

    int nAtoms() const;
    int nModels() const; // include symmetries! but does not inlcude 0th model.
    int nChains() const;
    int nSymmetries() const;
    int ithModelIndex(int i);
    bool looksLikeAnAnimation() const; /* heuristic */

};

} // namespace qmol

#endif // MOLECULE_H
