mkdir -p bychip
mkdir -p bychip_datavalid

for i in `seq 0 623` ; do 
        #zcat spix-latency-gray.txt.gz | grep " $i\$"  | sort -n | ./desync-gwt.py > bychip/desync$i.txt  
        #grep " $i " spix-latency-gray.txt | sed 's/ *//' > bychip/desync$i.txt  
        grep " $i *$" spix-latency-gray-datavalid.txt | sed 's/ .*$//' > bychip_datavalid/desync$i.txt  
        #grep " $i " spix-timeordered-gray-datavalid.txt | sed 's/ .*$//' > for_stefie/desync$i.txt  
        echo $i
done
