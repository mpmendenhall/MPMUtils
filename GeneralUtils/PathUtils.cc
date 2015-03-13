/* 
 * PathUtils.cc, part of the MPMUtils package.
 * Copyright (c) 2014 Michael P. Mendenhall
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

#include "PathUtils.hh"
#include "StringManip.hh"
#include "SMExcept.hh"

#include <dirent.h>
#include <algorithm>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <string.h>

bool fileExists(string f) {
    return !system(("test -r '" + f + "'").c_str());
}

bool dirExists(string d) {
    return !system(("test -d '" + d + "'").c_str());
}

void makePath(string p, bool forFile) {
    vector<string> pathels = split(p,"/");
    if(forFile && pathels.size())
        pathels.pop_back();
    if(!pathels.size())
        return;
    string thepath;
    if(p[0]=='/')
        thepath += "/";
    for(unsigned int i=0; i<pathels.size(); i++) {
        thepath += pathels[i] + "/";
        if(!dirExists(thepath)) {
            string cmd = "mkdir -p '"+thepath+"'";
            int err = system(cmd.c_str());
            if(err || !dirExists(thepath)) {
                SMExcept e("badPath");
                e.insert("pathName",thepath);
                e.insert("errnum",errno);
                e.insert("errname",strerror(errno));
                throw(e);
                throw("Failed to make path!");
            }
        }
    }
}

double fileAge(const string& fname) {
    if(!(fileExists(fname) || dirExists(fname)))
        return -1.;
    struct stat attrib;
    stat(fname.c_str(), &attrib);
    time_t timenow = time(NULL);
    return timenow - attrib.st_mtime;
}

vector<string> listdir(const string& dir, bool includeHidden, bool fullPath) {
    vector<string> dirs;
    dirent* entry;
    DIR* dp = opendir(dir.c_str());
    if (dp == NULL)
        return dirs;
    while((entry = readdir(dp)))
        if(includeHidden || entry->d_name[0] != '.')
            dirs.push_back((fullPath?dir+"/":"")+entry->d_name);
        closedir(dp);
    std::sort(dirs.begin(),dirs.end());
    return dirs;
}

string getEnvSafe(const string& v, const string& dflt) {
    const char* envv = getenv(v.c_str());
    if(!envv) {
        if(dflt == "FAIL_IF_MISSING") {
            SMExcept e("missingEnv");
            e.insert("var",v);
            throw(e);
            throw("Missing Environment Variable!");
        }
        return dflt;
    }
    return envv;
}

void combo_pdf(const vector<string>& namelist, const string& outname) {
    if(!namelist.size()) return;
    string cmd;
    if(namelist.size()==1) {
        cmd = "mv " + namelist[0] + " " + outname;
        int rc = system(cmd.c_str());
        if(rc) printf("%s: %i\n", cmd.c_str(), rc);
        return;
    }
    for(auto it = namelist.begin(); it != namelist.end(); it++)
        cmd += *it + " ";
    cmd = "pdfunite " + cmd + outname +"; rm " + cmd;
    int rc = system(cmd.c_str());
    if(rc) printf("%s: %i\n", cmd.c_str(), rc);
}

