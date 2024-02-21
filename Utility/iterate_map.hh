/// @file iterate_map.hh Helper to iterate map keys and values separately
// -- Michael P. Mendenhall (LLNL) 2024

/// Iteration over map keys
template<class M>
class iter_keys {
public:
    /// Constructor
    iter_keys(M& _m): m(_m) { }

    /// iterator through keys
    class iterator {
    protected:
        typename M::const_iterator it;  ///< underlying setting iterator
    public:
        /// constructor
        explicit iterator(typename M::const_iterator _it): it(_it) { }
        /// dereference to get contents
        decltype(it->first) operator*() { return it->first; }
        /// move to next
        iterator& operator++() { ++it; return *this; }
        /// check if iterators unequal
        bool operator!=(const iterator& rhs) const { return it != rhs.it; }
    };

    /// starting iterator
    iterator begin() { return iterator(m.begin()); }
    /// ending iterator
    iterator end()   { return iterator(m.end()); }

protected:
    const M& m;
};

/// Iteration over map values
template<class M>
class iter_vals {
public:
    /// Constructor
    iter_vals(M& _m): m(_m) { }

    /// iterator through keys
    class iterator {
    protected:
        typename M::iterator it;  ///< underlying setting iterator
    public:
        /// constructor
        explicit iterator(typename M::iterator _it): it(_it) { }
        /// dereference to get contents
        decltype(it->second)& operator*() { return it->second; }
        /// move to next
        iterator& operator++() { ++it; return *this; }
        /// check if iterators unequal
        bool operator!=(const iterator& rhs) const { return it != rhs.it; }
    };

    /// starting iterator
    iterator begin() { return iterator(m.begin()); }
    /// ending iterator
    iterator end()   { return iterator(m.end()); }

    /// const iterator through keys
    class const_iterator {
    protected:
        typename M::const_iterator it;  ///< underlying setting iterator
    public:
        /// constructor
        explicit const_iterator(typename M::const_iterator _it): it(_it) { }
        /// dereference to get contents
        const decltype(it->second)& operator*() { return it->second; }
        /// move to next
        iterator& operator++() { ++it; return *this; }
        /// check if iterators unequal
        bool operator!=(const iterator& rhs) const { return it != rhs.it; }
    };

    /// starting iterator
    const_iterator begin() const { return const_iterator(m.begin()); }
    /// ending iterator
    const_iterator end() const { return const_iterator(m.end()); }

protected:
    M& m;
};
