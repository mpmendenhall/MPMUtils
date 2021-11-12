/// \file lineReader.hh load istream line-by-line
// Michael P. Mendenhall, LLNL 2021

#ifndef LINEREADER_HH
#define LINEREADER_HH

#include <istream>
using std::istream;
#include <string>
using std::string;
#include <sstream>
using std::stringstream;

/// load istream line-by-line
class lineReader: public stringstream {
public:
    /// Constructor
    explicit lineReader(istream& _i): lineSrc(_i) { }

    /// load next line
    lineReader& next(char delim = '\n') {
        std::getline(lineSrc, lstr, delim);
        str(lstr);
        clear();
        ++lno;
        return *this;
    }

    /// load next if only whitespace to end of line
    void checkEnd() {
        while(peek() == ' ') get();
        if(peek() == EOF) next();
    }

    string lstr;        ///< line string
    int lno = 0;        ///< line number (starting from 1)
    istream& lineSrc;   ///< input
};

#endif
