/// \file Terminplot.hh "ASCII art" plotter
// -- Michael P. Mendenhall, 2021

#ifndef TERMINPLOT_HH
#define TERMINPLOT_HH

#include "Terminart.hh"

namespace Terminart {

    /// Base class plot axis
    class PlotAxis: public TermViewport {
    public:
        /// Constructor
        PlotAxis(bool horiz, int l): horizontal(horiz), length(l) { }

        /// polymorphic destructor
        virtual ~PlotAxis() { }

        /// auto-set range
        virtual void autorange(double /*x0*/, double /*x1*/) { }

        /// transform value to axis coordinates
        virtual double x2i(double x) const = 0;
        /// transform axis coordinates to value
        virtual double i2x(double i) const = 0;

        int getLength() const { return length; }

        /// horizontal or vertical orientation
        bool horizontal;

        /// fill viewport vector starting at p0 with given dimensions
        void getView(rowcol_t p0, pixelarray_t& v, const Compositor& C = Compositor::Cdefault) const override;
        /// get bounding box
        rectangle_t getBounds() const override { return {{0,0}, horizontal? rowcol_t{2,length} : rowcol_t{length,hpad+1}}; }

    protected:
        int length;                 ///< number of characters length
        int hpad = 4;               ///< horizontal padding for vertical axis
        vector<double> binedges;    ///< binning information

        /// calculate bin edges array
        void calc_binedges();
    };

    /// Linear axis
    class LinAxis: public PlotAxis {
    public:
        /// Constructor
        LinAxis(bool horiz, double _x0, double _x1, int l): PlotAxis(horiz, l), x0(_x0), x1(_x1) { }

        /// auto-set range
        void autorange(double _x0, double _x1) override { x0 = _x0; x1 = _x1; }

        /// transform value to axis coordinates
        double x2i(double x) const override { return (x - x0)*(length-1)/(x1 - x0); }
        /// transform axis coordinates to value
        double i2x(double i) const override { return x0 + i*(x1 - x0)/(length-1); }

        double x0;
        double x1;
    };

    /// x-y points graph/scatterplot
    class TermGraph: public TermViewport, public vector<pair<double,double>> {
    public:
        /// point on graph
        typedef pair<double,double> xypt_t;
        /// Inherit constructors
        using vector::vector;
        /// Destructor
        ~TermGraph() { delete Ax; delete Ay; }

        PlotAxis* Ax = nullptr; ///< x axis
        PlotAxis* Ay = nullptr; ///< y axis
        string symbs{",.~'^"};   ///< plotting symbols, interpolating low to high

        /// auto-range axes
        virtual void autorange();
        /// initialize nullptr axes
        virtual void initAxes();
        /// print as table
        void displayTable() const;

        /// get bounding box
        rectangle_t getBounds() const override;
        /// fill viewport vector starting at p0 with given dimensions
        void getView(rowcol_t p0, pixelarray_t& v, const Compositor& C = Compositor::Cdefault) const override;
    };

    /// Histogram
    class TermHisto: public TermViewport {
    public:
        /// Constructor
        TermHisto() {}
        /// Destructor
        ~TermHisto() { delete Ay; }

        /// Fill x contents
        void Fill(double x, double w = 1);

        PlotAxis* Ay = nullptr;     ///< length axis

        /// fill viewport vector starting at p0 with given dimensions
        void getView(rowcol_t p0, pixelarray_t& v, const Compositor& C = Compositor::Cdefault) const override;

    protected:
        //const PlotAxis& Ax;       ///< binning axis
        vector<double> binconts;    ///< bin contents
    };


}

#endif
