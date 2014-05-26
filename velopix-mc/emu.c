#include <stdio.h>

inline unsigned int binaryToGray(unsigned int num);
unsigned int grayToBinary(unsigned int num);

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
        unsigned int gwtdata[120];

        while(1==fread(&evt, sizeof(unsigned int), 1, f)){
                printf("evt %d\n", evt);
                if (feof(f)) break;
                fread(&chip, sizeof(unsigned int), 1, f);
                fread(&banksize, sizeof(unsigned int), 1, f);
                fread(&nsp, sizeof(unsigned int), 1, f);
                /*if (evt > 512) break;*/
                for(i=0; i<nsp; i++){
                        fread(&bank, sizeof(unsigned int), 1, f);
                        int pix = bank&0xff;
                        int spAddr = bank >> 8 ;
                        int bcid = evt%512;
                        int graybcid = binaryToGray(bcid);
                        int latency = evt + 8 + spAddr%64;  // this is time of arrival from t=0 first evt.
                        /*int graylatencybcid = binaryToGray(latency%3564) ;  // 3564 BCIDs */
                        int sp = bank|(graybcid<<21);
                        if(pix!=0){
                                unsigned int parity;
                                fprintf(outf, "%d ", latency); 
                                for(j=29; j>=0; j--){
                                        unsigned int bit = (sp>>j)&1;
                                        parity = (j==29) ? bit : bit ^ parity;             // assumes even parity
                                        gwtdata[30*k + j] = bit;
                                        fprintf(outf, "%d", bit);
                                }
                                fprintf(outf, " %d %d\n", parity, chip); 
                                

                                /*paritybits = paritybits | (parity<<k);
                                if(k==3){
                                        fprintf(outf, "0000"); //gwt header
                                        int c;
                                        fprintf(outf, " ");  // just to guide the eye
                                        for(c=3; c>=0; c--){
                                                fprintf(outf, "%d", (paritybits>>c)&1);
                                        }
                                        fprintf(outf, " ");  // just to guide the eye
                                        for(c=119; c>=0; c--){
                                                fprintf(outf, "%d", gwtdata[c]); //data
                                                if( c%30 ==0 ){
                                                        fprintf(outf, " ");  // just to guide the eye
                                                }
                                        }
                                        fprintf(outf, "\n"); //eof

                                        paritybits = 0;
                                        k = 0;
                                
                                } else {
                                        k++;
                                }
                                */

                        }
                        
                        /*printf(" %x %x ", bank, bcid);*/
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
