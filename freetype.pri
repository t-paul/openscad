freetype {
  QMAKE_CXXFLAGS += `pkg-config --cflags freetype2`
  LIBS += `pkg-config --libs freetype2`
}

