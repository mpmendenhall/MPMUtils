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
        self.x_txs = {}         # plot transform functions on x axis
        self.y_txs = {}         # plot transform functions on y axis
        
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
                xtx = self.x_txs.get(p,(lambda x: x))
                ytx = self.y_txs.get(p,(lambda y: y))
                for d in self.datasets[p]:
                        pwrite(gpt,"%f\t%f\n"%(xtx(d[0]), ytx(d[1])))
                pwrite(gpt,"e\n")
                gpt.stdin.flush()
                time.sleep(0.01)
        gpt.stdin.flush()
        time.sleep(0.1)
        
        return True
