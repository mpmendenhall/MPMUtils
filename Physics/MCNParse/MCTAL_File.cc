/// \file MCTAL_File.cc

#include "MCTAL_File.hh"
#include <stdio.h>

MCTAL_File::MCTAL_File(istream& i): lr(i), hdr(lr) {
    try {
        for(int n=0; n < hdr.ntal; ++n) emplace_back(&lr);
    } catch(std::runtime_error& e) {
        printf("Error loading MCTAL file at line %i [%s]\n", lr.lno, lr.lstr.c_str());
        throw;
    }
}

void MCTAL_File::display() const {
    printf("\n*******************************\n");
    hdr.display();
    for(auto& t: *this) {
        printf("\n---------------------------------------\n");
        t.display();
    }
    printf("*******************************\n\n");
}
