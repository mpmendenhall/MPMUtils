/// @file ElectronBindingEnergy.hh Atomic shell binding energy tables
// -- Michael P. Mendenhall

#ifndef ELECTRONBINDINGENERGY_HH
#define ELECTRONBINDINGENERGY_HH

#include "SMFile.hh"

/// table of electron binding energies
class BindingEnergyTable {
public:
    /// constructor from Stringmap
    explicit BindingEnergyTable(const Stringmap& m);
    /// get subshell binding energies for given shell
    const vector<double>& getShellBinding(unsigned int n) const;
    /// get subshell binding energy
    double getSubshellBinding(unsigned int n, unsigned int m) const;
    /// display summary of binding energies
    void display() const;
    /// get Z
    unsigned int getZ() const { return Z; }
    /// get element name
    string getName() const { return nm; }

    static const string shellnames;

protected:
    unsigned int Z;                     ///< element number
    string nm;                          ///< element name abbrev.
    vector< vector<double> > eBinding;  ///< binding energy by shell and subshell
    const vector<double> noBinding;     ///< empty vector for missing info
};

/// catalog of many BindingEnergyTables
class BindingEnergyLibrary {
public:
    /// constructor from SMFile containing element tables
    explicit BindingEnergyLibrary(const SMFile& Q);
    /// destructor
    ~BindingEnergyLibrary();
    /// get BindingEnergyTable for specified element
    const BindingEnergyTable* getBindingTable(unsigned int Z, bool allownullptr = false) const;
    /// display contents
    void display() const;
protected:
    map<unsigned int,BindingEnergyTable*> tables; ///< binding energy tables by element
};

#endif
