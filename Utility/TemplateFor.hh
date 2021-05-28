/// \file TemplateFor.hh
// -- Michael P. Mendenhall, LLNL 2021

// Nota Bene: the following employs unnecessary template evil
// just to test it out. Please do not emulate this code style
// unless you enjoy inflicting pain on yourself and others.

/// sort points along axis
template<size_t A>
struct sort_axis {
    template<typename U>
    bool operator()(const U& a, const U& b) const { return a[A] < b[A]; }
};

/// compare value to Ath element
template<size_t A>
struct compare_element {
    template<typename U, typename V>
    bool operator()(const U& a, const V& b) const { return a[A] < b; }
};

/// Template for-loop helper
template<size_t N>
struct do_N {
    /// Call function c.do_it<0>(); c.do_it<1>(); ... ; c.do_it<1>(N-1);
    template<class C>
    static void do_it(C& c) { do_N<N-1>::do_it(c); c.template do_it<N-1>(); }
    /// launder run-time integer value to template argument
    template<class C>
    static void do_nth(int i, C& c) { if(i==N-1) c.template do_it<N-1>(); else do_N<N-1>::do_nth(i,c); }
};

/// Specialization for terminating do_N
template<>
struct do_N<0> {
    template<class C>
    static void do_it(C&) { }
    template<class C>
    static void do_nth(int, C&) { }
};
