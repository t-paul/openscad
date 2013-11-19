#
# This script contains functions for building various libraries
# used by OpenSCAD.
# It's supposed to be included from the system specific scripts.
#

build_freetype()
{
  version="$1"
  extra_config_flags="$2"

  if [ -e "$DEPLOYDIR/include/freetype2" ]; then
    echo "freetype already installed. not building"
    return
  fi

  echo "Building freetype $version..."
  cd "$BASEDIR"/src
  rm -rf "freetype-$version"
  if [ ! -f "freetype-$version.tar.gz" ]; then
    curl --insecure -LO "http://download.savannah.gnu.org/releases/freetype/freetype-$version.tar.gz"
  fi
  tar xzf "freetype-$version.tar.gz"
  cd "freetype-$version"
  ./configure --prefix="$DEPLOYDIR" $extra_config_flags
  make -j"$NUMCPU"
  make install
}

build_fontconfig()
{
  version="$1"

  if [ -e $DEPLOYDIR/include/fontconfig ]; then
    echo "fontconfig already installed. not building"
    return
  fi

  echo "Building fontconfig $version..."
  cd "$BASEDIR"/src
  rm -rf "fontconfig-$version"
  if [ ! -f "fontconfig-$version.tar.gz" ]; then
    curl --insecure -LO "http://www.freedesktop.org/software/fontconfig/release/fontconfig-$version.tar.gz"
  fi
  tar xzf "fontconfig-$version.tar.gz"
  cd "fontconfig-$version"
  ./configure --prefix="$DEPLOYDIR"
  make -j$NUMCPU
  make install
}

build_ragel()
{
  version="$1"

  if [ -f $DEPLOYDIR/bin/ragel ]; then
    echo "ragel already installed. not building"
    return
  fi

  echo "Building ragel $version..."
  cd "$BASEDIR"/src
  rm -rf "ragel-$version"
  if [ ! -f "ragel-$version.tar.gz" ]; then
    curl --insecure -LO "http://www.complang.org/ragel/ragel-$version.tar.gz"
  fi
  tar xzf "ragel-$version.tar.gz"
  cd "ragel-$version"
  sed -i -e "s/setiosflags(ios::right)/std::&/g" "ragel/javacodegen.cpp"
  ./configure --prefix="$DEPLOYDIR"
  make -j$NUMCPU
  make install
}

build_harfbuzz()
{
  version="$1"
  extra_config_flags="$2"

  if [ -e $DEPLOYDIR/include/harfbuzz ]; then
    echo "harfbuzz already installed. not building"
    return
  fi

  echo "Building harfbuzz $version..."
  cd "$BASEDIR"/src
  rm -rf "harfbuzz-$version"
  if [ ! -f "harfbuzz-$version.tar.gz" ]; then
    curl --insecure -LO "http://cgit.freedesktop.org/harfbuzz/snapshot/harfbuzz-$version.tar.gz"
  fi
  tar xzf "harfbuzz-$version.tar.gz"
  cd "harfbuzz-$version"
  # Disable doc directories as they make problems on Mac OS Build
  sed -i -e "s/SUBDIRS = src util test docs/SUBDIRS = src util test/g" Makefile.am
  sed -i -e "s/^docs.*$//" configure.ac
  ./autogen.sh --prefix="$DEPLOYDIR" --with-freetype=yes --with-gobject=no --with-cairo=no --with-icu=no $extra_config_flags
  make -j$NUMCPU
  make install
}


build_glib2()
{
  version="$1"
  maj_min_version="${version%.*}" #Drop micro

  if [ -e $DEPLOYDIR/lib/glib-2.0 ]; then
    echo "glib2 already installed. not building"
    return
  fi

  echo "Building glib2 $version..."
  cd "$BASEDIR"/src
  rm -rf "glib-$version"
  if [ ! -f "glib-$version.tar.xz" ]; then
    curl --insecure -LO "http://ftp.gnome.org/pub/gnome/sources/glib/$maj_min_version/glib-$version.tar.xz"
  fi
  tar xJf "glib-$version.tar.xz"
  cd "glib-$version"

  ./configure --prefix="$DEPLOYDIR"
  make -j$NUMCPU
  make install
   
}
