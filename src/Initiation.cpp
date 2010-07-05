/**
 * @file examples/equation/Perturb.cpp
 * @author The VLE Development Team
 * See the AUTHORS or Authors.txt file
 */

/*
 * VLE Environment - the multimodeling and simulation environment
 * This file is a part of the VLE environment
 * http://www.vle-project.org
 *
 * Copyright (C) 2007-2010 INRA http://www.inra.fr
 * Copyright (C) 2003-2010 ULCO http://www.univ-littoral.fr
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
#include <vle/devs/DynamicsDbg.hpp>
#include <vle/extension/DifferenceEquation.hpp>
//#include <vle/extension/difference-equation/Base.hpp>
#include <vle/extension/fsa/Statechart.hpp>

namespace Crop {

namespace ve = vle::extension;
namespace vf = vle::extension::fsa;
namespace vd = vle::devs;
namespace vv = vle::value;

enum State { Healthy, Infected };

class Initiation : public vf::Statechart
{
public:
    Initiation(
	const vd::DynamicsInit& init,
	const vd::InitEventList& events) 
	: vf::Statechart(init, events)
	{
	    E_Init = std::floor(vv::toDouble(events.get("E_InitTime")));
	    
	    states(this) << Healthy << Infected;
	    
	    // passage de l'état Sain à Infecté, après un temps déterminé (E_Init)
	    // action : infection des unités existantes au moment de la transition
	    transition(this, Healthy, Infected) << after(E_Init)
				   << send(&Initiation::infection);

	    initialState(Healthy);
	}

    virtual ~Initiation() { }

    void infection (const vd::Time& /* time */, vd::ExternalEventList& output) const
	{
	    // Remplace la valeur d'une variable d'état dans un modèle "Unit"
	    output << (ve::DifferenceEquation::Var("AreaLatent") = 0.001);
	}
	
	
private:
    double E_Init; /**< Paramètre : date d'arrivé de la maladie dans le système */

}; // namespace Crop

} 

DECLARE_NAMED_DYNAMICS_DBG(Initiation, Crop::Initiation)

