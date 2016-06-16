#! /usr/bin/env python

import ROOT, copy, sys

from ROOT import TH1F, TLegend, TCanvas

from DataFormats import *

   

class Experiment:
    def __init__(self, filename):
        print "Starting the experiment"
        self.transcalodata0, self.transcalodata1 = self.unpackCaloData(filename)
        self.events = []
        self.buildEvents()


    def unpackCaloData(self,filename):
        data = open(filename,"r")
        transcalodata0 = []
        transcalodata1 = []

        for line in data:
            if line.startswith("#"):  continue
            columns = line.split()
            #if int(columns[2]) < 2700 or int(columns[2]) > 3200: continue
            if int(columns[0]) == 0: transcalodata0.append(CaloEvent(columns[1],columns[2]))
            if int(columns[0]) == 2: transcalodata1.append(CaloEvent(columns[1],columns[2]))

        data.close()

        print "Unpacked calo data. Events for channel 0 =", len(transcalodata0),", channel 1 = ", len(transcalodata1)

        return transcalodata0, transcalodata1


    def clearTransientData(self):
        self.transcalodata0 = []
        self.transcalodata1 = []


    def buildEvents(self):

        print "Building the events"

        index_matchCaloEv1 = 0

        index_caloEv0 = 0
        index_caloEv1 = 0
        index_start1  = 0

    

        for caloEv0 in self.transcalodata0:
            index_caloEv0 += 1
            #if index_caloEv0 == 5000: break
            if index_caloEv0 % 5000 == 0: print index_caloEv0
            #print caloEv0.time, caloEv0.energy
            prevdt = pow(10,12)
            matchedCaloEv1 = Event(0,0)    
            #print "New loop ", index_caloEv0
            for caloEv1 in self.transcalodata1[index_start1:]:               
                #print index_caloEv1
                index_caloEv1 +=1
                dt = caloEv1.time - caloEv0.time if caloEv1.time > caloEv0.time else caloEv0.time - caloEv1.time
                #print caloEv1.time, caloEv0.time, dt
                #print index_caloEv1
                if dt > prevdt : break
                prevdt = dt
                matchedCaloEv1 = caloEv1

            self.events.append(Event(caloEv0, matchedCaloEv1))
            index_start1 = [max(index_caloEv1 - 10,0)][0]
            index_caloEv1 = index_start1
            #print "start from", index_start1, index_caloEv1

        self.clearTransientData()

       

if __name__ == '__main__':

    numbit = 14
    maxRawEnergy = pow(2,numbit)-1 
    nbins = (maxRawEnergy+1)/16

    inputfilename = sys.argv[1]
    experiment = Experiment(inputfilename)

    
    hDeltat = ROOT.TH1F("hDetlat","#Delta t between channels",20, -95, 105)
    hRawEnergyCh0 = ROOT.TH1F("hRawEnergyCh0","Raw Energy channel 0",nbins,0,maxRawEnergy)
    hRawEnergyCh1 = ROOT.TH1F("hRawEnergyCh1","Raw Energy channel 1",nbins,0,maxRawEnergy)

    dataout = open("formatted_"+inputfilename,"w")

    print "Analyze events"

    for event in experiment.events:
        #dt = 10 * (event.caloCh1.time - event.caloCh0.time if event.caloCh1.time > event.caloCh0.time else event.caloCh0.time - event.caloCh1.time)
        dt = 10 * (event.caloCh1.time - event.caloCh0.time)
        #print event.caloCh0.time, event.caloCh1.time
        #if(event.caloCh0.energy > 2200 and event.caloCh0.energy < 3200 and event.caloCh1.energy > 2200 and event.caloCh1.energy < 3200):
        hDeltat.Fill(dt)
        #if(abs(dt-26.6) < 10):
        hRawEnergyCh0.Fill(event.caloCh0.energy)
        hRawEnergyCh1.Fill(event.caloCh1.energy)
        dataout.write("{0:s} {1:s} {2:s} {3:s}\n".format(str(event.caloCh0.time), str(event.caloCh0.energy), str(event.caloCh1.time), str(event.caloCh1.energy)))
        #dataout.write(event.caloCh0.time.astype('|S20') + "{0:s} {1:s} {2:s}\n".format(str(event.caloCh0.energy), str(event.caloCh1.time), str(event.caloCh1.energy)))

    hDeltat.Draw("text")
    
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
    dataout.close()

    input()

