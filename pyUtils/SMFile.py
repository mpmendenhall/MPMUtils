## @file SMFile.py key-value string map and file; compatible with SMFile.hh c++ output format

import os

class KVMap:
    """multimap (dictionary with lists of values for each key)"""
    
    def __init__(self,str=None):
        
        if isinstance(str,KVMap):
            self.dat = dict(str.dat)
        else:
            self.dat = {}
            if str is not None:
                for p in [w.split('=') for w in str.split('\t') ]:
                    if len(p) != 2:
                        continue
                    self.insert(p[0].strip(),p[1].strip())
    
    def insert(self,key,value):
        """insert new entry for given key"""
        self.dat[key] = self.dat.get(key,[]) + [value,]
    
    def getFirst(self,k,default=None):
        """get first value for given key"""
        v = self.dat.get(k,[])
        if not len(v):
            return default
        return v[0]
    
    def getFirstF(self,k,default=None):
        """get first value for given key as a float"""
        if default is not None:
            return float(self.getFirst(k,str(default)))
        else:
            s = self.getFirst(k,None)
            if s is None:
                return None
            return float(s)
    
    def getFirstV(self,k,default=None):
        """get first value as a vector of floats"""
        s = self.getFirst(k,None)
        if s is None:
            return default
        return [float(c) for c in s.split(",")]

    def loadFloats(self,names):
        """set attributes for float values"""
        for nm in names:
            self.__dict__[nm] = self.getFirstF(nm)
    
    def loadStrings(self,names):
        """set attributes for string values"""
        for nm in names:
            self.__dict__[nm] = self.getFirst(nm)

    def matches(self,k,v):
        """whether this has a matching key:value pair"""
        return v in self.dat.get(k,[])
    
    def matchesMany(self,kdict):
        """whether this matches a set of key:value pairs"""
        for k in kdict:
            if not self.matches(k,kdict[k]):
                return False
        return True
        
    def toString(self):
        """convert to string"""
        outstr = ""
        kk = list(self.dat.keys())
        kk.sort()
        for k in kk:
            for i in self.dat[k]:
                outstr += str(k)+" = "+str(i)+"\t"
        return outstr[:-1]

class SMFile(KVMap):
    """a KVMap< string, KVMap<string,string> >"""
    
    def __init__(self,fname=None):
        self.dat = {}
        self.fname = fname
        
        if fname is not None:
            if not os.path.exists(fname):
                print("No such file",fname)
                return
            for l in [z.split(':') for z in open(fname).readlines()]:
                if len(l) < 2:
                    continue
                self.dat[l[0]] = self.dat.get(l[0],[]) + [KVMap(l[1]),]
    
    def getItem(self,k1,k2,default=None):
        """get first value for key:key"""
        m = self.getFirst(k1,None)
        if not m:
            return default
        return m.getFirst(k2,default)
    
    def getItemF(self,k1,k2,default=None):
        """get first value for key:key as float"""
        m = self.getFirst(k1,None)
        if not m:
            return default
        return m.getFirstF(k2,default)
        
    def getMatching(self,key,value):
        """get KVMap with matching subkey:value pairs"""
        Q = SMFile()
        for k in self.dat.keys():
            for m in self.dat[k]:
                if m.matches(key,value):
                    Q.insert(k,m)
        return Q
        
    def getMatchingMany(self,requirements):
        """get KVMap with matching subkeys:values pairs"""
        Q = SMFile()
        for k in self.dat.keys():
            for m in self.dat[k]:
                if m.matchesMany(requirements):
                    Q.insert(k,m)
        return Q
    
    def toString(self):
        """convert to string"""
        outstr = ""
        dkeys = list(self.dat.keys())
        dkeys.sort()
        for k in dkeys:
            for i in self.dat[k]:
                outstr += str(k)+":\t\t"+i.toString()+"\n"
        return outstr[:-1]
