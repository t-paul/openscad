#!/bin/bash

echo "Making comparison images"


#TestFont.scad -- This is more an exposition than a comparison

declare -a names=( TestSurfaceText.scad  TestWriteCube.scad  TestWriteCylinder.scad  TestWrite.scad  TestWriteSphere.scad TestFont.scad )

#Make temp files that use the old write and insert a version number
for i in ${names[@]}
do
  
sed -e "s/write\/write.scad/write_original_v3\/write.scad/g" -e "s/write_lib_version = 4.0/write_lib_version = 3.0/g" $i  > $i.old.tmp
../../../openscad -o $i.png  $i &
../../../openscad -o $i.old.png  $i.old.tmp &

done
wait

#Remove the temp files
for i in ${names[@]}
do

rm $i.old.tmp

done


