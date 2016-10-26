
/*
 * @@tagdynamic@@
 * @@tagdepends: vle.discrete-time @@endtagdepends
 */

#include <vle/DiscreteTime.hpp>

namespace vd = vle::devs;
namespace vv = vle::value;

namespace vle {
namespace discrete_time {
namespace archidemio {

class CropClimate : public DiscreteTimeDyn
{
public:
    CropClimate(const vd::DynamicsInit& init, const vd::InitEventList& events)
    : DiscreteTimeDyn(init, events)
    {

        // Lecture des parametres
        E_TempInfectionOpt = vle::value::toDouble(events.get("E_TempInfectionOpt"));
        E_TempInfectionWidth = vle::value::toDouble(events.get("E_TempInfectionWidth"));

        // Variables d'etat gerees par ce composant CropClimate
        ActionRH.init(this, "ActionRH", events);
        ActionTemp.init(this, "ActionTemp", events);

        // Variables  gérées par un autre  composant
        RH.init(this, "RH", events);
        TempMean.init(this, "TempMean", events);


    }

    virtual ~CropClimate()
    {}

    void compute(const vle::devs::Time& t)
    {
        //std::cout << "CropClimate compute begin" << std::endl;

        /* Effet de la température (locale ou globale) sur la dispersion
         * ActionTemp <- function (T, a){pmax(1 - (0.0022 * (T - a)^2),0)}
         */
        ActionTemp = fmax(1 - (E_TempInfectionWidth * pow((TempMean() - E_TempInfectionOpt),2)),0);

        /* Effet de la l'hygrométrie (locale ou globale) sur la dispersion
         */
        ActionRH = 0.0;
    }


    //Parametres du modele
    double E_TempInfectionOpt; /**< Paramètre: Température optimale pour le dev. du pathogène, Unite: degres */
    double E_TempInfectionWidth; /**< Paramètre: Plage de la réponse à la température pour le dev. du pathogène, Unite: SD */

    //Variables d'etat
    Var ActionTemp; /**< Variable d'état: Effet de la température sur le pathogène */
    Var ActionRH; /**< Variable d'état: Effet de RH sur le pathogène*/

    //Entrées
    Var TempMean;//sync
    Var RH;//sync

};

} // namespace archidemio
} // namespace discrete_time
} // namespace vle

DECLARE_DYNAMICS(vle::discrete_time::archidemio::CropClimate)

