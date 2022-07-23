/// \file TestGammaScatter.cc Gamma scattering spectra approximations test plots
// -- Michael P. Mendenhall, LLNL 2022

#include "ConfigFactory.hh"
#include "GlobalArgs.hh"
#include "GammaMultiScatter.hh"
#include "GraphUtils.hh"
#include <stdio.h>
#include <TAxis.h>
#include <TPad.h>

REGISTER_EXECLET(TestGammaScatter) {

    double E = 2;
    optionalGlobalArg("E", E, "Incident gamma energy [MeV]");
    const double Em = E/m_e;

    double d = 5;
    optionalGlobalArg("d", d, "Material thickness [cm]");

    int npts = 100;
    optionalGlobalArg("npts", npts, "number of calculation points");

    double PE_per_MeV = 400;
    optionalGlobalArg("PE", PE_per_MeV, "PE/MeV energy resolution");
    double Erange = E + 3 * sqrt(E/PE_per_MeV);

    bool logy = false;
    optionalGlobalArg("logy", logy, "plot with log y axis");

    // electron density
    int e_per_molecule = 10;    // electrons per molecule unit
    double molar_mass = 18;     // molar mass (g)
    double mat_dens = 1;        // material density (g/cm^3)
    double eDens = mat_dens * e_per_molecule / molar_mass;  // electrons mol / cm^3

    printf("\nClassical electron radius: r_e = %g fm => pi r_e^2 = %g barn\n", r_e, cx_e);

    GammaScatterSteps GSS(E, d * eDens, 6, npts);

    double fmin = gamma_escatter_fmin(Em);
    double Ecompt = E - GSS.steps.back().Emin;
    printf("At incident E_gamma = %g MeV,\n", E);
    printf("\tf_min = %g (E_min = %g MeV)\n", fmin, fmin*E);
    printf("\tCompton edge at E = %g MeV\n", Ecompt);

    double s_tot = KN_total_xs(Em);
    printf("\tsigma_tot = %g barn\n", s_tot);

    double lambda = 1. / (N_A * eDens * s_tot * 1e-24);
    printf("\tat electron density (%g g / cm^3) * %ie / (%gg / mol)  = %.2f mol/cm^3,\n",
           mat_dens, e_per_molecule, molar_mass, eDens);
    printf("\t\tinteraction lambda = %.2f cm; %.1f%% scatter within %g cm\n\n",
           lambda, 100*GSS.Scatter_0, d);

    // calculate scattering steps
    vector<TGraph> gS;
    for(int i = 0; i < 100; ++i) {
        if(i) GSS.scatter_step();
        auto& S = GSS.steps.back();
        TGraph g = GSS.Egamma_to_Ee(S.Escape);
        g.GetYaxis()->SetTitle("Electron scattering [/gamma/MeV]");
        g.SetPoint(npts, E - GSS.steps.back().Emin, 0);
        gS.push_back(g);
        printf("* Scattering step %i: %.2f%% of gammas (E > %.2f MeV) remaining to re-scatter, %.2f%% fully captured\n",
               i, 100*S.nScatter, S.Emin, 100*GSS.FullCapt);
        if(S.nScatter < 1e-3 * GSS.Scatter_0) break;
    }

    // escaping and degraded spectrum

    GSS.calcRescatter(GSS);

    auto gEs = GSS.steps.back().EscapeSum;
    for(int i = GSS.steps.size()-2; i >= 0; --i)
        gEs = sumGraphs(gEs, GSS.steps[i].EscapeSum);
    gEs.SetMinimum(logy? 1e-4 : 0);
    gEs.GetXaxis()->SetRangeUser(0, E);
    gEs.Draw("AL");

    int i = 0;
    for(auto& S: GSS.steps) {
        auto& gE = S.Escape;
        gE.SetLineStyle(i + 2);
        gE.Draw("L");
        ++i;
    }
    GSS.bComptons.SetLineColor(2);
    GSS.bComptons.Draw("L");

    gPad->Print("Escaped.pdf");

    GSS.gInteract.Draw("AL");
    GSS.gInteract.GetXaxis()->SetRangeUser(0,E);
    gPad->Print("ScatterFraction.pdf");

    GSS.gCx.Draw("AL");
    GSS.gPE.SetLineColor(2);
    GSS.gPE.Draw("L");
    GSS.gCx.GetXaxis()->SetRangeUser(0,E);
    gPad->Print("TotalScatter.pdf");

    auto gCompton = GSS.eSpectrum();
    gCompton.SetPoint(gCompton.GetN(), Erange, 0);

    auto gSmear = GSS.eSpectrum(PE_per_MeV);
    gSmear.SetLineColor(2);

    gCompton.Draw("AL");
    gCompton.GetXaxis()->SetRangeUser(0, Erange);
    if(logy) gCompton.SetMinimum(1e-3);
    double smax = gSmear.Eval(E);
    double cmax = *std::max_element(gCompton.GetY(), gCompton.GetY() + gCompton.GetN());
    if(smax > cmax) gCompton.SetMaximum(1.1*smax);
    gPad->SetLogy(logy);
    i = 0;
    for(auto& g: gS) {
        g.SetLineStyle(i+2);
        g.Draw("L");
        ++i;
    }
    gSmear.Draw("C");
    gPad->Print("ComptonScatter.pdf"); // electron spectrum, with smeared overlay

    i = 0;
    for(auto& S: GSS.steps) {
        auto& gI = S.Incident;
        gI.SetPoint(gI.GetN(), logy? 0.03 : 0., 0);
        gI.SetPoint(gI.GetN(), 0.9999*S.Emin, 0);
        gI.Sort();
        if(logy) gI.SetMinimum(1e-4);
        gI.SetLineStyle(i+1);
        gI.Draw(i? "L" : "AL");
        gI.GetXaxis()->SetRangeUser(0,E);
        ++i;
    }
    gPad->SetLogx(true);
    gPad->Print("Incident.pdf"); // indcident gamma specrtum at each stage
}
