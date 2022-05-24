
for nVars in {23..24};
do
    for seed in {6..10};
    do
        nTerms=3000
        echo -n ${nVars}_${seed} >> exp1.csv 
        ./random_onset in1.pla ${nVars} ${nTerms} ${seed}
        ./timeout -t 10800 --memlimit-rss 62914560 ./abc -c "read in1.pla; resyn2; resyn2; resyn2; bddextract exp1.csv" &>> exp1.log
        ./timeout -t 10800 --memlimit-rss 62914560 ./abc -c "read in1.pla; resyn2; resyn2; resyn2; prunedextract -l 8 exp1.csv" &>> exp1.log
        ./timeout -t 10800 --memlimit-rss 62914560 ./abc -c "read in1.pla; resyn2; resyn2; resyn2; arextract -l 4 exp1.csv" &>> exp1.log
        ./timeout -t 10800 --memlimit-rss 62914560 ./abc_random -c "read in1.pla; resyn2; resyn2; resyn2; prunedextract exp1.csv" &>> exp1.log
        echo "" >> exp1.csv
    done
done

for nVars in {25..27};
do
    for seed in {6..10};
    do
        nTerms=3000
        echo -n ${nVars}_${seed} >> exp1.csv 
        ./random_onset in1.pla ${nVars} ${nTerms} ${seed}
        ./timeout -t 10800 --memlimit-rss 62914560 ./abc -c "read in1.pla; resyn2; resyn2; resyn2; prunedextract -l 8 exp1.csv" &>> exp1.log
        ./timeout -t 10800 --memlimit-rss 62914560 ./abc -c "read in1.pla; resyn2; resyn2; resyn2; arextract -l 4 exp1.csv" &>> exp1.log
        ./timeout -t 10800 --memlimit-rss 62914560 ./abc_random -c "read in1.pla; resyn2; resyn2; resyn2; prunedextract exp1.csv" &>> exp1.log
        echo "" >> exp1.csv
    done
done