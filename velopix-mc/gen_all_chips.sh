for i in `seq 0 623` ; do 
        zcat spix-latency-gray.txt.gz | grep "$i\$"  | sort -n | ./desync-gwt.py > /home/karol/Dropbox/bigdata/bychip/desync$i.txt  
done
