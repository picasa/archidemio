
/*
 * @@tagdynamic@@
 * @@tagdepends: vle.discrete-time @@endtagdepends
 */

#include <vle/DiscreteTimeDbg.hpp>
#include <vle/devs/DynamicsDbg.hpp>

namespace vd = vle::devs;
namespace vv = vle::value;

namespace vle {
namespace discrete_time {
namespace archidemio {

class Initialization : public DiscreteTimeDyn
{
public:
    Initialization(const vd::DynamicsInit& init, const vd::InitEventList& events)
    : DiscreteTimeDyn(init, events)
    {

        // parameters
        E_InitTime = events.getDouble("E_InitTime");
        E_InitQuantity = events.getDouble("E_InitQuantity");

        // state variables
        InitQuantity.init(this, "InitQuantity", events);
        BeginSimulation.init(this, "BeginSimulation", events);

        BeginSimulation.init_value(events.getDouble("begin"));


    }

    virtual ~Initialization()
    {}

    void compute(const vle::devs::Time& t)
    {
        if (t - BeginSimulation() == E_InitTime) {
            InitQuantity = E_InitQuantity;
        } else {
            InitQuantity = 0;
        }
    }

    //parameters
    double E_InitTime;
    double E_InitQuantity;

    //Variables d'etat
    Var InitQuantity;
    Var BeginSimulation;
};

} // namespace archidemio
} // namespace discrete_time
} // namespace vle

DECLARE_DYNAMICS(vle::discrete_time::archidemio::Initialization)

