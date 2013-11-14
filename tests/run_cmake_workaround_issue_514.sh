
echo "Execute CMake and then override"
cmake .


  #Don't use gcc 4.8 it will cause a crash :( -- is it only on linux?
  CC=gcc
  CXX=g++
  if [ "`$CC --version|grep 4.8`" ]; then
      echo "WARNING:gcc 4.8 causes a seg fault in CGAL -- see https://github.com/openscad/openscad/issues/514"
      if [ "`gcc-4.7 --version|grep 4.7`" ]; then
           echo "INFO:gcc 4.7 is installed -- let's use that instead (just for building CGAL and OpenSCAD)"



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


      fi
  fi

echo "GCC hacking done if required."
