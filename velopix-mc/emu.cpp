#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "TFile.h"
#include "TTree.h"

inline unsigned int binaryToGray(unsigned int num);
unsigned int grayToBinary(unsigned int num);
inline unsigned int numLinks(unsigned int chip);
void permute(unsigned int *p, unsigned int n);
void init_filling_scheme();
void write_out_gwtdata(unsigned char gwtdata[624][4][128], FILE *outfile);
struct SP* create_SP(int bank, int evt, int chip, unsigned int *globalid);
void init_trees(TTree *evttree, TTree *sptree, unsigned int *tevt, unsigned int *tchip, unsigned int *tbxid, unsigned int *tlink, unsigned int *tnumlinks, unsigned int *tspcount, unsigned int *tnsp, unsigned int *tbackp, unsigned int *tvalid);
void init_arrs(struct SP** latency_buff, struct SP** latency_buff_tail, unsigned int *SP_counts, unsigned int *backpressure, unsigned int *last_rrlink, unsigned int rand_spPos[624][16], unsigned char gwtdata[624][4][128]);

int filling_scheme[3654];

struct SP
{
        unsigned int latency;
        unsigned int chip;
        unsigned int fullbcid;
        //unsigned int spAddr;
        unsigned int parity;
        //unsigned int pixels;
        unsigned int bank;
        unsigned int id;  // for debug only

        struct SP *next;
};

void printList(struct SP *head);
struct SP* listSearchAndRemove(unsigned int chip, struct SP **head);

int total_hits = 0;

int main(int argc, char **argv)
{
        //FILE *f = fopen("/home/karol/Dropbox/bigdata/velopix-mc/booledata-2015.bin", "rb");
        //FILE *f = fopen("/home/karol/Dropbox/bigdata/velopix-mc/booledata-2016-02.bin", "rb");
        //FILE *f = fopen("test-2016_07-renum.bin", "rb");
        FILE *f = fopen("test-2016_07_29-renum.bin", "rb");
        //FILE *f = fopen("test.bin", "rb");
        FILE *outf = fopen("spix-latency-gray-datavalid.txt", "w");
        TFile *rootfile = new TFile("emutest.root", "recreate");
        TTree *evttree = new TTree("evttree", "evttree");
        TTree *sptree = new TTree("sptree", "sptree");

        int nevts = 10;
        int chip = -1;
        int banksize = -1;
        int nsp = 0;
        int total_nsp = 0;
        int bank ; 
        int evt ; 
        int i,j;
        int k=0;
        unsigned int globalid = 0;
        unsigned char paritybits = 0; 
        //unsigned int gwtdata[120];
        unsigned char gwtdata[624][4][128];  //chips, links, bits
        struct SP *latency_buff[512];
        struct SP *latency_buff_tail[512];
        unsigned int SP_counts[624];
        unsigned int nsp_perchip[624];
        unsigned int backpressure[624];
        unsigned int rand_spPos[624][16];
        unsigned int last_rrlink[624];  // round robin the links
        int tl=0;
        int lastevt=0;
        unsigned int c,l,b = 0; //chips, links, bits

        srand(time(NULL));
        init_filling_scheme();

        init_arrs(latency_buff, latency_buff_tail, SP_counts, backpressure, last_rrlink, rand_spPos, gwtdata);


        unsigned int tevt;
        unsigned int tchip;
        unsigned int tbxid;
        unsigned int tlink;
        unsigned int tnumlinks;
        unsigned int tspcount;
        unsigned int tnsp;
        unsigned int tbackp;
        unsigned int tvalid;

       
        init_trees(evttree, sptree, &tevt, &tchip, &tbxid, &tlink, &tnumlinks, &tspcount, &tnsp, &tbackp, &tvalid);

        while(1==fread(&evt, sizeof(unsigned int), 1, f)){
                //if (evt == 100) break;
                if (lastevt != evt){
                        
                        printf("evt %d hits %d nsp %d\r", evt, total_hits, total_nsp);
                        fflush(stdout);
                        //total_nsp = 0;
                        //total_hits = 0;
                        //fprintf(stderr, "sp in gwtdata\n");
                        
                        struct SP *backpressure_buff[512] ;
                        struct SP *backpressure_buff_tail[512] ;
                        int ii;
                        for (ii=0; ii<512; ii++){
                                backpressure_buff[ii] = NULL;
                                backpressure_buff_tail[ii] = NULL;
                        }

                        // put SPs in gwtdata
                        for(c=0; c<624; c++){
                                unsigned int t = lastevt%512;

                                //struct SP *head = latency_buff[t];
                                struct SP *curSP ;
                                //printList((latency_buff[t]));
                                //if (evt==100) printf("%d %d\n", c, numLinks(c));
                                while((curSP = listSearchAndRemove(c, &(latency_buff[t]))) != NULL){
                                        //if(evt > 510){
                                        //fprintf(stderr, "%d %d -- ", c, t);
                                        //printList(latency_buff[t]);
                                        //}


                                        // if it's an empty bunch crossing, dump it
                                        if(filling_scheme[curSP->fullbcid] == 0){
                                                free(curSP); 
                                                continue;
                                        }

                                        //put curSP in gwtdata 
                                        unsigned int curNumLinks = numLinks(c);
                                        unsigned int numSlots = 4*curNumLinks;  // total number of slots 4-16
                                        //if(SP_counts[c] < 4*curNumLinks){
                                        if(SP_counts[c] < numSlots){
                                                //int curLink = (last_rrlink[c]+SP_counts[c]/4)%4;
                                                if(SP_counts[c]==0){
                                                        unsigned int j;
                                                        for(j=0; j<numSlots; ++j){
                                                                rand_spPos[c][j]=j;
                                                        }
                                                        permute(rand_spPos[c], numSlots);
                                                }
                                                int spPos = rand_spPos[c][SP_counts[c]]; 
                                                int curLink = spPos/4;

                                                //if (c == 110) {
                                                        //printf(" on line chip %d addr %x latency %d\n", curSP->chip, (curSP->bank)>>8, curSP->latency);
                                                        //printf("    c=%d evt=%d link=%d sppos=%d rand=%d cnt=%d numlinks=%d lastrrlink=%d spcounts=%d \n", c, evt, curLink, spPos, rand_spPos[c], SP_counts[c], numLinks(c), last_rrlink[c], SP_counts[c]);
                                                //}

                                                for(b=0; b<30; b++){
                                                        unsigned int bit = ((curSP->bank)>>b)&1;  // may need to reverse LSB/MSB ordering to ((curSP->bank)>>(29-b))&1 
                                                        gwtdata[c][curLink][b+8+(30*(spPos%4))] = bit;
                                                }

                                                gwtdata[c][curLink][4+(spPos%4)] = curSP->parity;
                                                free(curSP);
                                                SP_counts[c]++;
                                        } else {
                                        // not enough room, move SP to next event. 
                                                

                                                //continue; // zzzz

                                                unsigned int tn = (t+1)%512;
                                                //curSP->next = latency_buff[tn];
                                                curSP->latency = (curSP->latency)+1;
                                                //latency_buff[tn] = curSP;

                                                backpressure[curSP->chip]++;

                                                if (backpressure_buff[tn] == NULL){
                                                        backpressure_buff[tn] = curSP;
                                                        backpressure_buff_tail[tn] = curSP;
                                                        backpressure_buff[tn]->next = NULL;
                                                } else {
                                                        backpressure_buff_tail[tn]->next = curSP;
                                                        backpressure_buff_tail[tn] = backpressure_buff_tail[tn]->next;
                                                        backpressure_buff_tail[tn]->next = NULL;
                                                
                                                }


                                                //if (c==111)
                                                //printf("backpressure %d chip %d addr %x latency %d\n", backpressure[curSP->chip], curSP->chip, (curSP->bank)>>8, curSP->latency);
                                        }

                                        //if (c == 218){
                                                //printf("id %d ", curSP->id);
                                        //}
                                }
                        }
                        //printf("backpressure ");
                        int totbp = 0;
                        for (ii=0; ii<624; ii++){
                                totbp += backpressure[ii];
                                //printf("%d %3d %d\n",ii, backpressure[ii], numLinks(ii));
                        }
                        //printf("\n");


                        //printf("backpressure 218 %d tot %d \n", backpressure[218], totbp);
                        for (ii=0; ii<624; ii++){
                                last_rrlink[ii] = (1+last_rrlink[ii]+(SP_counts[ii]-1)/4)%4;
                                
                                //rand_spPos[ii] = rand()%4;
                                //int j;
                                //for(j=0; j<4; ++j){
                                        //rand_spPos[ii][j]=j;
                                //}
                                //permute(rand_spPos[ii]);
                        }

                        for (ii=0; ii<512; ii++){
                                if (backpressure_buff_tail[ii] != NULL){
                                        backpressure_buff_tail[ii]->next = latency_buff[ii]; 
                                        latency_buff[ii] = backpressure_buff[ii];
                                }
                                backpressure_buff[ii] = NULL;
                        }

                        //fprintf(stderr, "clear gwtdata\n");
                        // print and clear gwtdata
                        write_out_gwtdata(gwtdata, outf);

                        // fill evttree and clear SP_counts and backpressure
                        for(c=0; c<624; c++){
                                unsigned nl = numLinks(c);
                                for(l=0; l<nl; l++){
                                        tevt = evt;
                                        tchip = c;
                                        tnumlinks = nl;
                                        tbackp = backpressure[c];
                                        //tvalid = datavalid;
                                        tspcount = SP_counts[c];
                                        tnsp = nsp;
                                        evttree->Fill();
                                }
                                SP_counts[c] = 0;
                                backpressure[c] = 0;
                        }

                }
                lastevt = evt;

                if (feof(f)) break;
                fread(&chip, sizeof(unsigned int), 1, f);
                fread(&banksize, sizeof(unsigned int), 1, f);
                fread(&nsp, sizeof(unsigned int), 1, f);
                total_nsp += nsp;

                //printf("evt %d chip %d nsp %d\n", evt, chip, nsp);
                // read SPs from file and create struct SPs
                //fprintf(stderr, "read bin file\n");
                for(i=0; i<nsp; i++){
                        fread(&bank, sizeof(unsigned int), 1, f);
                        
                        struct SP *newsp = create_SP(bank, evt, chip, &globalid);

                        // add to end of list
                        int latency = newsp->latency;
                        if (latency_buff[latency%512] == NULL){
                                latency_buff[latency%512] = newsp;
                                latency_buff_tail[latency%512] = newsp;
                        } else {
                                latency_buff_tail[latency%512]->next = newsp;
                                latency_buff_tail[latency%512] = latency_buff_tail[latency%512]->next;
                        }



                }

                tevt = evt;
                tchip = chip;
                tbackp = backpressure[chip];
                //tvalid = datavalid;
                tnsp = nsp;
                sptree->Fill();

        }

        fclose(f);
        fclose(outf);
        //evttree->Write();
        //sptree->Write();
        rootfile->Write();
        rootfile->Close();
        printf("\n");
        return 0;
}

void init_arrs(struct SP** latency_buff, struct SP** latency_buff_tail, unsigned int *SP_counts, unsigned int *backpressure, unsigned int *last_rrlink, unsigned int rand_spPos[624][16], unsigned char gwtdata[624][4][128])
{

        int ii, c, l, b;
        for (ii=0; ii<512; ii++){
                latency_buff[ii] = NULL;
                latency_buff_tail[ii] = NULL;
        }
        for (ii=0; ii<624; ii++){
                SP_counts[ii] = 0;
                backpressure[ii] = 0;
                //rand_spPos[ii] = rand()%4;
                int j;
                for(j=0; j<16; ++j){
                        rand_spPos[ii][j]=j;
                }
                last_rrlink[ii] = 0;
        }

        for(c=0; c<624; c++){
                for(l=0; l<4; l++){
                        for(b=0; b<128; b++){
                                gwtdata[c][l][b] = (b==1 || b==3)? 1 : 0;
                        }
                }
        }

}
void init_trees(TTree *evttree, TTree *sptree, unsigned int *tevt, unsigned int *tchip, unsigned int *tbxid, unsigned int *tlink, unsigned int *tnumlinks, unsigned int *tspcount, unsigned int *tnsp, unsigned int *tbackp, unsigned int *tvalid)
{

        evttree->Branch("evt", tevt, "evt/I");
        evttree->Branch("chip", tchip, "chip/I");
        evttree->Branch("bxid", tbxid, "bxid/I");
        evttree->Branch("link", tlink, "link/I");
        evttree->Branch("numlinks", tnumlinks, "numlinks/I");
        evttree->Branch("spcount", tspcount, "sp/I");
        evttree->Branch("nsp", tnsp, "sp/I");
        evttree->Branch("backpressure", tbackp, "backpressure/I");
        evttree->Branch("valid", tvalid, "valid/I");

        sptree->Branch("chip", tchip, "chip/I");
        sptree->Branch("nsp", tnsp, "nsp/I");
        sptree->Branch("backpressure", tbackp, "backpressure/I");
        sptree->Branch("evt", tevt, "evt/I");
}


struct SP* create_SP(int bank, int evt, int chip, unsigned int *globalid)
{

        unsigned int pix = bank&0xff;
        unsigned int spAddr = bank >> 8 ;
        unsigned int fullbcid = evt%3564;
        unsigned int bcid = fullbcid & 0x1ff;
        unsigned int graybcid = binaryToGray(bcid);  // NOTE : gray(9b) != gray(12b)&0x1ff  i.e. MUST gray 9b number!
        unsigned int latency = evt + 8 + spAddr%64;  // this is time of arrival from t=0 first evt.
        //int latency = evt + 1;  // cross-check bcid nums   //zzzz

        // TURN OFF GRAY CODING!!!  AND FULL BANK!!! (sacrificing addr for a test)
        unsigned int sp = bank|(graybcid<<21);
        //int sp = (bank&0xff)|(fullbcid<<18);  // full bcid test - sacrifices addr
        struct SP *newsp = (struct SP*)malloc(sizeof(struct SP));
        unsigned int parity = 0;

        //printf("evt %d latency %d sp %d spAddr %d pix %d bcid %d graybcid %d chip %d\n", evt, latency, i, spAddr, pix, bcid, graybcid, chip);

        if(pix!=0){
                //fprintf(outf, "%d ", latency);
                int j;
                for(j=29; j>=0; j--){
                        unsigned int bit = (sp>>j)&1;
                        parity = (j==29) ? bit : bit ^ parity;             // assumes even parity
                        if (j<8) total_hits += bit;
                }
                //fprintf(outf, " %d %d\n", parity, chip); 
        } else {
                fprintf(stderr, "should be no empty pixels!!\n");
        }

        // cross check bank is OK
        if (sp > (1<<30)){
                fprintf(stderr, "SP too large!!\n");
        }
        //if (chip==111 && evt < 100) 
        //printf("evt=%d, pix=%x, spaddr=%x, graybcid=%x, parity=%d, latency=%d\n", evt, pix, spAddr, graybcid, parity, latency);
        newsp->latency = latency;
        newsp->chip = chip;
        newsp->fullbcid = fullbcid;
        //newsp->spAddr=spAddr;
        newsp->parity = parity;
        //newsp->pixels = pix;
        newsp->bank = sp;
        newsp->id = (*globalid)++;
        newsp->next = NULL;

        return newsp;
}
        
void write_out_gwtdata(unsigned char gwtdata[624][4][128], FILE *outfile)
{

        unsigned int c, l, b;
        //for(c=0; c<624; c++){
        for(c=192; c<204; c++){
                unsigned nl = numLinks(c);
                char cbuff[129];
                cbuff[128] = '\0';
                for(l=0; l<4; l++){
                        if(l<nl){
                                int f1 = 0;
                                int f2 = 0; 
                                int f3 = 0;
                                int f4 = 0;
                                int sum = 0;
                                unsigned int datavalid;
                                for(b=4; b<128; b++){  // leave 1010 at start
                                        //fprintf(outf, "%d", gwtdata[c][l][b]);
                                        cbuff[b] = 48+gwtdata[c][l][b];
                                        if (b >= 8 && b < 38 ){ 
                                                f1 |= gwtdata[c][l][b];
                                        } else if (b >= 38 && b < 68 ) {
                                                f2 |= gwtdata[c][l][b] ;
                                        } else if (b >= 68 && b < 98 ) {
                                                f3 |= gwtdata[c][l][b] ;
                                        } else if (b >= 98 && b < 128 ) {
                                                f4 |= gwtdata[c][l][b] ;
                                        }
                                        gwtdata[c][l][b] = 0;
                                }


                                // NEED TO ADD 0110 FOR DATA INVALID FRAME


                                sum = f1+f2+f3+f4;
                                datavalid = (sum>0) ? 1 : 0;  // NOT CORRECT?!?
                                //datavalid = (SP_counts[c]>0) ? 1 : 0;  

                                //header
                                if (datavalid == 1){
                                        cbuff[0] = '0';
                                        cbuff[1] = '1';
                                        cbuff[2] = '0';
                                        cbuff[3] = '1';
                                } else {
                                        cbuff[0] = '0';
                                        cbuff[1] = '1';
                                        cbuff[2] = '1';
                                        cbuff[3] = '0';
                                }

                                fprintf(outfile, "%128s %d %d \n", cbuff, datavalid, c); 

                        }
                }
        }
}


inline unsigned int binaryToGray(unsigned int num)
{
        return (num >> 1) ^ num;
}

/*
 *         The purpose of this function is to convert a reflected binary
 *                 Gray code number to a binary number.
 *                 */
unsigned int grayToBinary(unsigned int num)
{
        unsigned int mask;
        for (mask = num >> 1; mask != 0; mask = mask >> 1)
        {
                num = num ^ mask;
        }
        return num;
}

inline unsigned int numLinks(unsigned int chip)
{
        unsigned int mod12 = chip%12;
        //return 4;
        //old data
        //if (mod12 == 0 || mod12 == 8) 
                //return 4;
        //else if (mod12 == 1 || mod12 == 7)
                //return 2;
        //else
                //return 1;
        //new data
        if (mod12 == 2 || mod12 == 6) 
                return 4;
        else if (mod12 == 1 || mod12 == 7)
                return 2;
        else
                return 1;
}

void printList(struct SP *head)
{

        struct SP *p = head;
        int len = 0;

        while(p != NULL){
                //fprintf(stderr, "%d ", p->chip);
                p = p->next;
                len++;

        }
        if(len != 0){
                fprintf(stderr, "list chip %d len %d \n", head->chip, len);
        }
}


struct SP* listSearchAndRemove(unsigned int chip, struct SP **head)
{
        //struct SP *p = head;
        struct SP *p = (*head);
        struct SP *prev = NULL;

        while(p != NULL){
                //fprintf(stderr, "1\n");
                if(p->chip == chip){
                        if(prev != NULL){
                                //fprintf(stderr, "2\n");
                                //fprintf(stderr, "chip %d %d %d \n", prev->chip, p->chip, (int)(p->next));
                                prev->next = p->next;
                        } else {
                                //fprintf(stderr, "3\n");
                                *head = p->next;
                                //head = p->next;
                        }
                        return p;
                } else {
                        //fprintf(stderr, "4\n");
                        prev = p;
                        p = p->next;
                }
        }
        return NULL;
        
}

void permute(unsigned int *p, unsigned int n)
{
        unsigned int i;
        for(i=0; i<n; ++i){
                int j = rand() % (i+1);
                p[i] = p[j];
                p[j] = i;
        }
}

void init_filling_scheme()
{
        int i;
        int curfs[] = {
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        for (i=0; i<3564; ++i){
                //filling_scheme[i] = curfs[i];  // nominal
                filling_scheme[i] = 1;         // all bunches filled
        }
}
