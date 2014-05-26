from Gaudi.Configuration import *

from GaudiKernel.ProcessJobOptions import importOptions
from Configurables import Boole
#importOptions("$APPCONFIGOPTS/Boole/Boole-Upgrade-VP-UT-FT.py")
Boole().DetectorDigi = ['VP']
Boole().DetectorLink = ['VP']
Boole().DetectorMoni = ['VP']
#importOptions("$APPCONFIGOPTS/Boole/patchUpgrade1.py")
#importOptions("$APPCONFIGOPTS/Boole/xdigi.py")

# Set the database tags.
from Configurables import CondDB, LHCbApp
CondDB().Upgrade     = True
LHCbApp().DDDBtag    = "dddb-20131025"
LHCbApp().CondDBtag  = "sim-20130830-vc-md100"
CondDB().AllLocalTagsByDataType=["VP_UVP+RICH_2019+UT"]

# Specify the input sim files.
#EventSelector().Input = ["DATAFILE='PFN:/castor/cern.ch/user/t/thead/ganga/VPdigital/325/%s/OctTest-Extended.digi"%n for n in xrange(50)]
EventSelector().Input = ["DATAFILE='PFN:OctTest-Extended.digi"]

#Boole.Outputs = ["MDF", "DIGI"]

# Optionally set the number of events to be processed.
LHCbApp().EvtMax = 10000

def moni():
  from Configurables import VPClusterMonitor, VPDepositMonitor
  GaudiSequencer("MoniVPSeq").Members += [VPDepositMonitor(), VPClusterMonitor()]
appendPostConfigAction(moni)

def patch():
  from Configurables import VPSuperPixBankEncoder
  GaudiSequencer("DigiVPSeq").Members += [VPSuperPixBankEncoder()]
  VPSuperPixBankEncoder().RawEventLocation = "/Event/Raw/VP/SuperPixels"
  #OutputStream("DigiWriter").ItemList += ["/Event/MC/VP/Hits#1"]
  OutputStream("DigiWriter").ItemList += ["/Event/Raw/VP/SuperPixels#1"]
  #OutputStream("RawWriter").ItemList += ["/Event/Raw/VP/SuperPixels#1"]
  #OutputStream("RawWriter").Output = "DATAFILE='PFN:SomeFile.mdf' SVC='LHCb::RawDataCnvSvc' OPT='REC'"

appendPostConfigAction(patch)
