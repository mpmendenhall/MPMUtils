/// \file OrderedData.hh Wrapper template for adding ordering tag to objects
// -- Michael P. Mendenhall, LLNL 2020

#ifndef ORDEREDDATA_HH
#define ORDEREDDATA_HH

template<class T, typename _ordering_t = double>
struct OrderedData {
    typedef T val_t;
    typedef _ordering_t ordering_t;

    ordering_t t;   ///< ordering value
    val_t o;        ///< contained value

    /// Constructor
    OrderedData(const ordering_t& _t, const val_t& _o): t(_t), o(_o) { }
    /// get ordering value
    explicit operator ordering_t() const { return t; }

    /// helper to strip ordering from contents
    struct val_extractor {
        /// converting constructor
        explicit val_extractor(const OrderedData& d): x(d.o) { }
        /// auto-convert to extracted contents
        operator val_t() const { return x; }
        val_t x; ///< extracted contents
    };
};

#endif
