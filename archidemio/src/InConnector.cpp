/*
 * @@tagdynamic@@
 * @@tagdepends: vle.discrete-time @@endtagdepends
 */

#include <vle/utils/DateTime.hpp>
#include <vle/devs/Executive.hpp>
#include <vle/discrete-time/DiscreteTimeExec.hpp>
#include "GraphTranslator.hpp"

#include <fstream>

namespace vd = vle::devs;
namespace vv = vle::value;

namespace vle {
namespace discrete_time {
namespace archidemio {

class InConnector : public DiscreteTimeExec
{
public:
    InConnector(const vle::devs::ExecutiveInit& init,
            const vd::InitEventList& events):
                DiscreteTimeExec(init, events), initialized(false)
    {
    }

    virtual ~InConnector() {


    }
    void compute(const vle::devs::Time& t)
    {
        if (not initialized){
            // Ajout des connexions modele couple -> modele sum
            const vle::vpz::ConnectionList& inputs = coupledmodel().getInputPortList();
            vle::vpz::ConnectionList::const_iterator it;

            /* DEBUG : affiche les connections
        for (it = inputs.begin(); it != inputs.end(); ++it) {
            std::cout << "model: " << getModel().getParentName() << it->first << std::endl;
        }*/

            for (it = inputs.begin(); it != inputs.end(); ++it) {
                if (it->first.size() >= 5 and it->first.compare(0, 5, "node-") == 0) {
                    addInputPort("GenericSum", it->first);
                    addConnection(coupledmodelName(), it->first,
                            "GenericSum", it->first);
                    // mPortList.push_back(it->first);
                }
            }
            initialized = true;
        }
    };
    bool initialized;

};

DECLARE_EXECUTIVE(InConnector)

}}}
