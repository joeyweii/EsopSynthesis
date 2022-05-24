true > exorcism.log
for i in {0..23};
do
./abc -c "&exorcism -Q 0 -V 1 ${i}.esop ${i}_out.esop" &>> exorcism.log
done
