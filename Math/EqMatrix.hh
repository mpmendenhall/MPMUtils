/// @file EqMatrix.hh Helper for solving a system of linear equations with RHS uncertainties
// -- Michael P. Mendenhall, LLNL 2024

#include <vector>
using std::vector;
using std::pair;
#include <stdlib.h>
#include <map>
using std::map;

/// Helper for solving an (overdetermined) system of linear equations with RHS uncertainties
class EqMatrix {
public:
    /// add data point for i-j = x +- dx
    void addDiff(int i, int j, double x, double dx);
    /// add data point for i+j = x +- dx
    void addSum(int i, int j, double x, double dx);
    /// calculate segment offsets
    void calculate(bool doErrs = false);

    /// one linear equation c1*v1 + c2*v2 + ... = x +- dx
    struct lineq_t {
        explicit lineq_t(double x0 = 0, double dx0 = 1): x(x0), dx(dx0) { }
        vector<pair<int, double>> coeffs;
        double x;
        double dx;
        /// print to stdout
        void display() const;
    };

    vector<lineq_t> dpts;       ///< system of linear equations

    /// solved value
    struct solution_t {
        int var = 0;
        double x = 0;
        double dx = 0;
    };

    double sumw;                ///< sum of weights
    double rms;                 ///< root-mean-square deviation of result

    vector<int> indices;        ///< internal index to variable number
    map<int,int> to_idx;        ///< variable number to internal index
    vector<solution_t> soln;    ///< calculated solution

protected:
    /// generate indices
    void index_vars();
};
