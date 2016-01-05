#!/usr/bin/python3

# gpt: gnuplot pipe
# plots: dictionary of x,y points

import time

def pwrite(p,s):
    p.stdin.write(bytes(s, 'UTF-8'))
    
class PlotMaker:
    def __init__(self):
        self.renames = {}       # graph title re-naming
        self.datasets = {}      # availabale datasets

    def pass_gnuplot_data(self,k,gpt):
        """Pass data to gnuplot for keys in k"""
        k = [p for p in k if self.datasets.get(p,None)]
        if not len(k):
                print("No data to plot!")
                return False
        pwrite(gpt,"plot")
        pstr = ""
        for p in k:
                pstr += """ "-" title "%s: %g","""%(self.renames.get(p,p), self.datasets[p][-1][1])
        pwrite(gpt,pstr[:-1]+'\n')
        time.sleep(0.01)
        for p in k:
                for d in self.datasets[p]:
                        pwrite(gpt,"%f\t%f\n"%(d[0],d[1]))
                pwrite(gpt,"e\n")
                gpt.stdin.flush()
                time.sleep(0.01)
        gpt.stdin.flush()
        time.sleep(0.1)
        return True

    def pass_gnuplot_timedata(self, k, gpt, tdelta = -time.time()):
        k = [p for p in k if self.datasets.get(p,None)]
        if not len(k):
                print("No data to plot!")
                return False
        pwrite(gpt,"plot")
        pstr = ""
        for p in k:
                pstr += """ "-" using 1:2 title "%s: %g","""%(self.renames.get(p,p),self.datasets[p][-1][1])
        pwrite(gpt,pstr[:-1]+'\n')
        time.sleep(0.01)
        for p in k:
                for d in self.datasets[p]:
                        pwrite(gpt,"%i\t%f\n"%((tdelta+d[0]), d[1]))
                pwrite(gpt,"e\n")
                gpt.stdin.flush()
                time.sleep(0.01)
        gpt.stdin.flush()
        time.sleep(0.1)
        return True