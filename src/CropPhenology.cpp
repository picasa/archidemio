/**
 * @file src/CropPhenology.cpp
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


#include <vle/extension/DifferenceEquationDbg.hpp>

using namespace std;

namespace Crop {

    class CropPhenology: public vle::extension::DifferenceEquation::Multiple
    {
	public:
	    CropPhenology(const vle::devs::DynamicsInit& init,
			  const vle::devs::InitEventList& events) :
	    vle::extension::DifferenceEquation::Multiple(init, events)
	    {
		// Variables d'etat gerees par ce composant CropPhenology
		ThermalTime = createVar("ThermalTime");
		TempEff = createVar("TempEff");
		TempMean = createVar("TempMean");

		// Variables  gérées par un autre  composant ici "meteo"
		TempMin = createSync("TempMin"); 
		TempMax = createSync("TempMax");

		// Lecture des parametres
		P_TempBase = events.getDouble("P_TempBase");
	    }


	    virtual ~CropPhenology() {};


    	    virtual void compute(const vle::devs::Time& /*time*/)
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

    	    virtual void initValue(const vle::devs::Time& /* time */)
    	    {
		//std::cout << "CropPhenology init begin" << std::endl;
		ThermalTime = 0.0;
		TempMean = 0.0;
		TempEff = 0.0;
	    }


        private:
	    //Variables d'etat
	    Var ThermalTime; /**< Variable d'état: Temps thermique */
	    Var TempMean; /**< Variable d'état: Température air moyenne (parcelle) */
	    Var TempEff; /**< Température efficace pour la croissance */

	    //Entrées
	    Sync TempMin;
	    Sync TempMax;

	    //Parametres du modele
	    double P_TempBase; /**< Paramètre: Température de base, Unite: degres */

    };
}
DECLARE_NAMED_DIFFERENCE_EQUATION_MULTIPLE_DBG(CropPhenology, Crop::CropPhenology); // balise specifique VLE


