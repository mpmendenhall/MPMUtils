/// \file MCTAL_Tally.cc

#include "MCTAL_Tally.hh"
#include <stdio.h>
#include "to_str.hh"

MCTAL_Tally::MCTAL_Tally(istream* i):
Fbins("Object Number"),
Dbins("Total/Direct"),
Ubins("User Bin"),
Sbins("Segment Number"),
Mbins("Multiplier"),
Cbins("cosine"),
Ebins("Energy [MeV]"),
Tbins("Time [shakes]") { if(i) load(*i); }

void MCTAL_Tally::load(istream& i) {
    string s_tmp;
    i >> s_tmp;
    check_expected(upper(s_tmp), "TALLY");

    i >> probnum;
    tally = tally_t(probnum % 10);
    int p;
    i >> p;
    int _int;
    i >> _int;
    detector = detector_t(_int);
    while(i.peek() == ' ') i.ignore(1);
    if(i.peek() != '\n') {
        i >> _int;
        tallymod = tallymod_t(_int);
    }
    i.ignore(256, '\n');

    if(p == 1 || p == 3 || p == 5 || p == 7) ptype.insert(PTYPE_N);
    if(p == 2 || p == 3 || p == 6 || p == 7) ptype.insert(PTYPE_P);
    if(p == 4 || p == 5 || p == 6 || p == 7) ptype.insert(PTYPE_E);
    if(p < 0) for(ptype_t n = PTYPE_N; n <= PTYPE_ION; ++n) {
        i >> p;
        if(p) ptype.insert(n);
    }
    i.ignore(256, '\n');

    // The FC card lines, if any, each starting with 5 blanks}
    while(i.peek() == ' ') i.ignore(256, '\n');

    Fbins.load(i);
    Dbins.load("D", i);
    Ubins.load("U", i);
    Sbins.load("S", i);
    // TODO For radiograph tally, a list of the bin boundaries will be printed.
    if(detector == DET_FIC || detector == DET_FIR)
        throw std::logic_error("Radiograph S bins unimplemented");
    Mbins.load("M", i);
    Cbins.load("C", i);
    Ebins.load("E", i);
    Tbins.load("T", i);

    getline(i, s_tmp);
    s_tmp = upper(s_tmp);
    if(!(s_tmp == "VALS" || s_tmp == "VALS_PERT"))
        throw std::runtime_error("Expected 'VALS [PERT]', got '" + s_tmp + "'");

    int nentries = 1;
    for(auto a = AXIS_T; a < AXIS_END; ++a) {
        auto nb = axis(a).nbins;
        _axis(a).stride = nentries;
        if(nb > 1) {
            axes.emplace_back(a);
            nentries *= nb;
        }
    }

    while(nentries--) {
        valerr_t v;
        i >> v.val >> v.rel_err;
        push_back(v);
    }
    i.ignore(256, '\n');

    tfc.load(i);

    if(toupper(i.peek()) == 'K') kcyc.load(i);
}

string MCTAL_Tally::ptype_name(ptype_t p) {
    if(PTYPE_N <= p && p <= PTYPE_E) {
        static const char* ptype_names[PTYPE_E+1] = {"?", "n", "p", "e"};
        return ptype_names[p];
    }
    return "ptcl[" + to_str(p) + "]";
}

const char* MCTAL_Tally::tally_name(tally_t t) {
    if(t >= TALLY_NONE && t <= TALLY_PULSE) {
        static const char* tally_names[TALLY_PULSE+1] = {
            "Null", "surface flux", "surface current", "unused",
            "cell flux", "point flux", "energy deposition",
            "fission energy deposition", "pulse size" };
        return tally_names[t];
    }
    throw std::runtime_error("Undefined tally type number " + to_str(t));
}

const char* MCTAL_Tally::detector_name(detector_t t) {
    if(t >= DET_NONE && t <= DET_FIC) {
        static const char* det_names[DET_FIC+1] = {
            "Null", "Point", "Ring", "Pinhole", "FIR", "FIC" };
        return det_names[t];
    }
    throw std::runtime_error("Undefined detector type number " + to_str(t));
}

const valerr_t&
MCTAL_Tally::operator()(size_t a1, size_t a2, size_t a3, size_t a4,
                        size_t a5, size_t a6, size_t a7, size_t a8) const {
    auto it = axes.begin();
    size_t i = 0;
    for(auto a: {a1,a2,a3,a4,a5,a6,a7,a8}) {
        i += a * axis(*it).stride;
        if(++it == axes.end()) break;
    }
    return at(i);
}

const MCTAL_Axis& MCTAL_Tally::axis(tallyax_id_t a) const {
    switch(a) {
        case AXIS_F: return Fbins;
        case AXIS_D: return Dbins;
        case AXIS_U: return Ubins;
        case AXIS_S: return Sbins;
        case AXIS_M: return Mbins;
        case AXIS_C: return Cbins;
        case AXIS_E: return Ebins;
        case AXIS_T: return Tbins;
        default: throw std::logic_error("undefined axis");
    }
}

void MCTAL_Tally::display() const {
    printf("F%i %s Tally %i%s, %zu entries for particles {",
           tally, tally_name(tally), probnum, detector == DET_NONE? "" :
           (string(" (") + detector_name(detector) + " detector)").c_str(),
           size());
    for(auto p: ptype) printf(" %s", ptype_name(p).c_str());
    printf(" } in objects ");
    Fbins.showbins();
    printf("\n");

    for(auto a: axes) { printf("\t"); axis(a).display(); }
    if(tfc.size()) tfc.display();
    if(kcyc.n_cyc) kcyc.display();
}
