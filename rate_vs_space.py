#! /usr/bin/env python

import ROOT, array, sys, math

from DataFormats import *


if __name__ == '__main__':

    #positions = [700]
    #err_pos   = [0.1]
    
    positions = [650,675,700,725,750,800]
    err_pos   = [0.1,0.1,0.1,0.1,0.1,0.1]
    positions = [350,400,450,500,550,600,650,700,725,750,775,800,850,900,950]
    err_pos   = [0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1]
    rates = []
    err_rates = []
    #filenamelist = ["formatted_run_20160613_"+str(position)+"_0_2cm.txt" for position in positions]
    filenamelist = ["formatted_run_20160613_"+str(position)+"_0.txt" for position in positions]
  

    for filename in filenamelist:
        analysis = Analysis(filename)
        counter = 0
        
        elapsedtime = pow(10,-8)*(analysis.events[len(analysis.events)-1].caloCh1.time + analysis.events[len(analysis.events)-1].caloCh0.time - analysis.events[0].caloCh1.time - analysis.events[0].caloCh0.time)/2
        print elapsedtime
        for event in analysis.events:
            dt = 10*(event.caloCh1.time - event.caloCh0.time)
            if(abs(dt) < 100 and event.caloCh0.energy > 2800 and event.caloCh0.energy < 3400 and event.caloCh1.energy > 2600 and event.caloCh1.energy < 3100):
                counter +=1
        rates.append(counter/elapsedtime)
        err_rates.append(math.sqrt(counter)/elapsedtime)

    gRateVsPosition = ROOT.TGraphErrors(len(positions), array.array('f', (position/10 for position in positions)), array.array('f',rates), array.array('f',err_pos), array.array('f',err_rates))

    gRateVsPosition.SetLineColor(2);
    gRateVsPosition.SetLineWidth(4);
    gRateVsPosition.SetMarkerColor(4);
    gRateVsPosition.SetMarkerSize(1.1);
    gRateVsPosition.SetMarkerStyle(21);
    gRateVsPosition.SetTitle("");
    gRateVsPosition.GetXaxis().SetTitle("position [mm]");
    gRateVsPosition.GetYaxis().SetTitle("rate [counts/s]");

    gRateVsPosition.Draw("AP")
    
    gaus = ROOT.TF1("gaus","gaus")
    gaus.SetParameter(0,1)
    gaus.SetParameter(1,7)
    gaus.SetParameter(2,5)
    
    gRateVsPosition.Fit(gaus)

    input()
    
