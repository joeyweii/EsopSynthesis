benchmarks="multiplier"

ks="23 24 25"
for K in  ${ks};
do
    for circuit in ${benchmarks};
    do
        echo -n ${circuit} >> exp2.csv
        echo -n "," >> exp2.csv
        echo -n ${K} >> exp2.csv
        ../timeout -t 4800 --memlimit-rss 20971560 ../abc2_bdd -c "read ../benchmarks/EPFL_suite/${circuit}.aig; resyn2; resyn2; if -K ${K} -a; bddextract -u" &>> exp2.log
        ../timeout -t 4800 --memlimit-rss 20971560 ../abc2 -c "read ../benchmarks/EPFL_suite/${circuit}.aig; resyn2; resyn2; if -K ${K} -a; bddextract -u" &>> exp2.log
        ../timeout -t 4800 --memlimit-rss 20971560 ../abc2 -c "read ../benchmarks/EPFL_suite/${circuit}.aig; resyn2; resyn2; if -K ${K} -a; prunedextract -u -l 8" &>> exp2.log
        ../timeout -t 4800 --memlimit-rss 20971560 ../abc2 -c "read ../benchmarks/EPFL_suite/${circuit}.aig; resyn2; resyn2;  if -K ${K} -a; prunedextract -u -l 4" &>> exp2.log
        ../timeout -t 4800 --memlimit-rss 20971560 ../abc2 -c "read ../benchmarks/EPFL_suite/${circuit}.aig; resyn2; resyn2;  if -K ${K} -a; arextract -u -l 4" &>> exp2.log
        echo "" >> exp2.csv
    done
done
