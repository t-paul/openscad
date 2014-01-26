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

        int max_idx;
        double slices;
        double height;
        std::string path;
        std::string rotate;
        std::string scale;
        std::string offset;
        
        std::vector<Value> values_path;
        std::vector<Value> values_path_tangent;
        std::vector<double> values_rotate;
        std::vector<Value> values_scale;
        std::vector<Value> values_offset;
};

#endif
