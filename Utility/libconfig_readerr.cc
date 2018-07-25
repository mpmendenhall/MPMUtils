#include "libconfig_readerr.hh"
#include <stdio.h>

void readConfigFile(Config& cfg, const string& cfgfile, bool autoconvert) {
    cfg.setAutoConvert(autoconvert);
    try { cfg.readFile(cfgfile.c_str()); }
    catch(ParseException& e) {
        printf("\n\nConfiguration file syntax error!\n");
        //auto ename = e.getError();
        //if(ename) printf("\t%s\n", ename);
        auto efile = e.getFile();
        if(efile) printf("In file: '%s' ", efile);
        printf("Line %i\n\n", e.getLine());
        fflush(stdout);
        throw e;
    } catch(FileIOException& e) {
        printf("File I/O error loading config '%s'; check file exists and is readable!\n", cfgfile.c_str());
        fflush(stdout);
        throw e;
    }
}
