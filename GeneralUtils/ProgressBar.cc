/// \file ProgressBar.cc
/* 
 * ProgressBar.cpp, part of the MPMUtils package
 * Copyright (c) 2007-2014 Michael P. Mendenhall
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "ProgressBar.hh"

ProgressBar::ProgressBar(uint64_t nt, unsigned int ns, bool v, const std::string& label):
ntotal(nt), nsteps(ns), c(0), s(0), verbose(v) {
    if(verbose) {
        printf("%s+",label.c_str()); 
        for(unsigned int i=0; i<nsteps; i++)
            printf("-");
        printf("\n|");
        fflush(stdout);
    }
}

void ProgressBar::update(unsigned int i) {
    if(i<=c) return;
    c = i;
    
    unsigned int smax = (uint64_t(nsteps)*c)/ntotal;
    for(; s < smax; s++) {
        if(verbose) {
            printf("*");
            fflush(stdout);
        }
    }
}
