/// \file DynamicPluginSaver.hh System for dynamically loading analyzer plugins by config file
/*
 * DynamicPluginSaver.hh, part of the MPMUtils package.
 * Copyright (c) 2016 Michael P. Mendenhall
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

#ifndef DYNAMICPLUGINSAVER_HH
#define DYNAMICPLUGINSAVER_HH

#include "PluginSaver.hh"
#include "libconfig.h++"
using namespace libconfig;

/// Template for PluginBuilder using config file
template <class Plug, class Base>
class ConfigPluginBuilder: public PluginBuilder {
public:
    /// Constructor
    ConfigPluginBuilder(const Setting& c, const string& rnm = ""): rename(rnm), cfg(c) { }
    /// Plugin construction... assumes proper constructor for Plug
    void makePlugin(SegmentSaver* pnt) override {
        assert(pnt);
        auto PBase = dynamic_cast<Base*>(pnt);
        assert(PBase);
        auto t0 = steady_clock::now();
        thePlugin = make_shared<Plug>(PBase, cfg);
        if(rename.size())thePlugin->rename(rename);
        thePlugin->tSetup += std::chrono::duration<double>(steady_clock::now()-t0).count();
        cfg.lookupValue("order", thePlugin->order);
    }
protected:
    string rename;          ///< plugin re-naming option
    const Setting& cfg;     ///< configuration info to pass to plugin
};

/// Base class for registering a plugin to global table
class PluginRegistrar {
public:
    /// generate appropriate builder class
    virtual shared_ptr<PluginBuilder> makeBuilder(Setting& c, const string& rename = "") const = 0;
};

class DynamicPluginSaver: public PluginSaver {
public:
    /// Constructor
    DynamicPluginSaver(OutputManager* pnt, const string& nm = "DynamicPluginSaver", const string& inflName = "");
    /// Configure from libconfig object, dynamically loading plugins
    virtual void Configure(const Setting& cfg);
    /// Configure, loading config by filename
    void LoadConfig(const string& fname);
    /// Configure, loading from input file
    void Reconfigure();

    /// global map of available plugins
    static map<string, PluginRegistrar*>& builderTable();

protected:
    TObjString* configstr;  ///< configuration file string
};

/// Compile-time registration of dynamically-loadable plugins
#define REGISTER_PLUGIN(NAME,BASE) \
class _##NAME##_Registrar: public PluginRegistrar { \
public: \
    _##NAME##_Registrar() { DynamicPluginSaver::builderTable().emplace(#NAME,this); } \
    shared_ptr<PluginBuilder> makeBuilder(Setting& c, const string& rename = "") const override { return make_shared<ConfigPluginBuilder<NAME,BASE>>(c,rename); } \
}; \
static _##NAME##_Registrar the_##NAME##_Registrar;

#endif
