
/*
 * @@tagdynamic@@
 * @@tagdepends: vle.discrete-time @@endtagdepends
 */

#include <vle/utils/DateTime.hpp>
#include <vle/devs/Executive.hpp>
#include <vle/discrete-time/DiscreteTimeExec.hpp>

#include <fstream>

namespace vd = vle::devs;
namespace vv = vle::value;

namespace vle {
namespace discrete_time {
namespace archidemio {

class UnitPilot : public DiscreteTimeExec
{
public:
    enum CropsStates {DEV, VEGETATIVE, REPRODUCITVE};

    UnitPilot(const vle::devs::ExecutiveInit& init,
            const vd::InitEventList& events): DiscreteTimeExec(init, events)
    {
        //init var
        Index.init(this, "Index", events);
        PreformedTT.init(this, "PreformedTT", events);
        CropState.init(this, "CropState", events);

        //init inputs
        ThermalTime.init(this, "ThermalTime", events);

        // Tuple pour une liste de dates d'initiation (P_InitiationTT)
        const vle::value::Tuple& p = events.getTuple("P_InitiationTT");

        for (unsigned int i=0; i<p.size(); i++) {
            P_InitiationTT.push_back(p.at(i));
        }

        //parameters
        P_ReproductiveTT = events.getDouble("P_ReproductiveTT");

        //initialization
        Index.init_value(1.0);
        CropState.init_value(DEV);
    }

    virtual ~UnitPilot()
    {}

    void compute(const vle::devs::Time& t)
    {
        //development condition
        if (P_InitiationTT.size() >= 2) {
            if (Index(-1) <= P_InitiationTT.size() -1 ) {

                if ( ThermalTime() > Index(-1) * P_InitiationTT[int(Index(-1)-1)]) {
                    Index = Index(-1) + 1;
                }
            } else {
                PreformedTT = std::accumulate(
                        P_InitiationTT.begin(),
                        P_InitiationTT.end()-1,0);
                if (ThermalTime() >  PreformedTT() +
                                     (Index(-1)-3)*P_InitiationTT.back()) {
                    Index = Index(-1) + 1;
                }
            }
        } else {
            if(ThermalTime() > (Index(-1)+1) * P_InitiationTT[0]) {
                Index = Index(-1) + 1;
            }
        }

        if (Index() != Index(-1)) {
            //has development, compute state
            if (ThermalTime() > P_ReproductiveTT) {
                //is flowering
                CropState = REPRODUCITVE;
            } else {
                if (Index(-1) >= P_InitiationTT.size() - 1) {
                    CropState = VEGETATIVE;
                } else {
                    CropState = DEV;
                }
            }
            //add a unit if required
            if (CropState() != REPRODUCITVE) {
                std::string previous((vle::fmt("Unit_%1%") % (Index(-1)-1)).str());
                std::string current((vle::fmt("Unit_%1%") % (Index()-1)).str());

                createModelFromClass("UnitClass", current);
                addConnection("CropClimate", "ActionTemp",
                              current, "ActionTemp");
                addConnection("CropPhenology", "TempEff",
                              current, "TempEff");
                addConnection("CropPhenology", "ThermalTime",
                              current, "ThermalTime");
                addConnection("Initialization", "InitQuantity",
                              current, "InitQuantity");
                if (Index()-1 > 1) {
                    addConnection(previous, "Out", current, "In");
                }

                addInputPort("GenericSum",
                        (vle::fmt("%1%_AreaActive") % current).str());

                addConnection(current, "AreaActive", "GenericSum",
                        (vle::fmt("%1%_AreaActive") % current).str());

                std::ofstream file("archidemio/exp/output.vpz");
                dump(file, "dump");
            }
        }
    }

    //internal var
    Var Index;
    Var PreformedTT;
    Var CropState;

    //inputs
    Var ThermalTime;//sync

    //parameters
    std::vector < unsigned int > P_InitiationTT;
    double P_ReproductiveTT;


};

} // namespace archidemio
} // namespace discrete_time
} // namespace vle

DECLARE_EXECUTIVE(vle::discrete_time::archidemio::UnitPilot)

