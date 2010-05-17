/**
 * @file src/LayerPhenology.cpp
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

namespace Layer {

    class LayerPhenology: public vle::extension::DifferenceEquation::Multiple
    {
	public:
	    LayerPhenology(const vle::devs::DynamicsInit& init,
			  const vle::devs::InitEventList& events) :
	    vle::extension::DifferenceEquation::Multiple(init, events)
	    {
		// Variables d'etat gerees par ce composant LayerPhenology
		LayerTTApp = createVar("LayerTTApp");
		LayerTTSen = createVar("LayerTTSen");
		LayerTTExp = createVar("LayerTTExp");
		LayerAge = createVar("LayerAge");
		LayerNb = createVar("LayerNb");

		// Variables  gérées par un autre  composant ici "meteo"
		TempEff = createSync("TempEff"); 

		// Lecture des parametres
		P_Phyllochron = vle::value::toDouble(events.get("P_Phyllochron"));
		P_LayerExp = vle::value::toDouble(events.get("P_LayerExp"));
		P_LayerSen = vle::value::toDouble(events.get("P_LayerSen"));
	    }


	    virtual ~LayerPhenology() {};


    	    virtual void compute(const vle::devs::Time& /*time*/)
	    {
		//std::cout << "LayerPhenology compute begin" << std::endl;
		// Date d'apparition des strates
		for ( int i=1; i<50; i++){
		    LayerTTApp[i] = i * P_Phyllochron;
		}
		
		// Date de demi-expansion des strates
		for ( int i=1; i<50; i++){
		    LayerTTExp[i] = LayerTTApp[i] + P_LayerExp;
		}
		
		// Date de demi-senescence des strates
		for ( int i=1; i<50; i++){
		    LayerTTSen[i] = LayerTTExp[i] + P_LayerSen;
		}
	    }

    	    virtual void initValue(const vle::devs::Time& /* time */)
    	    {
		//std::cout << "LayerPhenology init begin" << std::endl;
		LayerTTApp = 0.0;
		LayerTTExp = 0.0;
		LayerTTSen = 0.0;
	    }


        private:
	    //Variables d'etat
	    Var LayerTTApp; /**< Variable d'état:  */
	    Var LayerTTExp; /**< Variable d'état:  */
	    Var LayerTTSen; /**< Variable d'état:  */

	    //Entrées
	    Sync TempEff;

	    //Parametres du modele
	    double P_Phyllochron; /**< Paramètre: Phyllochrone d'apparition des strates, Unite: degres.jours */
	    double P_LayerExp; /**< Paramètre: Temps apparition - demi-expansion d'une strate, Unite: degres.jours */
	    double P_LayerSen; /**< Paramètre: Temp demi-expansion - demi-senescence, Unite: degres.jours */
    };
}
DECLARE_NAMED_DYNAMICS(LayerPhenology, Layer::LayerPhenology); // balise specifique VLE


