#!/usr/bin/env python
import sys

i=0
gwtdata=''
parbits=''
gwtframe=''
emptypix = '00000000'
lastclk = 0
nspix = 0
curbcid='000000000000'
chip='0000'

for line in sys.stdin :
        lsplit = line.split()
        clk = lsplit[0]
        if i == 0 : 
                lastclk=clk
        if clk != lastclk or nspix == 4:
                #print i, sp, par
                if nspix == 4:
                        gwtframe = '0101'+parbits+gwtdata
                else:
                        bcidstr = '{0:012b}'.format(graycurbcid)
                        chipstr = '{0:04b}'.format(chip)
                        spp = '010101'+bcidstr+chipstr+emptypix
                        par = int(spp[0])
                        for j in spp[1:] :
                                par = par^int(j)
                        #print 'parity ', spp, par
                        for j in range(4-nspix):
                                parbits = str(par)+parbits
                                gwtdata = spp+gwtdata  
                        gwtframe = '0101'+parbits+gwtdata
                print gwtframe #, nspix
                gwtdata = ''
                parbits = ''

                nspix = 0


        # for non-filled GWT frames, fill with current bcid and chip id
        curbcid = i%3564 # fill with current full bcid (not same as hit bcid) 
        graycurbcid = (curbcid >> 1) ^ curbcid;
        chip = int(lsplit[3])%12
        sp = lsplit[1]
        par = lsplit[2]
        gwtdata += sp
        parbits += par

        i+=1
        nspix += 1
        lastclk=clk

