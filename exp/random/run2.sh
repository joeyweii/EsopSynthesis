
for nVars in {23..24};
do
    for seed in {1..5};
    do
        nTerms=3000
        echo -n ${nVars}_${seed} >> exp2.csv 
        ./random_sop in2.pla ${nVars} ${nTerms} ${seed}
        ./timeout -t 10800 --memlimit-rss 62914560 ./abc -c "read in2.pla; resyn2; resyn2; resyn2; bddextract exp2.csv" &>> exp2.log
        ./timeout -t 10800 --memlimit-rss 62914560 ./abc -c "read in2.pla; resyn2; resyn2; resyn2; prunedextract -l 8 exp2.csv" &>> exp2.log
        ./timeout -t 10800 --memlimit-rss 62914560 ./abc -c "read in2.pla; resyn2; resyn2; resyn2; arextract -l 4 exp2.csv" &>> exp2.log
        ./timeout -t 10800 --memlimit-rss 62914560 ./abc_random -c "read in2.pla; resyn2; resyn2; resyn2; prunedextract exp2.csv" &>> exp2.log
        echo "" >> exp2.csv
    done
done

for nVars in {25..27};
do
    for seed in {1..5};
    do
        nTerms=3000
        echo -n ${nVars}_${seed} >> exp2.csv 
        ./random_sop in2.pla ${nVars} ${nTerms} ${seed}
        ./timeout -t 10800 --memlimit-rss 62914560 ./abc -c "read in2.pla; resyn2; resyn2; resyn2; prunedextract -l 8 exp2.csv" &>> exp2.log
        ./timeout -t 10800 --memlimit-rss 62914560 ./abc -c "read in2.pla; resyn2; resyn2; resyn2; arextract -l 4 exp2.csv" &>> exp2.log
        ./timeout -t 10800 --memlimit-rss 62914560 ./abc_random -c "read in2.pla; resyn2; resyn2; resyn2; prunedextract exp2.csv" &>> exp2.log
        echo "" >> exp2.csv
    done
done
