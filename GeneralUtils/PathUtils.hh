/// \file PathUtils.hh file, path, and environment variable utilities
/*
 * PathUtils.hh, part of the MPMUtils package.
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

#ifndef PATHUTILS_HH
#define PATHUTILS_HH

#include <string>
#include <vector>

using std::string;
using std::vector;

/// check if file exists
bool fileExists(string f);
/// check if directory exists
bool dirExists(string d);
/// make sure the specified path exists (if not, create it); optionally, exclude last item on path (filename)
void makePath(string p, bool forFile = false);
/// list directory contents
vector<string> listdir(const string& dir, bool includeHidden = false, bool fullPath = false);
/// get time since last file modification (s)
double fileAge(const string& fname);
/// get environment variable, with default or fail if missing
string getEnvSafe(const string& v, const string& dflt = "FAIL_IF_MISSING");
/// Combine list of PDF files into one multi-page document
void combo_pdf(const vector<string>& namelist, const string& outname);

#endif
