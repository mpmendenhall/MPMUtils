from ENDF_Base_Reader import *

# for f in ~/Data/ENDF-B-VII.1/atomic_relax/atom-*; do ./ENDF6_DB.py --load $f; done

class ENDF_File28_Transition:
    """File 28 transition between subshells"""
    def __init__(self, l):
        self.SUBJ = int(l[0])           # secondary subshell number
        self.SUBK = int(l[1])           # tertiary subshell number; 0 for radiative transitions
        self.ETR  = l[2]                # energy of transition (eV)
        self.FTR  = l[3]                # fractional probability of transition

    def __repr__(self):
        return "\t-> %s (%s), %g eV\t%g%%"%(ENDF_File28_List.ssname(self.SUBJ), ENDF_File28_List.ssname(self.SUBK), self.ETR, 100*self.FTR)

class ENDF_File28_List(ENDF_List):
    """File 28 subsection describing relaxation transisitions from one subshell"""
    ssnames = {0:'-', 1: "K", 2: "L1", 3:"L2", 4:"L3", 5:"M1", 6:"M2", 7:"M3"}
    def ssname(i): return ENDF_File28_List.ssnames.get(i,'#'+str(i))

    def __init__(self, iterlines):
        super().__init__(iterlines)
        self.rnm("C1","SUBI"); self.SUBI = int(self.SUBI)   # subshell enumeration, 1,2,3 = K,L1,L2,...
        self.EBI  = self.data[0]        # subshell binding energy
        self.ELN  = int(self.data[1])   # number of electrons in subshell when neutral
        self.rnm("N2","NTR")            # number of transitions
        self.transitions = [ENDF_File28_Transition(self.data[6*(i+1):]) for i in range(self.NTR)]

    def __repr__(self):
        s = self.printid() +'; Shell ' + ENDF_File28_List.ssname(self.SUBI) + ' (%g eV; n_e = %i)'%(self.EBI, self.ELN)
        for t in self.transitions: s += '\n'+str(t)
        return s

class ENDF_File28_Sec(ENDF_HEAD_Record):
    """File 28 'Atomic Relaxation Data'"""

    def __init__(self, iterlines, l0 = None):
        if l0 is None: l0 = next(iterlines)
        super().__init__(l0)
        assert self.MF == 28
        assert self.MT == 533

        self.rectp = "File 28 'Atomic relaxation data'"
        self.rnm("N1","NSS")  # number of subshell entries
        self.contents = [ENDF_File28_List(iterlines) for i in range(self.NSS)]


        footer = ENDF_HEAD_Record(next(iterlines))
        assert footer.rectp == "SEND"

    def __repr__(self):
        s = self.printid()
        for c in self.contents: s += '\n'+str(c)
        return s
