/// \file PathUtils.cc
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

#include <stdexcept>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <set>
using std::set;

bool fileExists(const string& f) {
    return !system(("test -r '" + f + "'").c_str());
}

bool dirExists(const string& d) {
    return !system(("test -d '" + d + "'").c_str());
}

void makePath(const string& p, bool forFile) {
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

        static set<string> madepaths;
        if(madepaths.count(thepath)) continue;
        madepaths.insert(thepath);
        if(!dirExists(thepath)) {
            string cmd = "mkdir -p '"+thepath+"'";
            int err = system(cmd.c_str());
            if(err || !dirExists(thepath)) throw std::runtime_error("Unable to make path '"+thepath+"'");
        }
    }
}

double fileAge(const string& fname) {
    if(!(fileExists(fname) || dirExists(fname)))
        return -1.;
    struct stat attrib;
    stat(fname.c_str(), &attrib);
    time_t timenow = time(nullptr);
    return timenow - attrib.st_mtime;
}

vector<string> listdir(const string& dir, bool includeHidden, bool fullPath) {
    vector<string> dirs;
    dirent* entry;
    DIR* dp = opendir(dir.c_str());
    if (dp == nullptr)
        return dirs;
    while((entry = readdir(dp)))
        if(includeHidden || entry->d_name[0] != '.')
            dirs.push_back((fullPath?dir+"/":"")+entry->d_name);
    closedir(dp);
    std::sort(dirs.begin(),dirs.end());
    return dirs;
}

void combo_pdf(const vector<string>& namelist, const string& outname) {
    if(!namelist.size()) return;
    makePath(outname, true);
    if(namelist.size()==1) {
        string cmd = "mv " + namelist[0] + " " + outname;
        int rc = system(cmd.c_str());
        if(rc) printf("%s: %i\n", cmd.c_str(), rc);
        return;
    }
    string cmd = join(namelist," "); // file list
    cmd = "if command -v pdfunite; then pdfunite " + cmd + " " + outname +"; else pdftk " + cmd + " cat output " + outname + "; fi; rm " + cmd;
    int rc = system(cmd.c_str());
    if(rc) printf("%s: %i\n", cmd.c_str(), rc);
}

