#ifndef FREETYPERENDERER_H
#define	FREETYPERENDERER_H

#include <string>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "DrawingCallback.h"

class FreetypeRenderer {
public:
    FreetypeRenderer();
    virtual ~FreetypeRenderer();

    void render(DrawingCallback *cb, std::string text, std::string font, double size) const;
private:
    const static double scale = 1000;
    
    FT_Library library;
    
    static int Outline_MoveToFunc(const FT_Vector *to, void *user);
    static int Outline_LineToFunc(const FT_Vector *to, void *user);
    static int Outline_ConicToFunc(const FT_Vector *c1, const FT_Vector *to, void *user);
    static int Outline_CubicToFunc(const FT_Vector *c1, const FT_Vector *c2, const FT_Vector *to, void *user);
};

#endif	/* FREETYPERENDERER_H */

