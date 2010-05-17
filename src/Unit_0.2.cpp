/**
  * @file UnitPathogen.cpp
  * @author P. Casadebaig-(The RECORD team -INRA )
  */

/*
 * Copyright (C) 2009 INRA
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <vle/extension/DifferenceEquation.hpp>
#include <vle/extension/DifferenceEquationDbg.hpp>

namespace vd = vle::devs;
namespace ve = vle::extension;
namespace vv = vle::value;

namespace Crop {

class Unit : public ve::DifferenceEquation::Multiple
{
public:
    Unit(
       const vd::DynamicsInit& atom,
       const vd::InitEventList& evts)
        : ve::DifferenceEquation::Multiple(atom, evts)
    {
        // Parametres
        P_AreaMax = vv::toDouble(evts.get("P_AreaMax"));
		P_UnitTTExp = vv::toDouble(evts.get("P_UnitTTExp"));
		P_UnitTTSen = vv::toDouble(evts.get("P_UnitTTSen"));
		P_SlopeExpansion = vv::toDouble(evts.get("P_SlopeExpansion"));
		P_SlopeSenescence = vv::toDouble(evts.get("P_SlopeExpansion"));
        E_Init = vv::toDouble(evts.get("E_Init"));
        E_RateDeseaseTransmission = vv::toDouble(evts.get("E_RateDeseaseTransmission"));
        E_InfectiousPeriod = vv::toDouble(evts.get("E_InfectiousPeriod"));
        E_LatentPeriod = vv::toDouble(evts.get("E_LatentPeriod"));
        
        // Variables synchrones
		TempEff = createSync("TempEff"); 
		ThermalTime = createSync("ThermalTime"); 
        ActionTemp = createSync("ActionTemp");
        
        // Variables d'état
        RateAreaExpansion = createVar("RateAreaExpansion");
        RateAreaSenescence = createVar("RateAreaSenescence");
		AreaExpansion = createVar("AreaExpansion");
		AreaActive = createVar("AreaActive");
        AreaHealthy = createVar("AreaHealthy");
        AreaLatent = createVar("AreaLatent");
        AreaInfectious = createVar("AreaInfectious");        
        AreaRemoved = createVar("AreaRemoved");
        AreaSenescence = createVar("AreaSenescence");
        RateDeseaseTransmission = createVar("RateDeseaseTransmission");
    }

    virtual ~Unit()
    {}

//@@begin:compute@@
virtual void compute(const vd::Time& /*time*/)
{
    // Vitesse de croissance
    RateAreaExpansion = TempEff() * (P_AreaMax * P_SlopeExpansion) * exp(-P_SlopeExpansion * (ThermalTime() - P_UnitTTExp)) / pow((1 + exp(-P_SlopeExpansion * (ThermalTime() - P_UnitTTExp))),2);
    
    // Vitesse de senescence (ne pas utiliser P_AreaMax si la taille finale n'est pas atteinte)
    RateAreaSenescence = TempEff() * (P_AreaMax * P_SlopeSenescence) * exp(-P_SlopeSenescence * (ThermalTime() - P_UnitTTSen)) / pow((1 + exp(-P_SlopeSenescence * (ThermalTime() - P_UnitTTSen))),2);
    
    // Surface totale : intégration de RateAreaExpansion
    AreaExpansion = AreaExpansion(-1) + RateAreaExpansion();
    
    // Surface senescente : intégration de RateAreaSenescence
    AreaSenescence = AreaSenescence(-1) + RateAreaSenescence();
    
    // Surface active : différence totale - senescence
    AreaActive = AreaExpansion() - AreaSenescence();

    // Vitesse d'infection, affectée par le microclimat et la géométrie du couvert
    RateDeseaseTransmission = E_RateDeseaseTransmission * ActionTemp();
    
    // Surface saine, création par la croissance, destruction par l'infection
    AreaHealthy = AreaHealthy(-1) +RateAreaExpansion() -(RateDeseaseTransmission() * AreaHealthy(-1));
    
    // Surface latente, création par l'infection, destruction par la propagation du pathogene
    AreaLatent = AreaLatent(-1) -(E_LatentPeriod * AreaLatent(-1)) +(E_RateDeseaseTransmission * AreaHealthy(-1));
    
    // Surface infectieuse, creation par la propagation du pathogene, destruction par le pathogene
    AreaInfectious = AreaInfectious(-1) -(E_InfectiousPeriod * AreaInfectious(-1)) +(E_LatentPeriod * AreaLatent(-1));
    
    // Surface détruite par le pathogène et la senescence naturelle
    AreaRemoved = E_InfectiousPeriod * AreaInfectious(-1);  
     

}
//@@end:compute@@

//@@begin:initValue@@
virtual void initValue(const vd::Time& /*time*/)
{
    RateAreaExpansion = 0.0;
    RateAreaSenescence = 0.0;
    AreaExpansion = 0.0;
    AreaActive = 0.0;
    AreaHealthy = 0.0;
    AreaLatent = 0.0;
    AreaInfectious = 0.0;
    AreaRemoved = 0.0;
    AreaSenescence = 0.0;
    RateDeseaseTransmission = 0.0;    
}
//@@end:initValue@@

private:
//@@begin:user@@
//@@end:user@@

    double P_AreaMax; /**< Paramètre : Surface potentielle de la strate, Unité : m^2 */
    double P_UnitTTExp;
    double P_UnitTTSen;
    double P_SlopeExpansion;
    double P_SlopeSenescence  ;  
    double E_Init;
    double E_RateDeseaseTransmission;
    double E_InfectiousPeriod;
    double E_LatentPeriod;
    
    
    Sync ActionTemp;
    Sync TempEff;
    Sync ThermalTime;
    
    Var RateAreaExpansion;
    Var RateAreaSenescence;
    Var AreaExpansion;
    Var AreaActive;
    Var AreaHealthy;
    Var AreaLatent;
    Var AreaInfectious;
    Var AreaRemoved;
    Var AreaSenescence;
    Var RateDeseaseTransmission;
    
};

} // namespace Unit

DECLARE_DIFFERENCE_EQUATION_MULTIPLE_DBG(Crop::Unit)

