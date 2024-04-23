/// @file iterate_map.hh Helper to iterate map keys and values separately
// -- Michael P. Mendenhall (LLNL) 2024

/// Iteration over map keys
template<class M>
class _iter_keys {
public:
    /// Constructor
    explicit _iter_keys(const M& _m): m(_m) { }

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

/// for c++11 function template deduction
template<class M>
_iter_keys<M> iter_keys(const M& m) { return _iter_keys<M>(m); }


/// Iteration over map values
template<typename M>
class _iter_vals {
public:
    /// Constructor
    explicit _iter_vals(M& _m): m(_m) { }

    /// iterator through keys (possibly const if M is const typename)
    class iterator {
    protected:
        decltype(std::declval<M>().begin()) it;  ///< underlying iterator
    public:
        /// constructor
        explicit iterator(decltype(it) _it): it(_it) { }
        /// dereference to get contents --- templated so `auto` works in c++11
        template<typename UNUSED=int>
        auto& operator*() { return it->second; }
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
        typename M::const_iterator it;  ///< underlying iterator
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

/// for c++11 function template deduction
template<typename M>
_iter_vals<M> iter_vals(M& m) { return _iter_vals<M>(m); }

