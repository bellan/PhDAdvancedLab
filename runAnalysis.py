#! /usr/bin/env python

import ROOT, copy, sys


from ROOT import TH1F, TLegend, TCanvas, TFile

from DataFormats import *




       

if __name__ == '__main__':

    numbit = 14
    maxRawEnergy = pow(2,numbit)-1 


    inputfilename = sys.argv[1]
    analysis = Analysis(inputfilename)
    
    hDeltat = ROOT.TH1F("hDeltat","#Delta t between channels",10, 0, 100)
    hRawEnergyCh0 = ROOT.TH1F("hRawEnergyCh0","Raw Energy channel 0",maxRawEnergy,0,maxRawEnergy)
    hRawEnergyCh1 = ROOT.TH1F("hRawEnergyCh1","Raw Energy channel 1",maxRawEnergy,0,maxRawEnergy)
    hDeltat_10 = ROOT.TH1F("hDeltat_10","#Delta t between channels",10, 0, 100)
    hDeltat_01 = ROOT.TH1F("hDeltat_01","#Delta t between channels",10, 0, 100)

    dataout = ROOT.TFile("pippo.root","recreate")
    dataout.cd()

    print "Analyze events"

    for event in analysis.events:
        dt = 10*(event.caloCh1.time - event.caloCh0.time if event.caloCh1.time > event.caloCh0.time else event.caloCh0.time - event.caloCh1.time)
        #print event.caloCh0.time, event.caloCh1.time
        #if(event.caloCh0.energy > 2200 and event.caloCh0.energy < 3200 and event.caloCh1.energy > 2200 and event.caloCh1.energy < 3200):
        hDeltat.Fill(dt)
        #if(abs(dt-26.6) < 10):
        if(event.caloCh0.energy > 2200 and event.caloCh0.energy < 3200 and dt < 10 ):
            hRawEnergyCh1.Fill(event.caloCh1.energy)
            hDeltat_10.Fill(dt)
            if(event.caloCh1.energy > 6400 and event.caloCh1.energy < 7000):
                hRawEnergyCh0.Fill(event.caloCh0.energy)
                hDeltat_01.Fill(dt)


    hDeltat.Draw()
    
    cE = ROOT.TCanvas("cE","cE",200,10,600,400)

    hRawEnergyCh0.SetLineColor(2)
    hRawEnergyCh0.SetStats(ROOT.kFALSE);
    hRawEnergyCh0.Draw()

    hRawEnergyCh1.SetLineColor(4)
    hRawEnergyCh1.Draw("same")

    legend = ROOT.TLegend(0.45,0.7,0.65,0.85);
    legend.AddEntry(hRawEnergyCh0,"Canale 0");
    legend.AddEntry(hRawEnergyCh1,"Canale 1");
    legend.SetFillColor(0);
    legend.Draw();

    hDeltat.Write()
    hDeltat_01.Write()
    hDeltat_10.Write()
    hRawEnergyCh0.Write()
    hRawEnergyCh1.Write()

    dataout.Close()


    input()

