#!/usr/bin/python3

# package for parsing Fortran-formatted text files
# pip install --user fortranformat
import fortranformat as ff

def get_MATMFMT_lines(lines, matmfmt):
    """Select lines by string for MAT[MF[MT]]"""
    return [l for l in lines if l[66:66+len(matmfmt)] == matmfmt]

class ENDF_Record:
    """Generic record (line in file) parder"""
    record_reader = ff.FortranRecordReader('(A66,I4,I2,I3,I5)')
    
    def __init__(self, line):
        self.text, self.MAT, self.MF, self.MT, self.NS = self.record_reader.read(line)
    
    def uid(self):
        """Record unique identifier: material, file, section, line number"""
        return (self.MAT, self.MF, self.MT, self.NS)
    
    def __repr__(self):
        return "[%i:%i.%i.%i"%self.uid() +" %s]"%self.text
    
class ENDF_CONT_Record(ENDF_Record):
    """CONTROL record, generic base type for many entries"""
    cont_reader = ff.FortranRecordReader('(2E11.0,4I11)')
    
    def __init__(self, line):
        ENDF_Record.__init__(self,line)
        self.C1, self.C2, self.L1, self.L2, self.N1, self.N2 = self.cont_reader.read(self.text)

    def parse(line, withUID = False):
        l = ENDF_Record(line)
        ldat = ENDF_CONT_Record.cont_reader.read(l.text)
        if withUID: return ldat + l.uid()
        return ldat

# HEAD = CONT, with C1,C2 -> ZA,AWR

def skip_to_file_end(iterlines):
    """Iterate through lines until end-of-file entry"""
    nlines = 0
    for l in iterlines:
        r = ENDF_Record(l)
        nlines += 1
        if r.MF == 0: break
    return nlines

def skip_to_section_end(iterlines):
    """Iterate through lines until end-of-section entry"""
    nlines = 0
    for l in iterlines:
        r = ENDF_Record(l)
        nlines += 1
        if r.MT == 0 and r.MF: break
    return nlines

class ENDF_List:
    """List data format"""
    list_reader = ff.FortranRecordReader('(6E11.0)')
    
    def __init__(self,iterlines):
        self.header = ENDF_CONT_Record(next(iterlines))
        self.NPL = self.header.N1 # number of entries
        dat = [self.list_reader.read(ENDF_Record(next(iterlines)).text) for i in range((self.NPL+5)//6)]
        self.data = [j for i in dat for j in i][:self.NPL]

class ENDF_Tab1:
    """1-dimensional table data format"""
    range_reader = ff.FortranRecordReader('(6I11)')
    pair_reader = ff.FortranRecordReader('(6E11.0)')
    
    def __init__(self, iterlines):
        self.header = ENDF_CONT_Record(next(iterlines))
        
        rdat = [self.range_reader.read(ENDF_Record(next(iterlines)).text) for i in range((self.header.N1+2)//3)]
        rdat = [j for i in rdat for j in i][:2*self.header.N1]
        self.ranges = list(zip(*[iter(rdat)]*2))
        
        pdat = [self.pair_reader.read(ENDF_Record(next(iterlines)).text) for i in range((self.header.N2+2)//3)]
        pdat = [j for i in pdat for j in i][:2*self.header.N2]
        self.xy = list(zip(*[iter(pdat)]*2))

    def __repr__(self):
        i0 = 0
        s = ""
        for r in self.ranges:
            s += "-- interpolation %i --\n"%r[1]
            for i in range(r[0]): s += "%g\t%g\n"%self.xy[i+i0]
            i0 += r[0]
        return s + "------------------"

class ENDF_File2_Range:
    
    LRUdescrip = {0: "radius only", 1: "unresolved", 2: "resolved" }
    LRFdescrip = {0: ""}
    
    def __init__(self, iterlines, LFW):
        self.LFW = LFW
        
        # EL    energy lower limit
        # EH    energy upper limit
        # LRU   resolved or unresolved parameters
        # LRF   representation of resonances
        # NRO   energy dependence of radius: 0: none; 1: from table
        # NAPS  interpretation of 2 radii
        self.EL, self.EH, self.LRU, self.LRF, self.NRO, self.NAPS = ENDF_CONT_Record.parse(next(iterlines))
            
        if self.LRU == 0: # only scattering radius given
            self.load_SPI_line(iterlines)
            assert self.NLS == 0
            
        elif self.LRU == 1: # resolved resonance parameters
            self.LRFdescrip = {1: "SLBW", 2: "MLBW", 3: "Reich-Moore", 4: "Adler-Adler", 7: "R-Matrix Limited"}
        
            if self.LRF in (1,2,3):
                self.ertable = ENDF_Tab1(iterlines) if self.NRO else None # energy radius table
                self.load_SPI_line(iterlines)
                # LAD   whether to use parameters to compute angular distributions (0/1)
                self.LAD = self.hdr_L1
                self.llist = ENDF_List(iterlines)   # data for each l
            
            else:
                print("LRF = %i unimplemented"%self.LRF)
                raise NotImplementedError
        
        elif self.LRU == 2: # unresolved resonance parameters
            self.LRFdescrip = {1: "energy-dependent widths", 2: "energy-dependent parameters"}
            
            #if self.LRF == 1: # only fission widths given with energy dependence
            #    pass
            
            if self.LRF == 2: # all parameters may be energy dependent
                
                self.load_SPI_line(iterlines)
                # LSSF: interpretation of File 3 cross sections
                self.LSSF = self.hdr_L1
                assert self.hdr_L2 == 0 and self.hdr_N2 == 0
                self.ljdat = []
                for l in range(self.NLS):
                    self.ljdat.append([])
                    AWRI, u1, L, u2, NJS, u3 = ENDF_CONT_Record.parse(next(iterlines))
                    for j in range(NJS):
                        self.ljdat[-1].append(ENDF_List(iterlines))
            
            else:
                print("LRF = %i unimplemented"%self.LRF)
                raise NotImplementedError
        else:
            print("LRU = %i unimplemented"%self.LRU)
            raise NotImplementedError

    def load_SPI_line(self,iterlines):
        # SPI   spin I of target nucleus
        # AP    scattering radius in 10^-12 cm
        # NLS   number of L-values
        self.SPI, self.AP, self.hdr_L1, self.hdr_L2, self.NLS, self.hdr_N2 = ENDF_CONT_Record.parse(next(iterlines))
    
    def __repr__(self): 
        s = "[Resonance range %g -- %g eV: %s, %s]"
        return s%(self.EL, self.EH, self.LRUdescrip[self.LRU], self.LRFdescrip.get(self.LRF, "LRF=%i"%self.LRF))
    
    
class ENDF_File2:
    """File 2 'Resonance Parameters'"""
    def __init__(self, iterlines):
        # ZA    
        # AWR   
        # NIS   number of isotopes in material
        self.ZA, self.AWR, u1, u2, self.NIS, u3 = ENDF_CONT_Record.parse(next(iterlines))
        # ZAI   isotope Z,A
        # ABN   isotope abundance in material, number fraction
        # LFW   average fission widths given? 0: no, 1: yes
        # NER   number of ranges
        self.ZAI, self.ABN, u1, self.LFW, self.NER, u2 = ENDF_CONT_Record.parse(next(iterlines))
        
        self.ranges = [ENDF_File2_Range(iterlines, self.LFW) for ri in range(self.NER)]
        assert skip_to_section_end(iterlines) == 1
        skip_to_file_end(iterlines)
        #assert skip_to_file_end(iterlines) == 1
        
    def __repr__(self):
        s = "[File 2: %i resonance parameter ranges]"%len(self.ranges)
        for r in self.ranges: s += "\n\t%s"%str(r)
        return s

class ENDF_File3:
    """File 3 'reaction cross sections'"""
    def __init__(self, iterlines):
        self.sections = []
        self.tables = []
        while(1):
            sechead = ENDF_CONT_Record(next(iterlines))
            if sechead.MF == 0: # end of file
                self.idinfo = sechead
                break
            tbl = ENDF_Tab1(iterlines)
            self.tables.append(tbl)
            self.sections.append({"section":sechead.MT, "ZA":sechead.C1, "AWR":sechead.C2, "QM":tbl.header.C1, "QI":tbl.header.C2, "LR":tbl.header.L2})
            assert skip_to_section_end(iterlines) == 1
        
    def __repr__(self):
        s = "[File 3 with %i sections]"%len(self.sections)
        for t in self.sections: s += "\n\t Section %(section)i: ZA=%(ZA)g, AWR=%(AWR)g, QM=%(QM)g, QI=%(QI)g, LR=%(LR)i"%t
        return s
