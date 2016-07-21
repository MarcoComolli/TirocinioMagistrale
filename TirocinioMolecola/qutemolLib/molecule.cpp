
#include<stdio.h>

#include "molecule.h"
#include "shapingOptions.h"
#include "shape.h"
#include "atomTable.h"
#include "log.h"

#include <cctype>
//#include <boost/algorithm/stri>
#include <string>
#include <vcg/space/point3.h>

typedef unsigned int uint;



namespace qmol{


class PdbToken {
public:
    enum{
    UNKNOWN,

    ATOM,    /* data */
    AUTHOR,  /* metadata */
    CAVEAT,  /* metadata */
    CISPEP,  /* unused */
    COMPND,  /* metadata */
    CONECT,  /* data */
    CRYST,   /* unused */
    DBREF,   /* unused */
    END,     /* control */
    ENDMDL,  /* control */
    EXPDTA,  /* metadata */
    FORMUL,  /* unused */
    HEADER,  /* metadata */
    HELIX,   /* unused (alpha helix) */
    HET,     /* unused */
    HETATM,  /* data */
    HETNAM,  /* unused */
    JRNL,    /* metadata */
    KEYWDS,  /* metadata */
    LINK,    /* unused */
    MASTER,  /* UNUSED */
    MDLTYP,  /* metadata */
    MODEL,   /* control */
    MTRIX,   /* transform: TODO */
    OBSLTE,  /* metadata */
    ORIG,    /* transform: TODO */
    REVDAT,  /* metadata */
    REMARK,  /* comment:   TODO */
    SCALE,   /* transform: TODO */
    SEQADV,  /* unused */
    SEQRES,  /* unused */
    SHEET,   /* unused (beta sheet) */
    SITE,    /* unused */
    SOURCE,  /* metadata */
    SPLIT,   /* metadata */
    SPRSDE,  /* metadata */
    TITLE,   /* metadata */
    TER,     /* unused */
    TURN,    /* unused */

    __LAST
    };
    int value;

    PdbToken(int i):value(i) {}

    static PdbToken read(const String &line);

    const char* toString();
    bool doubleCheck( const String &line );
};
//

const char* PdbToken::toString()  {
    static const char* tmp[__LAST] = {
        "UNKNOWN",

        "ATOM",
        "AUTHOR",
        "CAVEAT",
        "CISPEP",
        "COMPND",
        "CONECT",
        "CRYST",
        "DBREF",
        "END",
        "ENDMDL",
        "EXPDTA",
        "FORMUL",
        "HEADER",
        "HELIX",
        "HET",
        "HETATM",
        "HETNAM",
        "JRNL",
        "KEYWDS",
        "LINK",
        "MASTER",
        "MDLTYP",
        "MODEL",
        "MTRIX",
        "OBSLTE",
        "ORIG",
        "REVDAT",
        "REMARK",
        "SCALE",
        "SEQADV",
        "SEQRES",
        "SHEET",
        "SITE",
        "SOURCE",
        "SPLIT",
        "SPRSDE",
        "TITLE",
        "TER",
        "TURN",
    };
    return tmp[ value ];
}



bool PdbToken::doubleCheck( const String & line ){
    if (value==UNKNOWN) {
        //debug("Unkonwn token for line:\n\"%s\"",line.data() );
        return false;
    } else {
        const char* name = toString();
        if (line.compare(0,String(name).length(),name)==0) {
            return true;
        }
        //debug("Non matching token \"%s\" in line:\n",name);
        //debug("\"\%s\"", line.data() );
        return false;
    }

}


PdbToken PdbToken::read( const String & line ){
    switch (line[0]) {
    case 'A': switch (line[1]) {
        case 'T': return ATOM;
        case 'U': return AUTHOR;
        default : return UNKNOWN;
        }
    case 'C': switch (line[2]) {
        case 'V': return CAVEAT;
        case 'M': return COMPND;
        case 'N': return CONECT;
        case 'Y': return CRYST;
        case 'S': return CISPEP;
        default : return UNKNOWN;
        }
    case 'D': return DBREF;
    case 'E': switch (line[3]) {
        case 'M': return ENDMDL;
        case 'D': return EXPDTA;
        case  0 :
        case '\n':
        case ' ': return END;
        default : return UNKNOWN;
        }
    case 'F': return FORMUL;
    case 'H': switch (line[3]) {
        case 'D': return HEADER;
        case 'I': return HELIX;
        case ' ': return HET;
        case 'A': return HETATM;
        case 'N': return HETNAM;
        default : return UNKNOWN;
        }
    case 'J': return JRNL;
    case 'K': return KEYWDS;
    case 'L': return LINK;
    case 'M': switch (line[1]) {
        case 'A': return MASTER;
        case 'D': return MDLTYP;
        case 'O': return MODEL;
        case 'T': return MTRIX;
        default : return UNKNOWN;
        }
    case 'O': switch (line[1]) {
        case 'B': return OBSLTE;
        case 'R': return ORIG;
        default : return UNKNOWN;
        }
    case 'R': switch (line[2]) {
        case 'V': return REVDAT;
        case 'M': return REMARK;
        default : return UNKNOWN;
        }
    case 'S': switch (line[2]) {
        case 'Q': return (line[3]=='R')?SEQRES:SEQADV;
        case 'U': return SOURCE;
        case 'E': return SHEET;
        case 'L': return SPLIT;
        case 'R': return SPRSDE;
        case 'A': return SCALE;
        case 'T': return SITE;
        default : return UNKNOWN;
        }
    case 'T': switch (line[1]) {
        case 'I': return TITLE;
        case 'E': return TER;
        case 'U': return TURN;
        default : return UNKNOWN;
    }
    default : return UNKNOWN;
    }
}

/*
struct PdbInfoField{
    const char *strPDB;
    const char *descr;
    int numStart;
    int numEnd;
};

PdbInfoField pdbJrnlField[ Molecule::N_JOURNAL_FIELDS ] = {
    {"AUTH", "Authors", 17, 18},
    {"TITL", "Title", 17, 18},
    {"EDIT", "Editor", 17, 18},
    {"REF ",  "Status", 0,0 },
    {"PUBL", "Publication", 17, 18},
    {"REFN", "Reference", 0,0},
    {"PMID", "PubMed", 0,0},
    {"DOI ",  "DOI", 0,0},
};
*/

std::string PdbMetaData::activePdbKeeperUrlBase = "http://www.rcsb.org/pdb/explore/explore.do?structureId=%s";
std::string PdbMetaData::activeDoiUrlBase;

void Publication::clear(){
    *this = Publication();
}

void PdbMetaData::clear(){
    *this = PdbMetaData();
    idCode[0] = 0;
}

// string to month
int Date::st2month(const char* st){
    switch (st[0]) {
    case 'J': return (st[1]=='A')?0 : (st[2]=='N')?5:6;
    case 'F': return 1;
    case 'M': return (st[2]=='R')?2:4;
    case 'A': return (st[2]=='R')?3:7;
    case 'S': return 8;
    case 'O': return 9;
    case 'N': return 10;
    case 'D': return 11;
    }
    //debug("Unknown month name %d",st);
    return -1;
}

void Date::fromString(const char* st){
    sscanf(st,"%d",&day);
    sscanf(st+5,"%d",&year);
    if (year<50) year+=2000; // this will be fun 30 years from now (not my fault!)
    month = st2month( st+3 );
}

/*
static String trim(const String s){
    // shouldn't Std provide native support for this basic operation?? :-(
    // trim from start
    size_t endpos = s.find_last_not_of(" \t");
    size_t startpos = 0 ; //s.find_first_not_of(" \t");
    return s.substr( startpos, endpos - startpos + 1 );
}
*/

static String myClean(const String s){
    // shouldn't Std provide native support for this basic operation?? :-(
    // trim from start
    String res;
    uint endpos = s.find_last_not_of(' ')+1;
    res.resize( endpos );
    int m = true;
    for (uint i=0;i<endpos;i++) {
        /*//debug("%d",i);
        //debug("%c",(int)s[i]);*/
        if (m) res[i] = s[i]; else res[i] = std::tolower(s[i]);
        m = !(std::isalnum( s[i] ));
    }
    res[endpos] = 0;
    return res;
}

String Publication::toString() const {
    String res;

    if (!authors.empty()) res+=authors + ": <br/>";
    if (!title.empty()) res+= String("\"") + title +"\"<br/>";
    if (!journal.empty()) res += String("<i>") + journal +"</i><br/>";
    if (!publisher.empty()) res += String("<i>") + publisher +"</i><br/>";
    if (!editor.empty()) res += String("<i>") + editor +"</i><br/>";

    return res;
}

bool Publication::fromPDB(const String &line){


    /*{"AUTH", "Authors", 17, 18},
    {"TITL", "Title", 17, 18},
    {"EDIT", "Editor", 17, 18},
    {"REF ",  "Journal", 0,0 },
    {"PUBL", "Publisher", 17, 18},
    {"REFN", "issn or essn", 0,0},
    {"PMID", "PubMed", 0,0},
    {"DOI ",  "DOI", 0,0},*/

    std::string t = line.substr(12,3);
    switch(line[12]) {
    case 'T': // TITL
        if (!title.empty()) title+=' ';
        title += myClean(line.substr(19));
        return true;
    case 'E': // EDIT
        editor += myClean(line.substr(19));
        return true;
    case 'R': // REF or REFN
        if (line[15]=='N') {
            if (line[35]=='I') issnId += myClean( line.substr(40) );
            if (line[35]=='E') essnId += myClean( line.substr(40) );
        } else {
            journal += myClean( line.substr(19));
            if (journal == "To Be Published") journal = "["+journal+"]";
        }
        return true;
    case 'P': // PUBL o PMID
        if (line[13]=='U') publisher += myClean( line.substr(19) );
        else pubmedId += myClean( line.substr(19) );
        return true;
    case 'D': // DOI
        doi += myClean( line.substr( 19 ));
        return true;
    case 'A': // AUTH
        authors += myClean( line.substr( 19 ));
        return true;
    default:
        //debug("Unkonwn JRNL line:\n%s",line.c_str() );
        return false;
    }


}

bool PdbMetaData::fromPDB(const String &line, PdbToken t){
    switch (t.value) {
    case PdbToken::HEADER:
        date.fromString( line.substr(50,9).data() );
        classification = myClean( line.substr(10,40) );
        sprintf(idCode, "%s", line.substr(62,4).data() );
        return true;
    case PdbToken::MDLTYP:
        modelType += myClean( line.substr(10) );
        return true;
    case PdbToken::TITLE:
        title += myClean( line.substr(10) );
        return true;
    case PdbToken::CAVEAT:
        warnings += myClean( line.substr(10) );
        return true;
    case PdbToken::SPRSDE:
        obsoleting += myClean( line.substr(31) );
        return true;
    case PdbToken::OBSLTE:
        obsoletedBy += myClean( line.substr(31) );
        obsoletedSince.fromString( line.substr(11,9).c_str() );
        return true;
    case PdbToken::EXPDTA:
        experimentalData += myClean( line.substr(10) );
        return true;
    case PdbToken::REVDAT:
        if (!revisionHistory.empty()) revisionHistory += "\n";
        revisionHistory += myClean( line.substr(10) );
        /* TODO: format revisions! */
        return true;
    case PdbToken::COMPND:
        if (!macromolecularInfo.empty()) macromolecularInfo+="\n";
        macromolecularInfo += myClean( line.substr(10) );
        /* TODO: sort per model */
        return true;
    case PdbToken::SOURCE:
        if (!sourceInfo.empty()) sourceInfo+="\n";
        sourceInfo += myClean( line.substr(10) );
        /* TODO: sort per model */
        return true;
    case PdbToken::JRNL:
        //if (!publication.empty()) publication+="\n";
        //publication += myClean( line.substr(12) );
        publication.fromPDB( line );
        return true;
    case PdbToken::KEYWDS:
        keywords += myClean( line.substr(10) );
        return true;
    case PdbToken::SPLIT:
        partOf += myClean( line.substr(10) );
        // todo: "REMARK 350 will contain an amended statement to reflect the entire complex."
        return true;
    }
    return false;
}

String PdbMetaData::urlToActivePdbKeeper() const {
    char res[1024];
    /*std::*/sprintf( res, activePdbKeeperUrlBase.data(), idCode );
    return String( res );
}

typedef vcg::Box3<Scalar> Box;

void ShapingOptions::setDefaults(){
	mode = SPACE_FILL;
	bondRadius = Scalar(0.36);
	licoriceRadius = Scalar(0.5);
	keepHeteros = true;
	keepHydrogens = true;
	colorizeMode = 0;
	showOnlyModel = -1;
	cutPlane = Plane(0,0,0,0);
	selectedOnly = false;
}

/* Qmol::Molecole */
/******************/

void Molecule::apply( const Transformation &t){
    for (uint i=0; i<atom.size(); i++)
        atom[i].pos = t.apply( atom[i].pos);
}

Molecule::Molecule() {}

int Molecule::nModels() const{
    return maxModel * nSymmetries() ;
}

int Molecule::nSymmetries() const{
    return (int)symmetry.size()+1;
}


int Molecule::nChains() const{
    return maxChain+1;
}

/*
int Molecule::ithModelIndex(int i){
    return i+minModel;
}*/

bool Molecule::looksLikeAnAnimation() const{

    int nFrames = maxModel;
    if (nFrames<=1) return false;

    Box aClearBox; aClearBox.SetNull();

    std::vector< Box > boxes(nFrames,aClearBox);
    std::vector< int > atomCount(nFrames,0);

    for (uint ai=0; ai<atom.size(); ai++) {
        int mi = atom[ ai ].modelId - 1;
        if (mi<0) continue;
        boxes[ mi ].Add( atom[ ai ].pos );
        atomCount[ mi ]++;
    }

    float avgOverlap = 0;
    for (int i=0; i<nFrames; i++) {
        int j = (i+1)%nFrames; // next in line

        if (atomCount[i]!=atomCount[j]) return false;

        Box uBox = boxes[i], iBox = boxes[i];
        iBox.Intersect(boxes[j]);
        uBox.Add(boxes[j]);
        avgOverlap += iBox.Diag() / uBox.Diag();
    }

    avgOverlap /= nFrames;
    //debug("Average overlap = %f%%", (int)avgOverlap*100);
    return (avgOverlap > 0.8);

}


bool Atom::isHydrogen() const {
    return (type[0]==' ') && (type[1]=='H');
}

static void _wiserSscanf(const char* st, float *f){
	if (!sscanf( st, "%f", f)) {
		if (sscanf( st, " - %f", f))
			*f=-*f;
		else  *f=1.0;
	}
}

static void _wiserSscanf(const char* st, double *f){
    if (!sscanf( st, "%lf", f)) {
        if (sscanf( st, " - %lf", f))
            *f=-*f;
        else  *f=1.0;
    }
}
void Molecule::updateBoundingSphere(){
    Box box;

	for (uint i=0; i<atom.size(); i++) box.Add( atom[i].pos );
	center = box.Center();
	radius = 0;

	for (uint i=0; i<atom.size(); i++) {
		radius = std::max(
					radius,
					vcg::Distance( atom[i].pos, center)
					);
	}
	radius += 1.7f; // default radius
}

void Molecule::addBondsFromPDB(const String &line ){
    //int firstAtom=atoi( st.substr(6,5).c_str()) - 1;

    int stSize = line.size();
    for (int i=1; stSize>6+i*5+4 && isdigit(line[6+i*5+4]); i++)
    {
        //int secondAtom=atoi( st.substr(6+i*5,5).c_str()) - 1;

        // todo: add bond between firstAtom and secondAtom

        //atom[firstAtom].bond.push_back( secondAtom  );
    }
}

void Molecule::addSymmetryFromPDB(const String &line ){
    int index = atoi( line.substr(7,3).c_str() );
    if (line[59]!='1') {
        symmetry.resize(index+1,Transformation::identity() );
        symmetry[index].fromPDB(line);
    }
}

bool Molecule::makeSingleShape(Shape &p, const ShapingOptions &opt,
                         const AtomTable &table, const ChainTable &chainCols,
                               int modelID){



    p.clear();
    fillShape(p,opt,table,chainCols , 0 , modelID);

    if (!p.ball.size()) return false;
    // it's not safe to inherinting molecule b.sphere, like below...
    p.updateBoundingSphere();
    return true;
}


bool Molecule::makeShape(Shape &p, const ShapingOptions &opt,
                         const AtomTable &table, const ChainTable &chainCols){
    p.clear();

    fillShape(p,opt,table,chainCols , 0 );

    //debug("We have a total of %d spheres!",(int)p.ball.size());
    if (!p.ball.size()) return false;

    // it's not safe to inherinting molecule b.sphere, like below...
    //p.center = center; p.radius = radius;
    p.updateBoundingSphere();
    return true;
}

void Molecule::fillShape(Shape &p, const ShapingOptions &opt,
                         const AtomTable &table, const ChainTable &chainCols,
                         int modelBias, int modelID){


    bool useCutPlane = (opt.cutPlane!=Plane(0,0,0,0));
    Vec cutPlaneNorm(opt.cutPlane[0],opt.cutPlane[1],opt.cutPlane[2]);

    Scalar colPerAtom  = std::max( 1-opt.colorizeMode, 0.0f);
    Scalar colPerModel = std::max(-1+opt.colorizeMode, 0.0f);
    Scalar colPerChain = 1 - colPerAtom - colPerModel;


    int kk = 0;
    for (int si=-1; si<(int)symmetry.size(); si++)
    for (uint i=0; i<atom.size(); i++) {
        const Atom& a (atom[i]);
        int modelId = a.modelId + (si+1)*maxModel + modelBias;

        if(modelId != modelID) continue;

        if ( a.isHetero     && !opt.keepHeteros   ) continue;
        if ( a.isHydrogen() && !opt.keepHydrogens ) continue;
        if ( opt.showOnlyModel!=-1 && modelId!=opt.showOnlyModel) continue;


        const AtomType &type = table[ a.type ];
        Ball newBall;
        if (si==-1)
            newBall.pos = a.pos;
        else
            newBall.pos = symmetry[si].apply( a.pos );

        switch (opt.mode){
        case ShapingOptions::SPACE_FILL:     newBall.rad = type.spacefillRadius; break;
        case ShapingOptions::BALL_AND_STICK: newBall.rad = type.covalentRadius;  break;
        case ShapingOptions::LICORICE:       newBall.rad = opt.licoriceRadius;break;
        default: assert(0);
        }




        //Col fixedColor;
        //newBall.col = fixedColor;

        newBall.col =
            type.col * colPerAtom
            + chainCols[ (a.chainId)%chainCols.size() ] * colPerChain
            + chainCols[ (modelId)%chainCols.size() ] * colPerModel
        ;

        // kill spheres completely cut out by the cut-plane (optional)
//        if (useCutPlane &&
//                (newBall.pos * cutPlaneNorm + opt.cutPlane[3] < -newBall.rad) ) continue;



        // kill unselected spheres (optional)
        if (opt.selectedOnly && !a.isSelected) continue;


        p.ball.push_back( newBall );
    }

}

void Molecule::fillShape(Shape &p, const ShapingOptions &opt,
                         const AtomTable &table, const ChainTable &chainCols,
                         int modelBias ){

    //debug("Pushing %d atoms in shape",(int) atom.size() );
    bool useCutPlane = (opt.cutPlane!=Plane(0,0,0,0));
    Vec cutPlaneNorm(opt.cutPlane[0],opt.cutPlane[1],opt.cutPlane[2]);

    Scalar colPerAtom  = std::max( 1-opt.colorizeMode, 0.0f);
    Scalar colPerModel = std::max(-1+opt.colorizeMode, 0.0f);
    Scalar colPerChain = 1 - colPerAtom - colPerModel;


    for (int si=-1; si<(int)symmetry.size(); si++)
    for (uint i=0; i<atom.size(); i++) {
        const Atom& a (atom[i]);
        int modelId = a.modelId + (si+1)*maxModel + modelBias;

        if ( a.isHetero     && !opt.keepHeteros   ) continue;
        if ( a.isHydrogen() && !opt.keepHydrogens ) continue;   
        if ( opt.showOnlyModel!=-1 && modelId!=opt.showOnlyModel) continue;

        const AtomType &type = table[ a.type ];
        Ball newBall;
        if (si==-1)
            newBall.pos = a.pos;
        else
            newBall.pos = symmetry[si].apply( a.pos );

        switch (opt.mode){
        case ShapingOptions::SPACE_FILL:     newBall.rad = type.spacefillRadius; break;
        case ShapingOptions::BALL_AND_STICK: newBall.rad = type.covalentRadius;  break;
        case ShapingOptions::LICORICE:       newBall.rad = opt.licoriceRadius;break;
        default: assert(0);
        }

        Col fixedColor;
        newBall.col = fixedColor;
//            type.col * colPerAtom
//            + chainCols[ (a.chainId)%chainCols.size() ] * colPerChain
//            + chainCols[ (modelId)%chainCols.size() ] * colPerModel
//        ;


        // kill spheres completely cut out by the cut-plane (optional)
        if (useCutPlane &&
                (newBall.pos * cutPlaneNorm + opt.cutPlane[3] < -newBall.rad) ) continue;

        //if (!a.isSelected) newBall.col = Col(0.75,0.75,0.75); //continue;

        // kill unselected spheres (optional)
        if (opt.selectedOnly && !a.isSelected) continue;

        p.ball.push_back( newBall );

        /*
        if (opt.mode != ShapingOptions::SPACE_FILL ) {
            for (uint j=0; j<a.bond.size(); j++) {
                uint k = a.bond[j];

                if (k>=i) continue; // only one bond for each two

                Tube newTube ( atom[ i ].pos , atom[ k ].pos );
                newTube.rad =
                        (opt.mode == ShapingOptions::LICORICE) ? opt.licoriceRadius : opt.bondRadius;

                p.tube.push_back( newTube );
            }
        }
*/
    }
    //debug("Done (%d balls total)", p.ball.size() );


}


void Atom::fromPDB(const std::string &line){
    if ( !std::isdigit(line[14]) || !std::isdigit(line[15]) ) {
        /* From PDB specification:
         * Atom names start with element symbols right-justified in columns 13-14
         * as permitted by the length of the name. For example, the symbol FE for
         * iron appears in columns 13-14, whereas the symbol C for carbon appears
         * in column 14... */
        type[0] = (std::isdigit(line[12]))? ' ' : std::toupper( line[12] );
        type[1] = std::toupper( line[13] );
    } else {
        /* ...If an atom name has four characters,
         * however, it must start in column 13 even if the element symbol is a
         * single character */
        type[0] = ' ';
        type[1] = std::toupper( line[12] );
    }

    assert((line[0]=='H') || (line[0]=='A') );
    isHetero = (line[0]=='H');

    _wiserSscanf(line.substr( 30, 8).c_str(), &(pos.X()));
    _wiserSscanf(line.substr( 38, 8).c_str(), &(pos.Y()));
    _wiserSscanf(line.substr( 46, 8).c_str(), &(pos.Z()));

    chainId = (int) line[21] - 'A';

    bond.clear();
}

static int readModelFromPDB(const std::string &st){

    return atoi( st.substr(8,15).c_str() );
}


void Molecule::clear(){
    atom.clear();
    metadata.clear();
    symmetry.clear();

    maxChain = 0;
    maxModel = 0;
    nHydro = nHetero = 0;
}

Transformation Transformation::identity(){
    Transformation r;
    r.x = Vec(1,0,0);
    r.y = Vec(0,1,0);
    r.z = Vec(0,0,1);
    r.t = Vec(0,0,0);
    return r;
}

bool Transformation::isIdentity() const{
    return ( x == Vec(1,0,0) )&&
           ( y == Vec(0,1,0) )&&
           ( z == Vec(0,0,1) )&&
           ( t == Vec(0,0,0) );
}

Pos Transformation::apply( const Pos& p ) const{
    return t + Pos( x*p, y*p, z*p );
}

void Transformation::fromPDB(const String &line){
    switch (line[5]){
    case '1':
        _wiserSscanf(line.substr(10,10).c_str(), &(x.X()));
        _wiserSscanf(line.substr(20,10).c_str(), &(x.Y()));
        _wiserSscanf(line.substr(30,10).c_str(), &(x.Z()));
        _wiserSscanf(line.substr(45,10).c_str(), &(t.X()));
        return;
    case '2':
        _wiserSscanf(line.substr(10,10).c_str(), &(y.X()));
        _wiserSscanf(line.substr(20,10).c_str(), &(y.Y()));
        _wiserSscanf(line.substr(30,10).c_str(), &(y.Z()));
        _wiserSscanf(line.substr(45,10).c_str(), &(t.Y()));
        return;
    case '3':
        _wiserSscanf(line.substr(10,10).c_str(), &(z.X()));
        _wiserSscanf(line.substr(20,10).c_str(), &(z.Y()));
        _wiserSscanf(line.substr(30,10).c_str(), &(z.Z()));
        _wiserSscanf(line.substr(45,10).c_str(), &(t.Z()));
        return;
    default:
        ;//debug("Transformation error in line: '%s'",line.data());
    }
    //debug("Transformation in line:\n'%s'",line.data());
}


bool Molecule::loadPDB(const std::wstring &path, FeedbackFunctionT feedback ) throw (std::wstring) {

    clear();
	FILE* f = _wfopen(path.c_str(),L"r");


    fseek(f,0,SEEK_END);

    long totLines = ftell(f) / 81;
    int feedbackEvery = std::max( long(10000), totLines/20 );
    //debug("Feedback every %d lines read",feedbackEvery);
    //debug("Tot lines = %d",(int)totLines);
    fseek(f,0,SEEK_SET);

	int lineN=0;

    if (!f) throw std::string("Cannot open File.");

    char buf[83];
    buf[82]=0;
    int currModelId = 1;
    Transformation localTransform = Transformation::identity();

    bool over = false;
    int tmpCount = 0;
    while (!over){
        //fgets(buf,82,f);
        fread(buf,1,81,f);
        if (buf[80]!='\n') {
            tmpCount++;
            if (tmpCount<10) ////debug("ERROR");
            ;//buf[80] = 0;
        }
        if (feof(f)) break;
        std::string line(buf);
        if (line[0]==0) continue;
        if (line[0]=='\n') continue;
        lineN++;

        PdbToken t = PdbToken::read( line );

        if (feedback && ( (lineN%feedbackEvery) == 0 ) ) {
            bool canceled = (*feedback)( (100*lineN+totLines-1) / totLines);
            if (canceled) {
                clear();
                return false;
            }
        }
        t.doubleCheck( line ); // comment me out if not //debug!


        switch (t.value) {
        case PdbToken::ATOM:
        case PdbToken::HETATM: {
            Atom a;
            a.fromPDB(line);
            a.modelId = currModelId;
            atom.push_back(a);

            if (a.isHetero) nHetero ++;
            if (a.isHydrogen()) nHydro ++;
            if (a.chainId>maxChain) maxChain = a.chainId;
            if (a.modelId>maxModel) maxModel = a.modelId;
            break; }
        case PdbToken::MODEL: {
            int tmp  = readModelFromPDB(line);
            if (currModelId != tmp ) {
                //debug("Funny: %d-th model is labeled as \"model %d\"",tmp, currModelId);
            }
            if ((currModelId == 1) && (atom.size()>0)) {
                //debug("Funny: meloecule has a \"model 1\", but also atoms not in any models!" );
            }
            break;}
        case PdbToken::ENDMDL:
            currModelId++;
            break;
        case PdbToken::END:
            over = true;
            break;
        case PdbToken::SCALE:
            break;
        case PdbToken::ORIG:
            localTransform.fromPDB( line );
            break;
        case PdbToken::MTRIX:
            addSymmetryFromPDB( line );
            break;
        default:
            metadata.fromPDB( line , t );
            break;
        }

	}

    if (!localTransform.isIdentity()) {
        //debug( "applying a tranform to all molecule, "
          //     "as dictated by ORIG fields" );
        /*//debug("localTrnaform x = %f %f %f",localTransform.x[0],localTransform.x[1],localTransform.x[2]);
        //debug("localTrnaform y = %f %f %f",localTransform.y[0],localTransform.y[1],localTransform.y[2]);
        //debug("localTrnaform z = %f %f %f",localTransform.z[0],localTransform.z[1],localTransform.z[2]);
        //debug("localTrnaform t = %f %f %f",localTransform.t[0],localTransform.t[1],localTransform.t[2]);*/

        apply( localTransform );
    }

    compressSymmetryVector();
	updateBoundingSphere();

    //debug("done loading (%d atoms)",atom.size() );
    if (feedback) (*feedback)(100);
    return true;
}


void Molecule::compressSymmetryVector(){
    uint k = 0;
    for (uint i=0; i<symmetry.size(); i++) {
        symmetry[k] = symmetry[i];
        if (!(symmetry[i].isIdentity())) k++;
    }
    //debug("Found %d symmetries (%d implicit ones)",(int)symmetry.size(),(int)k);
    symmetry.resize( k );

}

/* Molecule List */

bool MoleculeList::isEmpty() const{
    for (uint i=0; i<size(); i++) if (!(at(i).isEmpty())) return false;
    return true;
}


int MoleculeList::loadPDBs(const std::vector< std::wstring > &filenames, FeedbackFunctionT feedback ) throw (std::wstring){
    resize( filenames.size() );
    for (uint i=0; i<filenames.size(); i++ ) {
        if (!at(i).loadPDB( filenames[i], feedback )) {
            resize( i );
            return i;
        }
    }
    return filenames.size();
}

bool MoleculeList::makeShape(Shape &p, const ShapingOptions &opt, const AtomTable &table, const ChainTable &chainCols)
{
    p.clear();

    int bias = 0;
    for (uint i=0; i<size(); i++) {
      at(i).fillShape(p,opt,table,chainCols , bias);
      bias += at(i).nModels();
    }

    ////debug("We have a total of %d spheres!",(int)p.ball.size());
    if (!p.ball.size()) return false;

    // attempt to change performance by changing ball orders (didn't do much):
    // p.shuffle();

    p.updateBoundingSphere();
    // note: it's not safe to inherinting molecule b.sphere, like below...
    // p.center = center; p.radius = radius;
    // ... because the molecule b.sphere does not know about settings

    return true;
}


int MoleculeList::nAtoms() const{
    int res=0;
    for (uint i=0; i<size(); i++)
        res += at(i).atom.size();
    return res;
}

int MoleculeList::nHetero() const{
    int tot=0;
    for (uint i=0; i<size(); i++)
        tot += at(i).nHetero;
    return tot;
}

int MoleculeList::nHydro() const{
    int tot=0;
    for (uint i=0; i<size(); i++)
        tot += at(i).nHydro;
    return tot;
}


int MoleculeList::nModels() const{
    int tot=0;
    for (uint i=0; i<size(); i++) tot += at(i).nModels();
    return tot;
}

int MoleculeList::nChains() const{
    int res=0;
    for (uint i=0; i<size(); i++)
        res = std::max( res, at(i).nChains() );
    return res;
}

int MoleculeList::nSymmetries() const{
    // rule: "a molecule list has k!=1 symmetries only if all its molecule do. Otherwise, it has 1."
    if (size()==0) return 1;
    int res=at(0).nSymmetries();
    for (uint i=1; i<size(); i++) {
        int k = at(i).nSymmetries();
        if (k != res) return 1;
    }
    return res;
}

bool MoleculeList::looksLikeAnAnimation() const{
    if (size()==0) return false;
    if (size()==1) return at(0).looksLikeAnAnimation();

    /* else, it does, if each molecule does AND
     * they all have the same number of frames */
    int nFrames = -1;
    uint nAtoms = 0;
    int nSymmetries = 0;
    for (uint i=0; i<size(); i++) {
        if (!at(i).looksLikeAnAnimation() && (at(i).maxModel>1) ) return false;

        if (i==0) {
            nFrames = at(i).maxModel;
            nAtoms = at(i).atom.size();
            nSymmetries = at(i).nSymmetries();
        } else {
            if ( nFrames != at(i).maxModel ) return false;
            if ( nAtoms != at(i).atom.size() ) return false;
            if ( nSymmetries != at(i).nSymmetries() ) return false;
        }

    }
    return true;
}


} // namespace qmol
