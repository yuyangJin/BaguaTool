#!/bin/bash
#psgDir="/mnt/home/jinyuyang/ScalAna/src_2020/static"
psgDir=${BAGUA_DIR}/src/staticAnalysis
f=$1
awk -F ' ' '{printf("%s %s\n", $5, $6)}'  ./$f.psg > psgaddr.txt
cat psgaddr.txt  | xargs addr2line -p -C -e ./$f  > $f.psg-symb
awk -F ' ' '{print $1}'  ./$f.psg-symb > $f.psg-symb-1
python $psgDir/cpsfile.py $f
#awk 'NR%2==1'  $f.psg-symb-1 > $f.psg-symb-1-1
#awk 'NR%2==0'  $f.psg-symb-1 > $f.psg-symb-1-2
sed -i "1i\ \t"  $f.psg-symb
#sed -i "1i\ \t"  $f.psg-symb-1-2
#cut -f1 $f.psg-symb-1-1 | paste $f.psg - > $f.psg-f-0
cp $f.psg $f.psg.cp
awk -F ' ' '{print $1,$2,$3,$4}'  ./$f.psg > $f.psg-wo-addr
cut -f "1,2,3,4" $f.psg-symb | paste $f.psg-wo-addr - > $f.psg
cat $f.psg-df >> $f.psg

rm -f psgaddr.txt $f.psg-symb*
