/**
 * @file src/UnitLeafArea.cpp
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

namespace Unit {

    class UnitLeafArea: public vle::extension::DifferenceEquation::Multiple
    {
	public:
	    UnitLeafArea(const vle::devs::DynamicsInit& init,
			  const vle::devs::InitEventList& events) :
	    vle::extension::DifferenceEquation::Multiple(init, events)
	    {
		// Variables d'etat gerees par ce composant UnitLeafArea
		RateAreaExpansion = createVar("RateAreaExpansion");
		RateAreaSenescence = createVar("RateAreaSenescence");		
		AreaExpansion = createVar("AreaExpansion");
		AreaSenescence = createVar("AreaSenescence");
		AreaActive = createVar("AreaActive");
		
		// Variables  gérées par un autre  composant
		TempEff = createSync("TempEff"); 
		ThermalTime = createSync("ThermalTime"); 
		
		// Lecture des parametres (certains deviendront des variables ensuite)
		// P_Phyllochron = vle::value::toDouble(events.get("P_Phyllochron"));
		P_AreaMax = vle::value::toDouble(events.get("P_AreaMax"));
		P_UnitTTExp = vle::value::toDouble(events.get("P_UnitTTExp"));
		P_UnitTTSen = vle::value::toDouble(events.get("P_UnitTTSen"));
		P_SlopeExpansion = vle::value::toDouble(events.get("P_SlopeExpansion"));
		P_SlopeSenescence = vle::value::toDouble(events.get("P_SlopeExpansion"));
		
	    }


	    virtual ~UnitLeafArea() {};


    	    virtual void compute(const vle::devs::Time& /*time*/)
	    {
		//std::cout << "UnitLeafArea compute begin" << std::endl;
		// Vitesse de croissance
		// Teff * (PhytoSmax[i] * P_ke) * exp(-P_ke * (TT - PhytoTTExp[i])) / (1 + exp(-P_ke * (TT - PhytoTTExp[i])))^2
		RateAreaExpansion = TempEff() * (P_AreaMax * P_SlopeExpansion) * exp(-P_SlopeExpansion * (ThermalTime() - P_UnitTTExp)) / pow((1 + exp(-P_SlopeExpansion * (ThermalTime() - P_UnitTTExp))),2);
		
		// Vitesse de senescence (ne pas utiliser P_AreaMax si la taille finale n'est pas atteinte)
		RateAreaSenescence = TempEff() * (P_AreaMax * P_SlopeSenescence) * exp(-P_SlopeSenescence * (ThermalTime() - P_UnitTTSen)) / pow((1 + exp(-P_SlopeSenescence * (ThermalTime() - P_UnitTTSen))),2);
		
		// Surface totale : intégration de RateAreaExpansion
		AreaExpansion = AreaExpansion(-1) + RateAreaExpansion();
		
		// Surface senescente : intégration de RateAreaSenescence
		AreaSenescence = AreaSenescence(-1) + RateAreaSenescence();
		
		// Surface active : différence totale - senescence
		AreaActive = AreaExpansion() - AreaSenescence();
	    }

    	    virtual void initValue(const vle::devs::Time& /* time */)
    	    {
		//std::cout << "UnitLeafArea init begin" << std::endl;
		RateAreaExpansion = 0.0;
		RateAreaSenescence = 0.0;
		AreaExpansion = 0.0;
		AreaSenescence = 0.0;
		AreaActive = 0.0;
	    }


        private:
	    //Variables d'etat
	    Var RateAreaExpansion; /**< Variable d'état: Vitesse d'expansion de la surface d'une strate, Unité : m^2/jour*/
	    Var RateAreaSenescence; /**< Variable d'état: Vitesse d'expansion de la surface d'une strate, Unité : m^2/jour*/
	    Var AreaExpansion; /**< Variable d'état: Surface foliaire totale, Unité : m^2*/
	    Var AreaSenescence; /**< Variable d'état: Surface foliaire senescente, Unité : m^2*/
	    Var AreaActive; /**< Variable d'état: Surface foliaire active de la strate, Unité : m^2*/
	    
	    //Entrées
	    Sync TempEff;
	    Sync ThermalTime;

	    //Parametres du modele
	    // double P_Phyllochron; /**< Paramètre: Phyllochrone d'apparition des strates, Unite: degres.jours */
	    double P_AreaMax; /**< Paramètre : Surface potentielle de la strate, Unité : m^2 */
	    double P_UnitTTExp; /**< Paramètre : Date de demi-expansion de la strate, Unité : degres.jours */
	    double P_UnitTTSen; /**< Paramètre : Date de demi-senescence de la strate, Unité : degres.jours */
	    double P_SlopeExpansion; /**< Paramètre : Pente de l'expansion */
	    double P_SlopeSenescence; /**< Paramètre : Pente de la sénescence */
    };
}
DECLARE_NAMED_DYNAMICS(UnitLeafArea, Unit::UnitLeafArea); // balise specifique VLE

