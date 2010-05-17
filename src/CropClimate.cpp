/**
 * @file src/CropClimate.cpp
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

namespace Crop {

    class CropClimate: public vle::extension::DifferenceEquation::Multiple
    {
	public:
	    CropClimate(const vle::devs::DynamicsInit& init,
			  const vle::devs::InitEventList& events) :
	    vle::extension::DifferenceEquation::Multiple(init, events)
	    {
		// Variables d'etat gerees par ce composant CropClimate
		ActionRH = createVar("ActionRH");
		ActionTemp = createVar("ActionTemp");

		// Variables  gérées par un autre  composant
		RH = createSync("RH"); 
		TempMean = createSync("TempMean");

		// Lecture des parametres
		M_TempOpt = vle::value::toDouble(events.get("M_TempOpt"));
	    }


	    virtual ~CropClimate() {};


    	    virtual void compute(const vle::devs::Time& /*time*/)
	    {
		//std::cout << "CropClimate compute begin" << std::endl;
		ActionTemp = 1 - (0.0022 * pow((TempMean() - M_TempOpt),2));
		ActionRH = 0.0;
	    }

    	    virtual void initValue(const vle::devs::Time& /* time */)
    	    {
		//std::cout << "CropClimate init begin" << std::endl;
		ActionTemp = 0.0;
		ActionRH = 0.0;
	    }


        private:
	    //Variables d'etat
	    Var ActionTemp; /**< Variable d'état: Effet de la température sur le pathogène */
	    Var ActionRH; /**< Variable d'état: Effet de RH sur le pathogène*/

	    //Entrées
	    Sync TempMean;
	    Sync RH;

	    //Parametres du modele
	    double M_TempOpt; /**< Paramètre: Température optimale pour le dev. du pathogène, Unite: degres */

    };
}
DECLARE_NAMED_DYNAMICS(CropClimate, Crop::CropClimate); // balise specifique VLE

