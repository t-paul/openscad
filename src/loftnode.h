#ifndef LOFTNODE_H_
#define LOFTNODE_H_

#include <vector>

#include "node.h"
#include "visitor.h"
#include "value.h"

class LoftNode : public AbstractPolyNode
{
public:
	LoftNode(const ModuleInstantiation *mi);
        virtual Response accept(class State &state, Visitor &visitor) const;
	virtual std::string toString() const;
	virtual std::string name() const;

	virtual PolySet *evaluate_polyset(class PolySetEvaluator *) const;

        double slices;
        std::string offset_x;
        std::string offset_y;
        
        std::vector<double> values_x;
        std::vector<double> values_y;
};

#endif
