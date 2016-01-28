#!/usr/bin/env python

from ROOT import *

f=TFile('boolehists_karol.root')
h_hits = f.Get('h_hits')
h_nsps = f.Get('h_nsps')

h_slim = TH1F('h_slim', 'h_slim', 624, -0.5, 623.5)

for i in range(624):
    lim = 1
    if i%12 == 2 or i%12 == 6 : lim = 4
    elif i%12 == 1 or i%12 == 7 : lim = 2
    else : lim = 1
    h_slim.Fill(i, 4*lim)

h_prof = h_nsps.ProfileX()
h_prof.Draw()
h_prof.SetTitle('Avg # SPs vs Chip')
h_slim.SetMarkerColor(kRed)
h_slim.Draw('p* same')

