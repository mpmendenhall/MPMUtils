/// \file "PolyFit.hh" Polynomial fitting configuration
/*
 * PolyFit.hh, part of the MPMUtils package.
 * Copyright (c) 2007-2018 Michael P. Mendenhall
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

#ifndef POLYFIT_HH
#define POLYFIT_HH

#include "Polynomial.hh"
#include "PolyEval.hh"
#include <cassert>

/// Configure LinMin-like classes to fit polynomial P(X;coeffs) = y(X)
template<class Poly>
class PolyFit: protected PolyEval<> {
public:
    /// Default constructor
    PolyFit() { }
    /// Constructor with fit polynomial
    PolyFit(const Poly& PP): P(PP) { }

    /// set coordinate grid for evaulation
    using PolyEval<>::setX;

    Poly P; ///< Solution polynomial form

    /// configure fit matrix for polynomial over X grid
    template<typename LM>
    void configure(LM& fitter) {
        assert(P.size() == fitter.nVar());
        fitter.setNeq(npts);
        size_t i = 0;
        for(auto& kv: P) {
            vector<double> v;
            evalMonomial(kv.first.get(), v);
            size_t j = 0;
            for(auto c: v) fitter.setM(j++,i,c);
            i++;
        }
    }

    /// load coefficients into polynomial
    template<typename LM>
    Poly& load(const LM& fitter) {
        vector<double> vx;
        fitter.getx(vx);
        if(vx.size() < P.size()) throw;
        size_t i = 0;
        for(auto& kv: P) kv.second = vx[i++];
        return P;
    }
};

#endif
