freetype {
  QMAKE_CXXFLAGS += `pkg-config --cflags freetype2 harfbuzz`
  LIBS += `pkg-config --libs freetype2 harfbuzz`
}

