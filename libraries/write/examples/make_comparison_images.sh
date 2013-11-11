#!/bin/bash

echo "Making comparison images"


#TestFont.scad -- This is more an exposition than a comparison

declare -a names=( TestSurfaceText.scad  TestWriteCube.scad  TestWriteCylinder.scad  TestWrite.scad  TestWriteSphere.scad TestFont.scad )


for i in ${names[@]}
do

sed "s/write\/write.scad/write_original_v3\/write.scad/g" $i > $i.old.tmp
../../../openscad -o $i.png  $i &
../../../openscad -o $i.old.png  $i.old.tmp &

done
wait

for i in ${names[@]}
do

rm $i.old.tmp

done


