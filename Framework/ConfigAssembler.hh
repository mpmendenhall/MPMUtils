/// \file ConfigAssembler.hh Helper pattern for dynamic object instantiation
// -- Michael P. Mendenhall, LLNL 2019

////////////////////////////////
////////////////////////////////
////////////////////////////////
/* Usage, parent and child of same class:

class Foo: public ConfigAssembler<Foo> {
public:
    /// Constructor
    Foo(const Setting& S) { for(auto& c: S["children"]) children.emplace_back(assemble(c.lookup("class"), c, this)); }
protected:
    /// Child-class constructor
    Foo() { }
};

class Bar: public Foo {
public:
    /// Required constructor signature to be REGISTER'd
    Bar(const Setting&, Foo*) { }
};

/// Helper
#define REGISTER_FOO(NAME) static ObjectFactory<Foo, NAME, const Setting&, Foo*> the_##NAME##_FooFactory(#NAME);
/// Helper, with alternate lookup name
#define REGISTER_FOO(NAME,CFGNAME) static ObjectFactory<Foo, NAME, const Setting&, Foo*> the_##NAME##_FooFactory(#CFGNAME);

/// construct from "class: Bar"
REGISTER_FOO(Bar)

// Foo(cfg) for top-level Foo(...)
// Foo::assemble(cfg) for top-level Bar(...)

*/


////////////////////////////////
////////////////////////////////
////////////////////////////////
/* Usage, with separate "parent" and "plugin" child classes:

/// Base for  constructable "plugins"
class Bar { };

/// "Parent" container for plugins
class Foo: public ConfigAssembler<Bar> {
public:
    /// Constructor
    Foo(const Setting& S) { for(auto& c: S["children"]) children.emplace_back(assemble(c.lookup("class"), c, *this)); }
}

/// A "plugin"
class Baz: public Bar {
public:
    /// Required constructor signature to be REGISTER'd
    Baz(const Setting&, Foo&) { }
};

/// Helper
#define REGISTER_BAR(NAME) static ObjectFactory<Bar, NAME, const Setting&, Foo&> the_##NAME##_BarFactory(#NAME);

REGISTER_BAR(Baz)

*/


#ifndef CONFIGASSEMBLER_HH
#define CONFIGASSEMBLER_HH

#include "ObjectFactory.hh"
#include <vector>
using std::vector;

/// Configuration assembler base class for generating 'C(...)'
template<typename C>
class ConfigAssembler {
public:
    /// base class being generated
    typedef C base_t;

    /// Destructor
    virtual ~ConfigAssembler() { for(auto c: children) delete c; }

    /// Construct object of base_t type
    template<typename... Args>
    static base_t* assemble(const string& cname, Args&&... a) {
        return BaseFactory<base_t>::construct(cname, std::forward<Args>(a)...);
    }

protected:
    /// Assemble and add to children
    template<typename... Args>
    base_t& assembleChild(const string& cname, Args&&... a) {
        children.push_back(assemble(cname, std::forward<Args>(a)...));
        return *children.back();
    }

    /// Child objects
    vector<C*> children;
};


#endif
