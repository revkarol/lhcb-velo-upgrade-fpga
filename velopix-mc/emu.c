#include <stdio.h>

inline unsigned int binaryToGray(unsigned int num);
unsigned int grayToBinary(unsigned int num);
unsigned int numLinks(unsigned int chip);
struct SP* listSearchAndRemove(unsigned int chip, struct SP **head);

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


int main(int argc, char **argv)
{
        FILE *f = fopen("boole_mc_data.bin", "rb");
        FILE *outf = fopen("spix-latency-gray.txt", "w");
        int nevts = 10;
        int chip = -1;
        int banksize = -1;
        int nsp = 0;
        int bank ; 
        int evt ; 
        int i,j;
        int k=0;
        unsigned char paritybits = 0; 
        //unsigned int gwtdata[120];
        unsigned char gwtdata[624][4][128];  //chips, links, bits
        int *latency_buff[512];
        unsigned int SP_counts[624];
        int tl=0;
        int last_evt=0;
        int c,l,b = 0; //chips, links, bits


        int ii;
        for (ii=0; ii<512; ii++){
                latency_buff[ii] = NULL;
        }
        for (ii=0; ii<624; ii++){
                SP_counts[ii] = 0;
        }

        for(int c=0; c<624; c++){
                for(int l=0; l<4; l++){
                        for(int b=0; b<128; b++){
                                gwtdata[c][l][b] = (b==1 || b==3)? 1 : 0;
                        }
                }
        }

        while(1==fread(&evt, sizeof(unsigned int), 1, f)){
                printf("evt %d\n", evt);
                if (lastevt != evt){
                        
                        for(c=0; c<624; c++){
                                unsigned int t = lastevt%512;

                                struct SP *head = latency_buff[t];
                                struct SP *curSP ;
                                while((curSP = listSearchAndRemove(c, head)) != NULL){
                                        //put curSP in gwtdata 
                                        if(SP_counts[c] < 4*numLinks[c]){
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
                                                curSP->next = latency_buff[t+1];
                                                curSP->latency = curSP->latency+1;
                                                latency_buff[t+1] = curSP;
                                        }
                                }
                        }

                        for(int c=0; c<624; c++){
                                for(int l=0; l<4; l++){
                                        for(int b=4; b<128; b++){  // leave 0101 at start
                                                if(l<numLinks(c)){
                                                        fprintf(outf, "%d", gwtdata[c][l][b]);
                                                        gwtdata[c][l][b] = 0;
                                                }
                                                fprintf(" %d \n", c);
                                        }
                                }
                                SP_counts[c] = 0;
                        }

                }
                if (feof(f)) break;
                fread(&chip, sizeof(unsigned int), 1, f);
                fread(&banksize, sizeof(unsigned int), 1, f);
                fread(&nsp, sizeof(unsigned int), 1, f);

                for(i=0; i<nsp; i++){
                        fread(&bank, sizeof(unsigned int), 1, f);
                        int pix = bank&0xff;
                        int spAddr = bank >> 8 ;
                        int bcid = evt%512;
                        int graybcid = binaryToGray(bcid);
                        int latency = evt + 8 + spAddr%64;  // this is time of arrival from t=0 first evt.
                        /*int graylatencybcid = binaryToGray(latency%3564) ;  // 3564 BCIDs */
                        int sp = bank|(graybcid<<21);
                        struct SP *newsp = (struct SP*)malloc(sizeof(struct SP));
                        if(pix!=0){
                                unsigned int parity;
                                fprintf(outf, "%d ", latency); 
                                for(j=29; j>=0; j--){
                                        unsigned int bit = (sp>>j)&1;
                                        parity = (j==29) ? bit : bit ^ parity;             // assumes even parity
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
                        newsp->next = latency_buff[latency];
                        latency_buff[latency] = newsp;




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

unsigned int numLinks(unsigned int chip)
{
        unsigned int mod12 = chip%12;
        if (mod12 == 0 || mod12 == 8) 
                return 4;
        else if (mod12 == 1 || mod12 == 7)
                return 2;
        else
                return 1;
}

struct SP* listSearchAndRemove(unsigned int chip, struct SP **head)
{
        struct SP *p = head;
        struct SP *prev = NULL;
        bool found = false;
        while(p != NULL){
                if(p->chip == chip){
                        if(prev != NULL){
                                prev->next = p->next;
                        } else {
                                head = p->next;
                        }
                        return p;
                } else {
                        prev = p;
                        p = p->next;
                }
        }
        return NULL;
        
}
