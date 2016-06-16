import numpy

class CaloEvent:
    def __init__(self,time,energy,timeCorr = 0):
        self.time   = numpy.int64(time) + numpy.int64(timeCorr)
        self.energy = int(energy)

class Event:
    def __init__(self,calo0,calo1):
        self.caloCh0 = calo0
        self.caloCh1 = calo1


class Analysis:
    def __init__(self, filename):
        print "Starting the analysis"
        self.events = []
        self.packData(filename)

    def packData(self,filename):
        data = open(filename,"r")
        transcalodata0 = []
        transcalodata1 = []

        for line in data:
            if line.startswith("#"):  continue
            columns = line.split()
            self.events.append(Event(CaloEvent(columns[0],columns[1]),CaloEvent(columns[2],columns[3])))

        data.close()

        print "Packed data."
