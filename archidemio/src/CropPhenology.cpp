
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

class CropPhenology : public DiscreteTimeDyn
{
public:
    CropPhenology(const vd::DynamicsInit& init, const vd::InitEventList& events)
    : DiscreteTimeDyn(init, events)
    {

        // Lecture des parametres
        P_TempBase = events.getDouble("P_TempBase");

        // Variables d'etat gerees par ce composant CropPhenology
        ThermalTime.init(this, "ThermalTime", events);
        TempEff.init(this, "TempEff", events);
        TempMean.init(this, "TempMean", events);

        // Variables  gérées par un autre  composant ici "meteo"
        TempMin.init(this, "TempMin", events);
        TempMax.init(this, "TempMax", events);

    }

    virtual ~CropPhenology()
    {}

    void compute(const vle::devs::Time& t)
    {
        //std::cout << "CropPhenology compute begin" << std::endl;
        TempMean = (TempMax() + TempMin()) / 2;

        double TempEff_tmp = TempMean() - P_TempBase;
        if (TempMean() < P_TempBase) {
            TempEff_tmp = 0.0;
        }

        TempEff = TempEff_tmp;

        ThermalTime = ThermalTime(-1) + TempEff();
    }

    //Parametres du modele
    double P_TempBase; /**< Paramètre: Température de base, Unite: degres */

    //Entrées
    Var TempMin;//sync
    Var TempMax;//sync

    //Variables d'etat
    Var ThermalTime; /**< Variable d'état: Temps thermique */
    Var TempMean; /**< Variable d'état: Température air moyenne (parcelle) */
    Var TempEff; /**< Température efficace pour la croissance */

};

} // namespace archidemio
} // namespace discrete_time
} // namespace vle

DECLARE_DYNAMICS(vle::discrete_time::archidemio::CropPhenology)

