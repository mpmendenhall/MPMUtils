/// \file lineReader.hh load istream line-by-line
// Michael P. Mendenhall, LLNL 2021

#ifndef LINEREADER_HH
#define LINEREADER_HH

#include "char_istream.hh"
using std::istream;

/// load istream line-by-line
class lineReader: public char_istream {
public:
    /// Constructor
    explicit lineReader(istream& _i): lineSrc(_i) { }

    /// load next line
    lineReader& next(char delim = '\n') {
        std::getline(lineSrc, lstr, delim);
        set_str(lstr);
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
