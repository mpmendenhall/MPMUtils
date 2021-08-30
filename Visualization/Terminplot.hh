/// \file Terminplot.hh "ASCII art" plotter
// -- Michael P. Mendenhall, 2021

#ifndef TERMINPLOT_HH
#define TERMINPLOT_HH

#include "Terminart.hh"

namespace Terminart {

    /// Base class plot axis
    class PlotAxis {
    public:
        /// Constructor
        PlotAxis(int l = 10): length(l) { }

        /// polymorphic destructor
        virtual ~PlotAxis() { }

        /// auto-set range
        virtual void autorange(double /*x0*/, double /*x1*/) { }

        /// transform value to axis coordinates
        virtual double x2i(double x) const = 0;
        /// transform axis coordinates to value
        virtual double i2x(double i) const = 0;

        int getLength() const { return length; }

    protected:
        int length;                 ///< number of characters length
        vector<double> binedges;    ///< binning information

        /// calculate bin edges array
        void calc_binedges();
    };

    /// Linear axis
    class LinAxis: public PlotAxis {
        /// Constructor
        LinAxis(double _x0, double _x1, int l): PlotAxis(l), x0(_x0), x1(_x1) { }

        /// auto-set range
        void autorange(double _x0, double _x1) override { x0 = _x0; x1 = _x1; }

        /// transform value to axis coordinates
        double x2i(double x) const override { return (x - x0)*length/(x1 - x0); }
        /// transform axis coordinates to value
        double i2x(double i) const override { return x0 + (i/length)*(x1 - x0); }

        double x0;
        double x1;
    };

    /// Horizontal axis display
    class HorizAxisView: public TermViewport {
    public:
        /// Constructor
        HorizAxisView(const PlotAxis& P): Ax(P) { }

        /// fill viewport vector starting at p0 with given dimensions
        void getView(rowcol_t p0, pixelarray_t& v, const Compositor& C = Compositor::Cdefault) const override;

        const PlotAxis& Ax; ///< axis to display
    };

    /// Vertical axis display
    class VertAxisView: public TermViewport {
    public:
        /// Constructor
        VertAxisView(const PlotAxis& P): Ax(P) { }

        /// fill viewport vector starting at p0 with given dimensions
        void getView(rowcol_t p0, pixelarray_t& v, const Compositor& C = Compositor::Cdefault) const override;

        const PlotAxis& Ax; ///< axis to display
    };

    /// Histogram
    class TermHisto: public TermViewport {
    public:
        /// Constructor
        TermHisto() {}
        /// Destructor
        ~TermHisto() { if(Ay) delete Ay; }

        /// Fill x contents
        void Fill(double x, double w = 1);

        PlotAxis* Ay = nullptr;     ///< length axis

        /// fill viewport vector starting at p0 with given dimensions
        void getView(rowcol_t p0, pixelarray_t& v, const Compositor& C = Compositor::Cdefault) const override;

    protected:
        //const PlotAxis& Ax;         ///< binning axis
        vector<double> binconts;    ///< bin contents
    };


}

#endif
