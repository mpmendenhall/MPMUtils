#!/usr/bin/python3
# stateless view of current reading values

from WebpageUtils import *
import xmlrpc.client
import time

class WebChecklist:
    def __init__(self):
        pass
    
    def get_readings(self):
        s = xmlrpc.client.ServerProxy('http://localhost:8000', allow_none=True)
        self.readings = s.newest()
        self.rnames = s.reading_names()
    
    def makeChecklistTable(self):
        t0 = time.time()
        trows = [{"class":"unknown", "data":["Device","Value","Unit","Last updated"]},]

        rlist = list(self.rnames.keys())
        rlist.sort()
        for i in rlist:
            rdat = [ self.rnames[i][1] + ":" + self.rnames[i][0], "???", self.rnames[i][2], "---", "..." ]
            cls = "good"
            if i in self.readings:
                rdat[1] = self.readings[i][1]
                rdat[3] = timeWriter(t0-self.readings[i][0])+" ago"
            else:
                cls = "unknown"
            trows.append({"class":cls, "data":rdat})

        return makeTable(trows)
        
    def makePage(self):
        self.get_readings()
        tbl = self.makeChecklistTable()
                    
        print(pageHeader("Readings Monitor", refresh=300))
        print('<h1>Readings as of %s</h1>'%time.asctime())
        print(tbl)
        print(pageFooter())
        

if __name__=="__main__":
    WC = WebChecklist()
    WC.makePage()    