/// @file PathUtils.cc
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

#if __cpp_lib_filesystem >= 201703L
#include <filesystem>

bool fileExists(const string& f) {
    return std::filesystem::is_regular_file(f);
}

bool dirExists(const string& d) {
    return std::filesystem::is_directory(d);
}

void makePath(const string& p, bool forFile) {
    auto pathels = split(p,"/");
    if(forFile && pathels.size()) pathels.pop_back();
    if(!pathels.size()) return;

    string thepath = p.front() == '/'? "/" : "";
    for(const auto& e: pathels) thepath += e + "/";
    std::filesystem::create_directories(thepath);
    if(!dirExists(thepath)) throw std::runtime_error("Unable to make path '"+thepath+"'");
}

#else

bool fileExists(const string& f) {
    return !system(("test -r '" + f + "'").c_str());
}

bool dirExists(const string& d) {
    return !system(("test -d '" + d + "'").c_str());
}


void makePath(const string& p, bool forFile) {
    auto pathels = split(p,"/");
    if(forFile && pathels.size()) pathels.pop_back();
    if(!pathels.size()) return;

    string thepath;
    if(p.front() == '/') thepath += "/";
    for(const auto& e: pathels) {
        thepath += e + "/";
        if(!dirExists(thepath)) {
            string cmd = "mkdir -p '"+thepath+"'";
            int err = system(cmd.c_str());
            if(err || !dirExists(thepath)) throw std::runtime_error("Unable to make path '"+thepath+"'");
        }
    }
}

#endif


double fileAge(const string& fname) {
    if(!(fileExists(fname) || dirExists(fname))) return -1.;
    struct stat attrib;
    stat(fname.c_str(), &attrib);
    return time(nullptr) - attrib.st_mtime;
}

vector<string> listdir(const string& dir, bool includeHidden, bool fullPath) {
    vector<string> dirs;
    DIR* dp = opendir(dir.c_str());
    if (dp == nullptr) return dirs;

    dirent* entry;
    while((entry = readdir(dp)))
        if(includeHidden || entry->d_name[0] != '.')
            dirs.push_back((fullPath?dir+"/":"")+entry->d_name);
    closedir(dp);

    std::sort(dirs.begin(),dirs.end());
    return dirs;
}

bool syscmd(const string& cmd, bool failOK) {
    auto ret = system(cmd.c_str());
    if(!ret) return true;
    if(failOK) return false;
    throw std::runtime_error("system(" + cmd +") failed");
}
