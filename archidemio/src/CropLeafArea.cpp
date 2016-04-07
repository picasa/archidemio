/*
 * @@tagdynamic@@
 * @@tagdepends: vle.discrete-time @@endtagdepends
 */

#include <vle/DiscreteTimeDbg.hpp>
#include <vle/devs/DynamicsDbg.hpp>

namespace vd = vle::devs;
namespace vv = vle::value;

namespace archidemio {

using namespace vle::discrete_time;

class CropLeafArea : public DiscreteTimeDyn
{
public:
    CropLeafArea(const vd::DynamicsInit& init, const vd::InitEventList& events)
: DiscreteTimeDyn(init, events)
{
        C_Density = events.getDouble("C_Density");

        LAI.init(this, "LAI", events);
        CropAreaActive.init(this, "CropAreaActive", events);
}

    virtual ~CropLeafArea()
    {}

    virtual void compute(const vd::Time& /*time*/)
    {
        // On utilise directement la surface d'une unité à ce stade,
        // il faudra sommer ces valeurs quand il y aura n strates.

        // Indice de surface foliaire de la culture
        LAI = CropAreaActive() * C_Density;

        // Indice relatif de surface foliaire : indication de l'état de dev de la culture
        // RLAI = CropAreaActive() / P_AreaMax;

        // Surface totale atteinte par le pathogène (infectée, infectieuse, détruite)
        // CropDeseased = AreaDeseased();
    }

private:

    double C_Density;
    //state variable
    Var LAI;
    //inputs
    Var CropAreaActive;
};

DECLARE_DYNAMICS(CropLeafArea)

} // namespaces


