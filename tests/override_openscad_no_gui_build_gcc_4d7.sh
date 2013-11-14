
echo "Can't override cmake in this directory -- CC=gcc-4.7 CXX=gxx-4.7 cmake doesn't work"
echo "We get a little hacky!! -- see https://github.com/openscad/openscad/issues/514"
echo "/usr/bin/c++ -> g++-4.7 "


declare -a directories=(
tests-cgal.dir
tests-nocgal.dir
cgalcachetest.dir
csgtexttest.dir
modulecachetest.dir
tests-common.dir
tests-offscreen.dir
openscad_nogui.dir
test_pretty_print.dir
tests-core.dir
)

declare -a files=(link.txt build.make)

for d in ${directories[@]}
do

for f in ${files[@]}
do
    echo $d/$f
    mv CMakeFiles/$d/$f CMakeFiles/$d/$f.bak
    sed s/"\/usr\/bin\/c++"/"g++-4.7"/g CMakeFiles/$d/$f.bak > CMakeFiles/$d/$f
done

done

