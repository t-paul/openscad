/*
 *  OpenSCAD (www.openscad.org)
 *  Copyright (C) 2009-2011 Clifford Wolf <clifford@clifford.at> and
 *                          Marius Kintel <marius@kintel.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  As a special exception, you have permission to link this program
 *  with the CGAL library and distribute executables, as long as you
 *  follow the requirements of the GNU GPL in regard to all of the
 *  software in the executable aside from CGAL.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "expression.h"
#include "value.h"
#include "evalcontext.h"
#include <assert.h>
#include <sstream>
#include <algorithm>
#include "stl-utils.h"
#include "printutils.h"
#include <boost/bind.hpp>
#include <boost/foreach.hpp>

/*Unicode support for string lengths and array accesses - use Harbuzz library functions (already in project for text render)*/
#include <hb.h>

Expression::Expression() : recursioncount(0)
{
}

Expression::Expression(const std::string &type, 
											 Expression *left, Expression *right)
	: type(type), recursioncount(0)
{
	this->children.push_back(left);
	this->children.push_back(right);
}

Expression::Expression(const std::string &type, Expression *expr)
	: type(type), recursioncount(0)
{
	this->children.push_back(expr);
}

Expression::Expression(const Value &val) : const_value(val), type("C"), recursioncount(0)
{
}

Expression::~Expression()
{
	std::for_each(this->children.begin(), this->children.end(), del_fun<Expression>());
}

class FuncRecursionGuard
{
public:
	FuncRecursionGuard(const Expression &e) : expr(e) { 
		expr.recursioncount++; 
	}
	~FuncRecursionGuard() { expr.recursioncount--; }
	bool recursion_detected() const { return (expr.recursioncount > 1000); }
private:
	const Expression &expr;
};

Value Expression::evaluate(const Context *context) const
{
	if (this->type == "!")
		return ! this->children[0]->evaluate(context);
	if (this->type == "&&")
		return this->children[0]->evaluate(context) && this->children[1]->evaluate(context);
	if (this->type == "||")
		return this->children[0]->evaluate(context) || this->children[1]->evaluate(context);
	if (this->type == "*")
		return this->children[0]->evaluate(context) * this->children[1]->evaluate(context);
	if (this->type == "/")
		return this->children[0]->evaluate(context) / this->children[1]->evaluate(context);
	if (this->type == "%")
		return this->children[0]->evaluate(context) % this->children[1]->evaluate(context);
	if (this->type == "+")
		return this->children[0]->evaluate(context) + this->children[1]->evaluate(context);
	if (this->type == "-")
		return this->children[0]->evaluate(context) - this->children[1]->evaluate(context);
	if (this->type == "<")
		return this->children[0]->evaluate(context) < this->children[1]->evaluate(context);
	if (this->type == "<=")
		return this->children[0]->evaluate(context) <= this->children[1]->evaluate(context);
	if (this->type == "==")
		return this->children[0]->evaluate(context) == this->children[1]->evaluate(context);
	if (this->type == "!=")
		return this->children[0]->evaluate(context) != this->children[1]->evaluate(context);
	if (this->type == ">=")
		return this->children[0]->evaluate(context) >= this->children[1]->evaluate(context);
	if (this->type == ">")
		return this->children[0]->evaluate(context) > this->children[1]->evaluate(context);
	if (this->type == "?:") {
		return this->children[this->children[0]->evaluate(context) ? 1 : 2]->evaluate(context);
	}
	if (this->type == "[]") {
#if 1 //INBUILT_STR_UNICODE_AWARE
		int index = (int) this->children[1]->evaluate(context).toDouble();

		Value thing_to_access = this->children[0]->evaluate(context);
		Value evaluated_value = thing_to_access[index];

		//If it is a string -- let's treat it as though it is unicode (as byte chars are a subset)
		if(thing_to_access.type() == Value::STRING){

			std::string text = this->children[0]->evaluate(context).toString();
			//PRINTB_NOCACHE("Unicode for index %u in text '%s'", this->children[1]->evaluate(context) % text);

			hb_buffer_t *hb_buf = hb_buffer_create();
			hb_buffer_add_utf8(hb_buf, text.c_str(), strlen(text.c_str()), 0, strlen(text.c_str()));
			unsigned int glyph_count = hb_buffer_get_length ( hb_buf );
			hb_glyph_info_t *glyph_info = hb_buffer_get_glyph_infos(hb_buf, &glyph_count);

			//We have the codepoint now...
			uint32_t /*FT_UInt*/ glyph_cp_of_interest = glyph_info[ index ].codepoint;
			//PRINTB_NOCACHE("Codepoint %u (0x%x) at index %u in text '%s'", glyph_cp_of_interest % glyph_cp_of_interest % index % text);
			//....BUT
			//There is no reverse lookup to get back to the chars in Harbuzz
			//http://mces.blogspot.in/2009/11/pango-vs-harfbuzz.html
			//- HarfBuzz doesn't provide: A Unicode Bidirection Algorithm implementation
			//GLIB has one @g_unichar_to_utf8(), but we don't have glib here...
			//So, let's encode the chars the old-fashioned way...
			//TODO: Add a 'real' implementation for unicode/utf8 conversion
			/*
			 * From the man page on utf8
			Encoding
			   The following byte sequences are used to represent a character.
			   The sequence to be used depends on the UCS code number of the character:

			   0x00000000 - 0x0000007F:
				   0xxxxxxx

			   0x00000080 - 0x000007FF:
				   110xxxxx 10xxxxxx

			   0x00000800 - 0x0000FFFF:
				   1110xxxx 10xxxxxx 10xxxxxx

			   0x00010000 - 0x001FFFFF:
				   11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

			   0x00200000 - 0x03FFFFFF:
				   111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx

			   0x04000000 - 0x7FFFFFFF:
				   1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx

			   The xxx bit positions are filled with the bits of the
			   character code number in binary representation.  Only the
			   shortest possible multibyte sequence which can represent the
			   code number of the character can be used.

			   The UCS code values 0xd800–0xdfff (UTF-16 surrogates) as well
			   as 0xfffe and 0xffff (UCS noncharacters) should not appear in
			   conforming UTF-8 streams.
			   */
				#define BYTE_10XXXXXX  (0x80)
				#define BYTE_110XXXXX  (0xC0)
				#define BYTE_1110XXXX  (0xE0)

				#define BYTE_11110XXX  (0xF0)
				#define BYTE_111110XX  (0xF8)
				#define BYTE_1111110X  (0xFC)

				#define BYTE_MASK_SIX_LSB (0x3F)

				std::string utf8_of_cp=""; //Create the utf8 string into this.

				if(glyph_cp_of_interest<=0x7F){
					utf8_of_cp += glyph_cp_of_interest;
				}else if(glyph_cp_of_interest<=0x7FF){
					utf8_of_cp += (((glyph_cp_of_interest >>  6)                    )   |BYTE_110XXXXX);
					utf8_of_cp += (((glyph_cp_of_interest >>  0) & BYTE_MASK_SIX_LSB)	|BYTE_10XXXXXX);
				}else if(glyph_cp_of_interest<=0xFFFF){
					utf8_of_cp += (((glyph_cp_of_interest >> 12)                    )	|BYTE_1110XXXX);
					utf8_of_cp += (((glyph_cp_of_interest >>  6) & BYTE_MASK_SIX_LSB)	|BYTE_10XXXXXX);
					utf8_of_cp += (((glyph_cp_of_interest >>  0) & BYTE_MASK_SIX_LSB)	|BYTE_10XXXXXX);
				}else if(glyph_cp_of_interest<=0x1FFFFF){
					utf8_of_cp += (((glyph_cp_of_interest >> 18)	                )	|BYTE_11110XXX);
					utf8_of_cp += (((glyph_cp_of_interest >> 12) & BYTE_MASK_SIX_LSB)	|BYTE_10XXXXXX);
					utf8_of_cp += (((glyph_cp_of_interest >>  6) & BYTE_MASK_SIX_LSB)	|BYTE_10XXXXXX);
					utf8_of_cp += (((glyph_cp_of_interest >>  0) & BYTE_MASK_SIX_LSB)	|BYTE_10XXXXXX);
				}else if(glyph_cp_of_interest<=0x03FFFFFF){
					utf8_of_cp += (((glyph_cp_of_interest >> 24)	                )	|BYTE_111110XX);
					utf8_of_cp += (((glyph_cp_of_interest >> 18) & BYTE_MASK_SIX_LSB)	|BYTE_10XXXXXX);
					utf8_of_cp += (((glyph_cp_of_interest >> 12) & BYTE_MASK_SIX_LSB)	|BYTE_10XXXXXX);
					utf8_of_cp += (((glyph_cp_of_interest >>  6) & BYTE_MASK_SIX_LSB)	|BYTE_10XXXXXX);
					utf8_of_cp += (((glyph_cp_of_interest >>  0) & BYTE_MASK_SIX_LSB)	|BYTE_10XXXXXX);
				}else if(glyph_cp_of_interest<=0x7FFFFFFF){
					utf8_of_cp += (((glyph_cp_of_interest >> 30)                    )	|BYTE_1111110X);
					utf8_of_cp += (((glyph_cp_of_interest >> 24) & BYTE_MASK_SIX_LSB)	|BYTE_10XXXXXX);
					utf8_of_cp += (((glyph_cp_of_interest >> 18) & BYTE_MASK_SIX_LSB)	|BYTE_10XXXXXX);
					utf8_of_cp += (((glyph_cp_of_interest >> 12) & BYTE_MASK_SIX_LSB)	|BYTE_10XXXXXX);
					utf8_of_cp += (((glyph_cp_of_interest >>  6) & BYTE_MASK_SIX_LSB)	|BYTE_10XXXXXX);
					utf8_of_cp += (((glyph_cp_of_interest >>  0) & BYTE_MASK_SIX_LSB)	|BYTE_10XXXXXX);
				}


			//Return the string of utf8 chars for this requested 'character' index into the string
			evaluated_value = utf8_of_cp;

			hb_buffer_destroy(hb_buf);
		}
		return evaluated_value;
#else
		return this->children[0]->evaluate(context)[this->children[1]->evaluate(context)];
#endif

	}
	if (this->type == "I")
		return -this->children[0]->evaluate(context);
	if (this->type == "C")
		return this->const_value;
	if (this->type == "R") {
		Value v1 = this->children[0]->evaluate(context);
		Value v2 = this->children[1]->evaluate(context);
		Value v3 = this->children[2]->evaluate(context);
		if (v1.type() == Value::NUMBER && v2.type() == Value::NUMBER && v3.type() == Value::NUMBER) {
			Value::RangeType range(v1.toDouble(), v2.toDouble(), v3.toDouble());
			return Value(range);
		}
		return Value();
	}
	if (this->type == "V") {
		Value::VectorType vec;
		BOOST_FOREACH(const Expression *e, this->children) {
			vec.push_back(e->evaluate(context));
		}
		return Value(vec);
	}
	if (this->type == "L")
		return context->lookup_variable(this->var_name);
	if (this->type == "N")
	{
		Value v = this->children[0]->evaluate(context);

		if (v.type() == Value::VECTOR && this->var_name == "x")
			return v[0];
		if (v.type() == Value::VECTOR && this->var_name == "y")
			return v[1];
		if (v.type() == Value::VECTOR && this->var_name == "z")
			return v[2];

		if (v.type() == Value::RANGE && this->var_name == "begin")
			return Value(v[0]);
		if (v.type() == Value::RANGE && this->var_name == "step")
			return Value(v[1]);
		if (v.type() == Value::RANGE && this->var_name == "end")
			return Value(v[2]);

		return Value();
	}
	if (this->type == "F") {
		FuncRecursionGuard g(*this);
		if (g.recursion_detected()) { 
			PRINTB("ERROR: Recursion detected calling function '%s'", this->call_funcname);
			return Value();
		}

		EvalContext c(context, this->call_arguments);
		return context->evaluate_function(this->call_funcname, &c);
	}
	abort();
}

std::string Expression::toString() const
{
	std::stringstream stream;

	if (this->type == "*" || this->type == "/" || this->type == "%" || this->type == "+" ||
			this->type == "-" || this->type == "<" || this->type == "<=" || this->type == "==" || 
			this->type == "!=" || this->type == ">=" || this->type == ">" ||
			this->type == "&&" || this->type == "||") {
		stream << "(" << *this->children[0] << " " << this->type << " " << *this->children[1] << ")";
	}
	else if (this->type == "?:") {
		stream << "(" << *this->children[0] << " ? " << *this->children[1] << " : " << *this->children[2] << ")";
	}
	else if (this->type == "[]") {
		stream << *this->children[0] << "[" << *this->children[1] << "]";
	}
	else if (this->type == "I") {
		stream << "-" << *this->children[0];
	}
	else if (this->type == "!") {
		stream << "!" << *this->children[0];
	}
	else if (this->type == "C") {
		stream << this->const_value;
	}
	else if (this->type == "R") {
		stream << "[" << *this->children[0] << " : " << *this->children[1] << " : " << *this->children[2] << "]";
	}
	else if (this->type == "V") {
		stream << "[";
		for (size_t i=0; i < this->children.size(); i++) {
			if (i > 0) stream << ", ";
			stream << *this->children[i];
		}
		stream << "]";
	}
	else if (this->type == "L") {
		stream << this->var_name;
	}
	else if (this->type == "N") {
		stream << *this->children[0] << "." << this->var_name;
	}
	else if (this->type == "F") {
		stream << this->call_funcname << "(";
		for (size_t i=0; i < this->call_arguments.size(); i++) {
			const Assignment &arg = this->call_arguments[i];
			if (i > 0) stream << ", ";
			if (!arg.first.empty()) stream << arg.first  << " = ";
			stream << *arg.second;
		}
		stream << ")";
	}
	else {
		assert(false && "Illegal expression type");
	}

	return stream.str();
}

std::ostream &operator<<(std::ostream &stream, const Expression &expr)
{
	stream << expr.toString();
	return stream;
}
