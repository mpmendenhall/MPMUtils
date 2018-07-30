/// \file "PolyFit.hh" Least-squares linear multidimensional polynomial fits
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
#include "LinMin.hh"

/// Polynomial fitter P(X) = y
template<class Poly>
class PolyFit: protected PolyEval<>, protected LinMin {
public:
    /// Default constructor
    PolyFit() { }
    /// Constructor with fit polynomial
    PolyFit(const Poly& PP): P(PP) { }

    /// set coordinate grid for evaulation
    using PolyEval<>::setX;
    /// get residuals
    using LinMin::getr;
    /// get sum of squares of residuals
    using LinMin::ssresid;

    /// set polynomial fit form (coefficients ignored)
    void setPoly(const Poly& PP) { P = PP; LinMin::clear(); }
    /// get solution polynomial
    const Poly& getPoly() const { return P; }

    /// solve for RHS y (results stored in Poly coefficients)
    template<typename YVec>
    const Poly& solve(const YVec& y) {
        assert(y.size() == npts);
        if(!Xd.size()) LinMin::clear(); // need new M for updated Xs

        if(!M) {
            LinMin::resize(y.size(), P.size());
            size_t i = 0;
            for(auto& kv: P) {
                vector<double> v;
                evalMonomial(kv.first,v);
                size_t j = 0;
                for(auto c: v) gsl_matrix_set(M,j++,i,c);
                i++;
            }
        }
        LinMin::solve(y);

        size_t i=0;
        assert(x && x->size == P.size());
        for(auto& kv: P) kv.second = gsl_vector_get(x,i++);
        return P;
    }

protected:
    Poly P; ///< Solution polynomial form
};

#endif
