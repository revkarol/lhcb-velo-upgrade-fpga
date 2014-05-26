#include <stdio.h>

int main(int argc, char **argv)
{
        FILE *f = fopen("boole_mc_data.bin", "rb");
        int nevts = 10;
        int chip = -1;
        int banksize = -1;
        int nsp = 0;
        int bank ; 
        int evt ; 
        int i,j;

        while(1==fread(&evt, sizeof(unsigned int), 1, f)){
                if (feof(f)) break;
                fread(&chip, sizeof(unsigned int), 1, f);
                fread(&banksize, sizeof(unsigned int), 1, f);
                fread(&nsp, sizeof(unsigned int), 1, f);
                printf("evt %d chip %d banksize %d nsp %d\n", evt, chip, banksize, nsp);

                /*if(i==0)*/
                for(i=0; i<nsp; i++){
                        printf("   new sp \n");
                        fread(&bank, sizeof(unsigned int), 1, f);
                        int pix = bank&0xff;
                        int spAddr = bank >> 8 ;
                        int row = 4*(spAddr%64);
                        int col = ((spAddr-row/4)/32);

                        for (j=0; j<8; j++){
                                if(((pix>>j) & 0x1) == 1){

                                        printf("          row %d col %d spaddr %x pix %x\n", row+(j%4), col+j/4, spAddr, bank&0xff);
                                }
                        }
                        /*printf(" %x", bank);*/
                }
                printf("\n");
        }

        fclose(f);
        return 0;
}
