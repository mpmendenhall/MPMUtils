#!/usr/bin/python3
"""@package AtomsDB
Atom information resources --- currently dumb elements list"""

class ElementNames:
    """Element name/symbol lookup table"""
    def __init__(self, fname="ElementsList.txt"):
        # (Z,symb,name)
        self.dat = [(int(x[0]),x[1],x[2]) for x in [l.split() for l in open(fname).readlines()] if len(x)==3]
        self.byNum = dict([(e[0],e) for e in self.dat])
        self.bySymb = dict([(e[1].lower(),e) for e in self.dat])
        self.byName = dict([(e[2].lower(),e) for e in self.dat])
        
    def elNum(self, s):
        """Element Z by name or symbol, case-insensitive"""
        s = s.lower()
        n = self.bySymb.get(s,None)
        n = n if n else self.byName.get(s,None)
        return n[0] if n else None
    
    def elSym(self,Z):
        """Element symbol by Z"""
        return self.byNum.get(Z,(None,None,None))[1]