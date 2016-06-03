#include <fstream>

#include "atomTable.h"


namespace qmol{

static void _trim(std::string &str) {
    std::string output;
    int len = str.length();
    for (int i=0; i<len; i++) {
        if (str[i]=='\t' || str[i]=='\"' || str[i]==' ' ) ;
        else {
            output.push_back( str[i]);
        }
    }
    str = output;/*
	// trim initial spaces
	size_t startpos = str.find_first_not_of(" \t\"");
	size_t endpos = str.find_last_not_of(" \t\"");
	if (startpos<endpos)
        str = str.substr( startpos, endpos+1-startpos );*/
}


AtomType::AtomType(){}

static Col _randomCol(){
	Col res;
	do {
		res = Col(rand()%255,rand()%255,rand()%255)/255.0f;
	} while (res.Norm()<0.25);
	return res;
}

void ChainTable::setRandom(int size){
	resize(size);
	for (int i=0; i<size; i++) std::vector<Col>::at(i) = _randomCol();
}

Col ChainTable::operator [](int i) const {
	if (i>=(int)size()) return Col(1,1,1);
	return std::vector<Col>::at(i);
}

void AtomType::fromCSVline(std::string input, char *code){

    code[0]=' '; code[1]=' ';
	col = Col(1,0.7f,0);
	covalentRadius = 1.0;
	spacefillRadius = 1.5;
	int i=0;
	for(
	    std::string::size_type a = 0, b = input.find(',');
	    a != std::string::npos || b != std::string::npos;
	    (a=(b==std::string::npos)?b:++b),b=input.find(',',b)
	    ) {

        if (b<=a) return;
		std::string token = input.substr(a,b-a);

		Scalar tmp;
		switch(i++) {
		case 0:
			_trim(token);
            if (token.length()==1) code[1] = token[0];
            else
            if (token.length()==2) {
                code[0] = token[0];
                code[1] = token[1];
            }
			break;
		case 1:
			tmp = (Scalar)atof( token.c_str() );
			if (tmp) covalentRadius = tmp;
			break;
		case 2:
			tmp  = (Scalar)atof( token.c_str() );
			if (tmp) spacefillRadius = tmp;
			break;
		case 3:
			_trim(token);
			if (token.length()==6) col =  string2col( token.c_str() );
			break;
		}
	}
}


bool _skip(std::string &line){
	_trim(line);
	if (line[0]==':') return true; // skip comments
	if (line.length()==0) return true; // skip empty lines
	return false;
}

bool AtomTable::loadCSV(std::string filename)  throw (std::wstring){

    clearAll();
	std::ifstream file;

	file.open(filename.data(),std::ios_base::in);
	if (!file.is_open()) return false;

	char line[300];
	AtomType at;

	while ( !file.eof()) {
		file.getline(line,300);
		std::string linest(line);

		if (file.eof()) break;
		if (!_skip(linest))
		{
            char code[2];
            at.fromCSVline(linest,code);
			data[ twoCharsToIndex(code) ] = at;
		}
	}
	file.close();

	return true;

}

AtomType::AtomType(Scalar _spacefillRadius, Scalar _covalentRadius, const char *_col)
{
	if (_spacefillRadius) spacefillRadius = _spacefillRadius;
	else spacefillRadius = 1.5;

	if (_covalentRadius) covalentRadius = _covalentRadius;
	else covalentRadius = 1.0;

	if (_col) col =  string2col(_col);
	else col = Col(1.0,1.0,1.0);
}

AtomTable::AtomTable()
{
	clearAll();
}

void AtomTable::clearAll(){
	data.clear();

	// fills the table with void (default) values)
    AtomType defValue(0,0,"660066");

	data.resize( MAX_CHAR*MAX_CHAR, defValue );

    /*data[twoCharsToIndex(" O")].fromCSVline("O, 1.2 , 0.8 , FFAAAA");

    data[ twoCharsToIndex(" O") ] = AtomType(1.2f, 0.8f, "FFAAAA");
    data[ twoCharsToIndex(" C") ] = AtomType(1.1f, 0.8f, "DDDDDD");
    data[ twoCharsToIndex(" H") ] = AtomType(0.5f, 0.3f, "FFFFFF");*/

}

static int _hexDigit(char c){
    if (c>='0' && c <='9') return c-'0';
    if (c>='A' && c <='F') return 10+c-'A';
    if (c>='a' && c <='f') return 10+c-'a';
	return 0;
}

static char _hexDigit(int i){
	if (i < 9) return i+'0'; else return i-10+'A';
}

static void _colComp2string(Scalar q, char* res){
	int base16 = (int) q * 255;
	if (base16 < 0x00) base16 = 0x00;
	if (base16 > 0xFF) base16 = 0xFF;
	res[0] = _hexDigit(base16/16);
	res[1] = _hexDigit(base16%16);
}

static Scalar _string2colComp(const char* str){
	return (_hexDigit(str[0])*16 + _hexDigit(str[1])) / Scalar(255);
}

char* AtomType::col2string(const Col &c){
	static char res[7];
	res[6] = 0;
	_colComp2string(c[0],res+0);
	_colComp2string(c[1],res+2);
	_colComp2string(c[2],res+4);
	return res;
}


Col AtomType::string2col(const char *str){
	return Col(
		_string2colComp(str+0),
		_string2colComp(str+2),
		_string2colComp(str+4)
	);

}

int AtomTable::charToIndex(char c){
    if (c>='a' && c<='z') return c - 'a' + 1;
    if (c>='A' && c<='Z') return c - 'A' + 1;
    // if (c>='0' && c<='9') return c + (2 - '0' + 'z' - 'a');
	return 0; // anything else, including space
}

int AtomTable::twoCharsToIndex(const char* c){
    return ( charToIndex(c[1]) + MAX_CHAR* charToIndex(c[0]) );
}

} // namespace qmol
