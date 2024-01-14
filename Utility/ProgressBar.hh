/// @file ProgressBar.hh text-based progress bar
/*
 * ProgressBar.hh, part of the MPMUtils package
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

#ifndef PROGRESSBAR_HH
#define PROGRESSBAR_HH

#include <stdio.h>
#include <cstdint> // for uint64_t

/// Print a progress bar to stdout
class ProgressBar {
public:
    /// Constructor, given total number of items and number of output steps
    explicit ProgressBar(uint64_t nt, unsigned int ns = 20, bool v=true);
    /// Destructor
    ~ProgressBar() { if(verbose) printf("  Done.\n"); }

    /// update status at i items completed
    void update(uint64_t i) { _update(i*nsteps); }
    /// increment status by n items
    void increment(int64_t n = 1) { _update(c_nstp + n*nsteps); }
    /// prefix operator++ to increment
    ProgressBar& operator++() { increment(); return *this; }
    /// check if completed
    operator bool() const { return c_nstp == nstp_ntot; }
    /// current item number
    operator uint64_t() const { return c_nstp/nsteps; }

    const uint64_t ntotal;  ///< total number of items to completion
    const uint64_t nsteps;  ///< number of steps to mark

protected:
    /// update c_nstp
    void _update(uint64_t cn);

    const uint64_t nstp_ntot;   ///< nsteps*ntotal
    uint64_t c_nstp;        ///< (number of items completed)*nsteps
    uint64_t s_ntot;        ///< (number of steps displayed)*ntotal
    const bool verbose;     ///< whether to display the progress bar
};

#endif
