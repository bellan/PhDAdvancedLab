#! /usr/bin/env python

import ROOT, copy, sys

from ROOT import TH1F, TLegend

numbit = 14
maxRawEnergy = pow(2,numbit)-1 
nbins = (maxRawEnergy+1)/16

data = open(sys.argv[1],"r")

hRawEnergyCh0 = ROOT.TH1F("hRawEnergyCh0","Raw Energy channel 0",nbins,0,maxRawEnergy)
hRawEnergyCh1 = ROOT.TH1F("hRawEnergyCh1","Raw Energy channel 1",nbins,0,maxRawEnergy)

hExtrasCh0 = ROOT.TH1F("hExtrasCh0","Extras channel 0",10,0,10)
hExtrasCh1 = ROOT.TH1F("hExtrasCh1","Extras channel 1",10,0,10)


for line in data:
    if line.startswith("#"):  continue
    columns = line.split()
    if int(columns[0]) == 0: 
        hRawEnergyCh0.Fill(float(columns[2]))
        hExtrasCh0.Fill(int(columns[3]))
    if int(columns[0]) == 2: 
        hRawEnergyCh1.Fill(float(columns[2]))
        hExtrasCh1.Fill(int(columns[3]))

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


cExtras = ROOT.TCanvas("cExtras","Extras")
cExtras.cd()

hExtrasCh0.SetLineColor(2)
hExtrasCh0.SetStats(ROOT.kFALSE);
hExtrasCh1.SetLineColor(4)

hExtrasCh0.Draw()
hExtrasCh1.Draw("same")

legend2 = ROOT.TLegend(0.45,0.7,0.65,0.85);
legend2.AddEntry(hExtrasCh0,"Canale 0");
legend2.AddEntry(hExtrasCh1,"Canale 1");
legend2.SetFillColor(0);
legend2.Draw();


input()
