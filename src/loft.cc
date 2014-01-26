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

#include "loftnode.h"

#include "expression.h"
#include "evalcontext.h"
#include "typedefs.h"
#include "module.h"
#include "evalcontext.h"
#include "printutils.h"
#include "builtin.h"
#include "PolySetEvaluator.h"

#include <sstream>
#include <boost/assign/std/vector.hpp>
using namespace boost::assign; // bring 'operator+=()' into scope

LoftNode::LoftNode(const ModuleInstantiation *mi) : AbstractPolyNode(mi)
{
}

Response LoftNode::accept(class State &state, Visitor &visitor) const
{
	return visitor.visit(state, *this);
}

std::string LoftNode::name() const
{
	return "loft";
}

class PolySet *LoftNode::evaluate_polyset(PolySetEvaluator *evaluator) const
{
	if (!evaluator) {
		PRINTB("WARNING: No suitable PolySetEvaluator found for %s module!", this->name());
		return NULL;
	}

	print_messages_push();

	PolySet *ps = evaluator->evaluatePolySet(*this);

	print_messages_pop();

	return ps;
}

std::string LoftNode::toString() const
{
	std::stringstream stream;

	stream << this->name() << "("
		<< "path = " << path
		<< ", rotate = " << rotate
		<< ", scale_x = " << scale_x
		<< ", scale_y = " << scale_y
		<< ", offset_x = " << offset_x
		<< ", offset_y = " << offset_y
                << ", slices = " << slices
                << ", height = " << height
		<< ")";

	return stream.str();
}

class LoftModule : public AbstractModule
{
public:
	LoftModule() { }
	virtual AbstractNode *instantiate(const Context *ctx, const ModuleInstantiation *inst, const EvalContext *evalctx) const;
private:
	virtual void evaluate(Value &val, std::vector<double> &vec, EvalContext &ctx) const;
	virtual void evaluate(Value &val, std::vector<Value> &vec, EvalContext &ctx) const;
	virtual void calculate_tangent(const EvalContext *evalctx, Value &func, std::vector<Value> &vec, double t) const;
	virtual Value evaluate(const EvalContext *evalctx, std::string func, double x) const;
};

AbstractNode *LoftModule::instantiate(const Context *ctx, const ModuleInstantiation *inst, const EvalContext *evalctx) const
{
	LoftNode *node = new LoftNode(inst);

	AssignmentList args;

	Context c(ctx);
	c.setVariables(args, evalctx);

        node->height = c.lookup_variable("height").toDouble();
	node->slices = c.lookup_variable("slices").toDouble();
	Value path = c.lookup_variable("path");
	Value rotate = c.lookup_variable("rotate");
	Value scale_x = c.lookup_variable("scale_x");
	Value scale_y = c.lookup_variable("scale_y");
	Value offset_x = c.lookup_variable("offset_x");
	Value offset_y = c.lookup_variable("offset_y");
	
	node->path = path.toString();
	node->rotate = rotate.toString();
	node->scale_x = scale_x.toString();
	node->scale_y = scale_y.toString();
	node->offset_x = offset_x.toString();
	node->offset_y = offset_y.toString();

	std::vector<AbstractNode *> instantiatednodes = inst->instantiateChildren(evalctx);
	node->children.insert(node->children.end(), instantiatednodes.begin(), instantiatednodes.end());

	node->max_idx = floor(node->slices);
        for (int a = 0;a <= node->max_idx;a++) {
                double v = (double)a / node->max_idx;
		calculate_tangent(evalctx, path, node->values_path_tangent, v);
		AssignmentList func_args;
		std::string var("x");
		func_args += Assignment(var, new Expression(Value(v)));
		EvalContext func_context(evalctx, func_args);
		evaluate(path, node->values_path, func_context);
		evaluate(rotate, node->values_rotate, func_context);
		evaluate(scale_x, node->values_scale_x, func_context);
		evaluate(scale_y, node->values_scale_y, func_context);
		evaluate(offset_x, node->values_offset_x, func_context);
		evaluate(offset_y, node->values_offset_y, func_context);
	}
	
	return node;
}

void LoftModule::evaluate(Value &func, std::vector<double> &vec, EvalContext &ctx) const
{
	if (func.type() != Value::STRING) {
		return;
	}
	
	Value val = ctx.evaluate_function(func.toString(), &ctx);
	vec.push_back(val.toDouble());
}

void LoftModule::evaluate(Value &func, std::vector<Value> &vec, EvalContext &ctx) const
{
	if (func.type() != Value::STRING) {
		return;
	}
	
	Value val = ctx.evaluate_function(func.toString(), &ctx);
	vec.push_back(val);
}

void LoftModule::calculate_tangent(const EvalContext *evalctx, Value &func, std::vector<Value> &vec, double t) const
{
	if (func.type() != Value::STRING) {
		return;
	}
	
	std::string func_name = func.toString();
	
	double h = 1e-9;
	while (true) {
		double x0, y0, z0, x1, y1, z1;
		Value v0 = evaluate(evalctx, func_name, t - h);
		Value v1 = evaluate(evalctx, func_name, t + h);
		v0.getVec3(x0, y0, z0);
		v1.getVec3(x1, y1, z1);
		double x = (x1 - x0) / (2 * h);
		double y = (y1 - y0) / (2 * h);
		double z = (z1 - z0) / (2 * h);
		double l = x * x + y * y + z * z;
		if ((l >= 1) || (h >= 1)) {
			Value::VectorType v;
			v.push_back(Value(x));
			v.push_back(Value(y));
			v.push_back(Value(z));
			vec.push_back(Value(v));
			break;
		}
		h *= 2;
	}
}

Value LoftModule::evaluate(const EvalContext *evalctx, std::string func, double x) const
{
	AssignmentList func_args;
	std::string var("x");
	func_args += Assignment(var, new Expression(Value(x)));
	EvalContext func_context(evalctx, func_args);
	Value val = func_context.evaluate_function(func, &func_context);
	return val;
}

void register_builtin_loft()
{
	Builtins::init("loft", new LoftModule());
}
