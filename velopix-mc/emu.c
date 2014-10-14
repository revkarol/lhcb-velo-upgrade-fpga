#include <stdio.h>
#include <stdlib.h>

inline unsigned int binaryToGray(unsigned int num);
unsigned int grayToBinary(unsigned int num);
inline unsigned int numLinks(unsigned int chip);

struct SP
{
        unsigned int latency;
        unsigned int chip;
        //unsigned int graybcid;
        //unsigned int spAddr;
        unsigned int parity;
        //unsigned int pixels;
        unsigned int bank;

        struct SP *next;
};

void printList(struct SP *head);
struct SP* listSearchAndRemove(unsigned int chip, struct SP **head);

int main(int argc, char **argv)
{
        //FILE *f = fopen("test.bin", "rb");  // old data, new MC
        //FILE *f = fopen("boole_mc_data.bin", "rb");  // new data
        FILE *f = fopen("old_boole_mc.bin", "rb");  // old data
        FILE *outf = fopen("spix-latency-gray-datavalid.txt", "w");
        int nevts = 10;
        int chip = -1;
        int banksize = -1;
        int nsp = 0;
        int total_nsp = 0;
        int total_hits = 0;
        int bank ; 
        int evt ; 
        int i,j;
        int k=0;
        unsigned char paritybits = 0; 
        //unsigned int gwtdata[120];
        unsigned char gwtdata[624][4][128];  //chips, links, bits
        struct SP *latency_buff[512];
        unsigned int SP_counts[624];
        unsigned int backpressure[624];
        int tl=0;
        int lastevt=0;
        int c,l,b = 0; //chips, links, bits


        int ii;
        for (ii=0; ii<512; ii++){
                latency_buff[ii] = NULL;
        }
        for (ii=0; ii<624; ii++){
                SP_counts[ii] = 0;
                backpressure[ii] = 0;
        }

        for(c=0; c<624; c++){
                for(l=0; l<4; l++){
                        for(b=0; b<128; b++){
                                gwtdata[c][l][b] = (b==1 || b==3)? 1 : 0;
                        }
                }
        }

        while(1==fread(&evt, sizeof(unsigned int), 1, f)){
                if (lastevt != evt){
                        
                        printf("evt %d hits %d nsp %d\n", evt, total_hits, total_nsp);
                        //total_nsp = 0;
                        //total_hits = 0;
                        //fprintf(stderr, "sp in gwtdata\n");
                        
                        // put SPs in gwtdata
                        for(c=0; c<624; c++){
                                unsigned int t = lastevt%512;

                                //struct SP *head = latency_buff[t];
                                struct SP *curSP ;
                                //printList((latency_buff[t]));
                                while((curSP = listSearchAndRemove(c, &(latency_buff[t]))) != NULL){
                                        //if(evt > 510){
                                        //fprintf(stderr, "%d %d -- ", c, t);
                                        //printList(latency_buff[t]);
                                        //}
                                        //put curSP in gwtdata 
                                        if(SP_counts[c] < 4*numLinks(c)){
                                                int curLink = SP_counts[c]/4;
                                                int spPos = SP_counts[c]%4;

                                                for(b=0; b<30; b++){
                                                        unsigned int bit = ((curSP->bank)>>b)&1;  // may need to reverse LSB/MSB ordering to ((curSP->bank)>>(29-b))&1 
                                                        gwtdata[c][curLink][b+8+(30*spPos)] = bit;
                                                }

                                                gwtdata[c][curLink][4+spPos] = curSP->parity;
                                                free(curSP);
                                                SP_counts[c]++;
                                        } else {
                                        // not enough room, move SP to next event. 
                                                
                                                unsigned int tn = (t+1)%512;
                                                curSP->next = latency_buff[tn];
                                                curSP->latency = (curSP->latency)+1;
                                                latency_buff[tn] = curSP;

                                                backpressure[curSP->chip]++;
                                                //fprintf(stderr, "backpressure chip %d latency %d\n", curSP->chip, curSP->latency);
                                        }
                                }
                        }
                        //printf("backpressure ");
                        //for (ii=0; ii<624; ii++){
                                //printf("%3d ", backpressure[ii]);
                        //}
                        //printf("\n");
                        for (ii=0; ii<624; ii++){
                                SP_counts[ii] = 0;
                                backpressure[ii] = 0;
                        }

                //fprintf(stderr, "clear gwtdata\n");
                        // print and clear gwtdata
                        for(c=0; c<624; c++){
                                unsigned nl = numLinks(c);
                                char cbuff[124];
                                for(l=0; l<4; l++){
                                        if(l<nl){
                                                int f1 = 0;
                                                int f2 = 0; 
                                                int f3 = 0;
                                                int f4 = 0;
                                                int sum = 0;
                                                unsigned int datavalid;
                                                for(b=4; b<128; b++){  // leave 0101 at start
                                                        //fprintf(outf, "%d", gwtdata[c][l][b]);
                                                        cbuff[b-4] = 48+gwtdata[c][l][b];
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
                                                sum = f1+f2+f3+f4;
                                                datavalid = (sum>0) ? 1 : 0;
                                                //fprintf(outf, "0101");
                                                //fprintf(outf, "0101%124s %d %d %d %d \n", cbuff, c, sum, evt, nl);

                                                //fprintf(outf, "0101%124s %d \n", cbuff, c);
                                                fprintf(outf, "0101%124s%d %d \n", cbuff, datavalid, c);
                                        }
                                }
                                SP_counts[c] = 0;
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
                        int pix = bank&0xff;
                        int spAddr = bank >> 8 ;
                        int bcid = evt%512;
                        int graybcid = binaryToGray(bcid);
                        //int latency = evt + 8 + spAddr%64;  // this is time of arrival from t=0 first evt.
                        int latency = evt + 8 + spAddr%64;  // this is time of arrival from t=0 first evt.
                        /*int graylatencybcid = binaryToGray(latency%3564) ;  // 3564 BCIDs */
                        int sp = bank|(graybcid<<21);
                        struct SP *newsp = (struct SP*)malloc(sizeof(struct SP));
                        unsigned int parity = 0;

                        //printf("evt %d latency %d sp %d spAddr %d pix %d bcid %d graybcid %d chip %d\n", evt, latency, i, spAddr, pix, bcid, graybcid, chip);

                        if(pix!=0){
                                //fprintf(outf, "%d ", latency);
                                for(j=29; j>=0; j--){
                                        unsigned int bit = (sp>>j)&1;
                                        parity = (j==29) ? bit : bit ^ parity;             // assumes even parity
                                        if (j<8) total_hits += bit;
                                        //gwtdata[30*k + j] = bit;
                                        //fprintf(outf, "%d", bit);
                                }
                                //fprintf(outf, " %d %d\n", parity, chip); 
                        }
                        newsp->latency = latency;
                        newsp->chip = chip;
                        //newsp->graybcid = graybcid;
                        //newsp->spAddr=spAddr;
                        newsp->parity = parity;
                        //newsp->pixels = pix;
                        newsp->bank = sp;
                        newsp->next = latency_buff[latency%512];
                        latency_buff[latency%512] = newsp;




                }
        }

        fclose(f);
        fclose(outf);
        return 0;
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
        if (mod12 == 0 || mod12 == 8) 
                return 4;
        else if (mod12 == 1 || mod12 == 7)
                return 2;
        else
                return 1;
        //new data
        //if (mod12 == 2 || mod12 == 6) 
                //return 4;
        //else if (mod12 == 1 || mod12 == 7)
                //return 2;
        //else
                //return 1;
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
