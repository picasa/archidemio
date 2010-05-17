/**
  * @file UnitPathogen.cpp
  * @author ...
  * ...
  * @@tag DifferenceEquationMultiple@@namespace:Unit;class:UnitPathogen;par:E_Init,*|E_RateDeseaseTransmission,*|E_InfectiousPeriod,*|E_LatentPeriod,*|;sync:RateAreaExpansion|;nosync:@@end tag@@
  */

#include <vle/extension/DifferenceEquation.hpp>
#include <vle/extension/DifferenceEquationDbg.hpp>

namespace vd = vle::devs;
namespace ve = vle::extension;
namespace vv = vle::value;

namespace Unit {

class UnitPathogen : public ve::DifferenceEquation::Multiple
{
public:
    UnitPathogen(
       const vd::DynamicsInit& atom,
       const vd::InitEventList& evts)
        : ve::DifferenceEquation::Multiple(atom, evts)
    {
        E_Init = vv::toDouble(evts.get("E_Init"));
        E_RateDeseaseTransmission = vv::toDouble(evts.get("E_RateDeseaseTransmission"));
        E_InfectiousPeriod = vv::toDouble(evts.get("E_InfectiousPeriod"));
        E_LatentPeriod = vv::toDouble(evts.get("E_LatentPeriod"));
        P_AreaMax = vv::toDouble(evts.get("P_AreaMax"));
        RateAreaExpansion = createSync("RateAreaExpansion");
        RateAreaSenescence = createSync("RateAreaSenescence");
        AreaSenescence = createSync("AreaSenescence");
        ActionTemp = createSync("ActionTemp");
        AreaHealthy = createVar("AreaHealthy");
        AreaLatent = createVar("AreaLatent");
        AreaInfectious = createVar("AreaInfectious");
        AreaRemoved = createVar("AreaRemoved");
        RateDeseaseTransmission = createVar("RateDeseaseTransmission");
    }

    virtual ~UnitPathogen()
    {}

//@@begin:compute@@
virtual void compute(const vd::Time& /*time*/)
{
    // Vitesse d'infection, affectée par le microclimat et la géométrie du couvert
    RateDeseaseTransmission = E_RateDeseaseTransmission * ActionTemp();
    
    // Surface saine, création par la croissance, destruction par l'infection
    AreaHealthy = AreaHealthy(-1) +RateAreaExpansion() -(RateDeseaseTransmission() * AreaHealthy(-1));
    
    // Surface infectée, création par l'infection, destruction par la propagation du pathogène
    AreaLatent = AreaLatent(-1) -(E_LatentPeriod * AreaLatent(-1)) +(E_RateDeseaseTransmission * AreaHealthy(-1));
    
    // Surface infectieuse, création par la propagation du pathogène, destruction par le pathogène
    AreaInfectious = AreaInfectious(-1) -(E_InfectiousPeriod * AreaInfectious(-1)) +(E_LatentPeriod * AreaLatent(-1));
    
    // Surface détruite par le pathogène
    AreaRemoved = E_InfectiousPeriod * AreaInfectious(-1);  
    
     

}
//@@end:compute@@

//@@begin:initValue@@
virtual void initValue(const vd::Time& /*time*/)
{
    AreaHealthy = 0.0; 
    AreaLatent = 0.0; 
    AreaInfectious = 0.0; 
    AreaRemoved = 0.0; 
        
}
//@@end:initValue@@

private:
//@@begin:user@@
//@@end:user@@

    double E_Init;
    double E_RateDeseaseTransmission;
    double E_InfectiousPeriod;
    double E_LatentPeriod;
    double P_AreaMax; /**< Paramètre : Surface potentielle de la strate, Unité : m^2 */
    Sync RateAreaExpansion;
    Sync RateAreaSenescence;
    Sync ActionTemp;
    Sync AreaSenescence;
    Var AreaHealthy;
    Var AreaLatent;
    Var AreaInfectious;
    Var AreaRemoved;
    Var RateDeseaseTransmission;
};

} // namespace Unit

DECLARE_DIFFERENCE_EQUATION_MULTIPLE_DBG(Unit::UnitPathogen)

