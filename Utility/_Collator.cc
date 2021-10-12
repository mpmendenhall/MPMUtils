/// \file Collator.cc

#include "_Collator.hh"

size_t _Collator::add_input(int nreq) {
    ++inputs_waiting;
    size_t nI = input_n.size();
    input_n.emplace_back(0,0);
    if(nreq) change_required(nI, nreq);
    return nI;
}

void _Collator::change_required(size_t nI, int i) {
    auto& n = input_n.at(nI).first;
    if(n <= 0 && n-i > 0) {
        if(inputs_waiting <= 0) throw std::logic_error("invalid inputs reduction");
        --inputs_waiting;
    }
    if(n > 0 && n-i <= 0) ++inputs_waiting;
    input_n[nI].second += i;
    n -= i;
}

void _Collator::reset() {
    signal(DATASTREAM_FLUSH);
    inputs_waiting = 0;
    input_n.clear();
}

vector<size_t> _Collator::get_waiting() const {
    vector<size_t> v;
    for(size_t i=0; i<input_n.size(); ++i)
        if(input_n[i].first <= 0) v.push_back(i);
    return v;
}

vector<size_t> _Collator::get_free() const {
    vector<size_t> v;
    for(size_t i=0; i<input_n.size(); ++i)
        if(input_n[i].second < 0) v.push_back(i);
    return v;
}

vector<size_t> _Collator::unstick() {
    auto v = get_waiting();
    for(auto nI: v) set_required(nI,-1);
    return v;
}
