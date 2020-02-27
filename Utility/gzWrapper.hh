/// \file gzWrapper.hh Wrapper for transparently reading .gz files using boost::iostreams
// Michael P. Mendenhall, 2017

#ifndef GZWRAPPER_HH
#define GZWRAPPER_HH

#include <iostream>
#include <fstream>
#include <string>
#include <utility> // for std::move

#ifdef WITH_BOOST

#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/gzip.hpp>

/// Wrapper for transparently reading .gz files using boost::iostreams
class gzWrapper {
public:
    /// Constructor
    explicit gzWrapper(const std::string& fname, bool isGZ = true): isZipped(isGZ),
    fgz(fname.c_str(), std::ios_base::in | std::ios_base::binary), f(&fsb) {
        if(isZipped) fsb.push(boost::iostreams::gzip_decompressor());
        fsb.push(fgz);
    }

    /* doesn't work on older gcc, which won't std::move fgz
    /// Move constructor
    gzWrapper(gzWrapper&& w): isZipped(w.isZipped), fgz(std::move(w.fgz)), f(&fsb) {
        if(isZipped) fsb.push(boost::iostreams::gzip_decompressor());
        fsb.push(fgz);
    }
    */

protected:
    bool isZipped;      ///< whether input file needs decompression
    std::ifstream fgz;  ///< .gz file input
    boost::iostreams::filtering_streambuf<boost::iostreams::input> fsb; ///< decompressor
public:
    std::istream f;     ///< transparent access to decompressed .gz file as istream
};

/// Wrapper for transparently writing .gz files using boost::iostreams
class gzOutWrapper {
public:
    /// Constructor
    explicit gzOutWrapper(const std::string& fname, bool isGZ = true): isZipped(isGZ),
    fgz(fname.c_str(), std::ios_base::out | std::ios_base::binary), f(&fsb) {
        if(isZipped) fsb.push(boost::iostreams::gzip_compressor());
        fsb.push(fgz);
    }

protected:
    bool isZipped;      ///< whether input file needs decompression
    std::ofstream fgz;  ///< .gz file input
    boost::iostreams::filtering_streambuf<boost::iostreams::output> fsb; ///< compressor
public:
    static const bool canZip = true;
    std::ostream f;     ///< transparent access to decompressed .gz file as istream
};

#else

#include <stdio.h>

/// Placeholder for reading uncompressed files when compiled without boost
class gzWrapper {
public:
    /// Constructor
    gzWrapper(const std::string& fname, bool isZipped = true):
    f(fname.c_str(), std::ios_base::in | std::ios_base::binary) {
        if(isZipped) {
            printf("Must compile WITH_BOOST to read .gz files!\n");
            throw;
        }
    }
    std::ifstream f;    ///< file input
};

/// Placeholder for writing uncompressed files when compiled without boost
class gzOutWrapper {
public:
    /// Constructor
    gzOutWrapper(const std::string& fname, bool isZipped = true):
    f(fname.c_str(), std::ios_base::out | std::ios_base::binary) {
        if(isZipped) {
            printf("Must compile WITH_BOOST to write .gz files!\n");
            throw;
        }
    }
    static const bool canZip = false;
    std::ofstream f;    ///< file input
};

#endif
#endif
