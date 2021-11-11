/// \file MCTAL_File.cc

#include "MCTAL_File.hh"
#include <stdio.h>

MCTAL_File::MCTAL_File(istream& i): hdr(i) {
    for(int n=0; n < hdr.ntal; ++n) emplace_back(&i);
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
