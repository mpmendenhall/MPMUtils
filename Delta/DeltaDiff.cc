/// @file DeltaDiff.cc

#include "DeltaDiff.hh"
#include <stdio.h>
#include <array>

bool DeltaDiff::_compare() {
    auto cmd = "diff '" + fref + "' '" + fcomp + "'";
    auto p = popen(cmd.c_str(), "r");
    if(!p) throw std::runtime_error("Failed popen for diff");

    string res;
    while(!std::feof(p)) {
        std::array<char, 256> buffer;
        auto bytes = std::fread(buffer.data(), 1, buffer.size(), p);
        res.append(buffer.data(), bytes);
    }

    auto rc = pclose(p);
    printf("%s\n%s\nexited %i\n", cmd.c_str(), res.c_str(), rc);
    return !rc;
}
