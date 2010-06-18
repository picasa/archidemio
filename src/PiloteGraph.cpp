#include <vle/extension/fsa/Statechart.hpp>
#include <vle/extension/DifferenceEquation.hpp>
#include <vle/devs/DynamicsDbg.hpp>

namespace vd = vle::devs;
namespace ve = vle::extension;
namespace vv = vle::value;

class PiloteGraph : public vle::devs::Dynamics
{
public:
    PiloteGraph(const vle::devs::DynamicsInit& init,
              const vle::devs::InitEventList& events) :
	vle::devs::Dynamics(init, events)
    {	}
    
    vle::devs::Time init(const vle::devs::Time& /* time */)
    {
        return 0;
    }
    
    void output(const vle::devs::Time& /* time */,
                vle::devs::ExternalEventList& output) const
	{	
		vd::ExternalEvent* ee=new vd::ExternalEvent("add");

		vle::value::Map* mp = new vle::value::Map ();
        mp->addString("prefix", "node");
        mp->addInt("number", 6);
        mp->addString("port", "in-out");
        
        // 6 noeuds, 1 avec 2 voisins
        mp->addString("adjacency matrix",
                     " 0 1 0 0 0 0"
                     " 0 0 1 0 0 0"
                     " 0 0 0 1 0 0"
                     " 0 0 0 0 1 0"
                     " 0 0 0 0 0 1"
                     " 0 0 0 0 0 0");
        mp->addString("classes", "Unit Unit Unit Unit Unit Unit");
		
		ee << vd::attribute("parameter", mp);
   		output.addEvent(ee);
	}
		
	    vle::devs::Time timeAdvance() const
    {
		return vle::devs::Time::infinity;
    }
	
    virtual ~PiloteGraph() { }

};

DECLARE_NAMED_DYNAMICS_DBG(PiloteGraph, PiloteGraph)


