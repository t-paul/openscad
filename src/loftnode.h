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
        std::string scale_x;
        std::string scale_y;
        std::string offset_x;
        std::string offset_y;
        
        std::vector<Value> values_path;
        std::vector<double> values_rotate;
        std::vector<double> values_scale_x;
        std::vector<double> values_scale_y;
        std::vector<double> values_offset_x;
        std::vector<double> values_offset_y;
};

#endif
