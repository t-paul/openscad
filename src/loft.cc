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
		<< "offset_x = " << offset_x
		<< ", offset_y = " << offset_y
		<< ")";

	return stream.str();
}

class LoftModule : public AbstractModule
{
public:
	LoftModule() { }
	virtual AbstractNode *instantiate(const Context *ctx, const ModuleInstantiation *inst, const EvalContext *evalctx) const;
};

AbstractNode *LoftModule::instantiate(const Context *ctx, const ModuleInstantiation *inst, const EvalContext *evalctx) const
{
	LoftNode *node = new LoftNode(inst);

	AssignmentList args;

	Context c(ctx);
	c.setVariables(args, evalctx);

	node->slices = c.lookup_variable("slices").toDouble();
	node->offset_x = c.lookup_variable("offset_x").toString();
	node->offset_y = c.lookup_variable("offset_y").toString();

	std::vector<AbstractNode *> instantiatednodes = inst->instantiateChildren(evalctx);
	node->children.insert(node->children.end(), instantiatednodes.begin(), instantiatednodes.end());

	for (double v = 0;v < 1;v += 1/node->slices) {
		AssignmentList func_args;
		std::string var("x");
		func_args += Assignment(var, new Expression(Value(v)));
		EvalContext func_context(evalctx, func_args);
		Value val_x = func_context.evaluate_function(node->offset_x, &func_context);
		Value val_y = func_context.evaluate_function(node->offset_y, &func_context);
		std::cout << "x(" << v << "): " << val_x
			<< ", y(" << v << "): " << val_y
			<< std::endl;
		
		node->values_x.push_back(val_x.toDouble());
		node->values_y.push_back(val_y.toDouble());
	}
	
	return node;
}

void register_builtin_loft()
{
	Builtins::init("loft", new LoftModule());
}
