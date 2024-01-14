/// @file LinkDef.h ROOT linkdef for generating ROOT object library

#ifdef __CLING__

#pragma link C++ class CumulativeData+;
#pragma link C++ class TCumulative+;
#pragma link C++ class TCumulativeMap<Int_t, Double_t>+;
#pragma link C++ class TCumulativeMap<string, Double_t>+;
#pragma link C++ class TDynamicHistogram+;

#endif
