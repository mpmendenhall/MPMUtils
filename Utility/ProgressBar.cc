/// @file ProgressBar.cc
/*
 * ProgressBar.cc, part of the MPMUtils package
 * Copyright (c) 2007-2019 Michael P. Mendenhall
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
#include <limits> // for std::numeric_limits
#include <stdexcept> // for std::domain_error

ProgressBar::ProgressBar(uint64_t nt, unsigned int ns, bool v):
ntotal(nt), nsteps(ns), nstp_ntot(nt*ns), c_nstp(0), s_ntot(0), verbose(v) {

    if(std::numeric_limits<uint64_t>::max()/nsteps < ntotal)
        throw std::domain_error("Overflow in progress bar counts!");

    if(!verbose) return;

    printf("+");
    for(unsigned int i=0; i<nsteps; i++) printf("-");
    printf("\n|");
    fflush(stdout);
}

void ProgressBar::_update(uint64_t cn) {
    if(cn <= c_nstp) return;

    c_nstp = cn;

    if(verbose && ntotal) {
        while(c_nstp > s_ntot) {
            s_ntot += ntotal;
            printf("*");
            fflush(stdout);
        }
    } else s_ntot = c_nstp;
}
