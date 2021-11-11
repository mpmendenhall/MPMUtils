/// \file testMCTAL.cc Test of MCNP MCTAL parser

#include "ConfigFactory.hh"
#include "MCTAL_File.hh"
#include "GlobalArgs.hh"
#include <fstream>
#include <stdio.h>

REGISTER_EXECLET(testMCTAL) {
    std::ifstream i(requiredGlobalArg("f", "MCTAL file"));
    MCTAL_File MF(i);
    MF.display();
}
