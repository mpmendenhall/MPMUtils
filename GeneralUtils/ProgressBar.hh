/// \file ProgressBar.hh text-based progress bar
/* 
 * ProgressBar.hh, part of the MPMUtils package
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

/// \file ProgressBar.hh Text output progress bar
#ifndef PROGRESSBAR_HH
/// Make sure this header is only loaded once
#define PROGRESSBAR_HH

#include <stdio.h>
#include <string>

/// class for printing a progress bar to stdout
class ProgressBar {
public:
    
    /// constructor, given total number of items and number of output steps
    ProgressBar(uint64_t nt, unsigned int ns = 20, bool v=true, const std::string& label="");
    
    /// destructor
    ~ProgressBar() { if(verbose) printf("* Done.\n"); }
    
    /// update status at i items completed
    void update(uint64_t i);
    /// increment status by n items
    void increment(int64_t n) { update(c+n); }
    
protected:
    
    const uint64_t ntotal;      ///< total number of items to completion
    const unsigned int nsteps;  ///< number of steps to mark
    
    uint64_t c;                 ///< number of items completed
    unsigned int s;             ///< steps displayed
    const bool verbose;         ///< whether to display the progress bar
};

#endif
