/// @file ElectronBindingEnergy.cc

#include "ElectronBindingEnergy.hh"
#include <stdio.h>

const string BindingEnergyTable::shellnames = "KLMNOPQRST";

BindingEnergyTable::BindingEnergyTable(const Stringmap& m): Z(m.getDefault("Z",0)), nm(m.getDefault("name","")) {
    for(unsigned int n=0; n<shellnames.size(); n++) {
        vector<double> v;
        for(unsigned int s=1; s<=9; s++) {
            double b = m.getDefault(c_to_str(shellnames[n])+(n+s==1?"":to_str(s)),0);
            if(b) v.push_back(b/1000.0);
            else break;
        }
        if(v.size()) eBinding.push_back(v);
        else break;
    }
}

const vector<double>& BindingEnergyTable::getShellBinding(unsigned int n) const {
    if(n>=eBinding.size()) return noBinding;
    return eBinding[n];
}

double BindingEnergyTable::getSubshellBinding(unsigned int n, unsigned int m) const {
    const vector<double>& v = getShellBinding(n);
    if(m>=v.size()) return 0;
    return v[m];
}

void BindingEnergyTable::display() const {
    printf("----- %u %s Electron Binding -----\n",Z,nm.c_str());
    for(unsigned int n=0; n<eBinding.size(); n++) {
        printf("\t%c:",shellnames[n]);
        for(unsigned int m=0; m<eBinding[n].size(); m++)
            printf("\t%.2f",eBinding[n][m]);
        printf("\n");
    }
}

//----------------------------------------------

BindingEnergyLibrary::BindingEnergyLibrary(const SMFile& Q) {
    for(auto& b: Q.retrieve("binding"))
        tables.emplace(b.getDefault("Z",0), new BindingEnergyTable(b));
}

BindingEnergyLibrary::~BindingEnergyLibrary() {
    for(const auto& kv: tables) delete kv.second;
}

const BindingEnergyTable* BindingEnergyLibrary::getBindingTable(unsigned int Z, bool allownullptr) const {
    map<unsigned int,BindingEnergyTable*>::const_iterator it =  tables.find(Z);
    if(it==tables.end()) {
        if(allownullptr) return nullptr;
        throw std::runtime_error("Missing Element");
    }
    return it->second;
}

void BindingEnergyLibrary::display() const {
    for(auto& kv: tables) kv.second->display();
}

