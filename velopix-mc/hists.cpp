#include <stdio.h>
//#include <cstdlib>
#include "TROOT.h"
#include "TH2F.h"
#include "TH1D.h"
#include "TFile.h"
#include "TProfile.h"
#include "TCanvas.h"


int main(int argc, char **argv)
{
        FILE *f = fopen("boole_mc_data.bin", "rb");
        TFile *rf = new TFile("hists.root", "recreate");
        int nevts = 10;
        int chip = -1;
        int banksize = -1;
        int nsp = 0;
        int bank ; 
        int evt ; 
        int i,j;
        int k=0;
        unsigned char paritybits = 0; 
        unsigned int gwtdata[120];

        TH2F *h = new TH2F("h", "#SPP / event", 56, -0.5, 55.5, 512, -0.5, 511.5);
        TH2F *hchip = new TH2F("hch", "#SPP / chip", 624, -0.5, 623.5, 50, -0.5, 49.5);
        TH2F *hchip12 = new TH2F("hch12", "#SPP / chip%12", 12, -0.5, 11.5, 5000, -0.5, 4999.5);

        int nhits = 0;
        int mod = 0;
        int lastmod = 0;

        while(1==fread(&evt, sizeof(unsigned int), 1, f)){


                lastmod = mod;
                if (feof(f)) break;
                fread(&chip, sizeof(unsigned int), 1, f);
                mod = chip/12;
                if (mod!=lastmod){
                        h->Fill(mod, nhits);
                        nhits = 0;
                }
                //printf("evt %d chip %d \n", evt, chip);
                fread(&banksize, sizeof(unsigned int), 1, f);
                fread(&nsp, sizeof(unsigned int), 1, f);
                nhits += nsp;
                hchip->Fill(chip, nsp);
                hchip12->Fill(chip%12, nsp);
                for(i=0; i<nsp; i++){
                        fread(&bank, sizeof(unsigned int), 1, f);
                }

        }

        h->GetXaxis()->SetTitle("module number");
        h->GetYaxis()->SetTitle("#SPP");
        TH1D *hpy = h->ProjectionY();
        hpy->SetTitle("Average #SPP/event (whole VELO)");
        TProfile *hpfx = h->ProfileX();
        hpfx->GetXaxis()->SetTitle("module number");
        hpfx->SetTitle("#SPP/event");
        hpfx->Write();
        hpy->Write();
        h->Write();

        hchip->GetXaxis()->SetTitle("chip number 0-623");
        hchip->GetYaxis()->SetTitle("#SPP");
        TH1D *hchippy = hchip->ProjectionY();
        hchippy->SetTitle("Average #SPP/event ()");
        TProfile *hchippfx = hchip->ProfileX();
        hchippfx->GetXaxis()->SetTitle("chip number 0-623");
        hchippfx->SetTitle("#SPP/event");
        hchippfx->Write();
        hchippy->Write();
        hchip->Write();

        hchip12->GetXaxis()->SetTitle("chip number 0-11");
        hchip12->GetYaxis()->SetTitle("#SPP");
        TH1D *hchip12py = hchip12->ProjectionY();
        hchip12py->SetTitle("Average #SPP/event (chip%12)");
        TProfile *hchip12pfx = hchip12->ProfileX();
        hchip12pfx->GetXaxis()->SetTitle("chip number 0-11");
        hchip12pfx->SetTitle("#SPP/event");
        hchip12pfx->Write();
        hchip12py->Write();
        hchip12->Write();

        TCanvas *c = new TCanvas();
        for(int i=0; i<52; i++){
                char c1[20];
                char c2[20];
                sprintf(c1, "h_pf_%02d", i);
                sprintf(c2, "plots/h_pf_%02d.png", i);
                TH1D *pj = h->ProjectionY(c1, i+1, i+1);
                pj->Write();

                pj->Draw();
                c->Update();
                c->SaveAs(c2);
        }

        rf->Close();
        fclose(f);
        return 0;
}

