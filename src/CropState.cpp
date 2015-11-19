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

enum State { BareSoil, Sowing, Emergence, Closure, Reproductive, Maturity };

class CropState : public vf::Statechart
{
public:
    CropState(
	const vd::DynamicsInit& init,
	const vd::InitEventList& events) 
	: vf::Statechart(init, events)	
	{
		// Variables d'�tat
	    eventInState(this, "ThermalTime", &CropState::in)
		>> BareSoil >> Sowing >> Emergence >> Closure
			>> Reproductive >> Maturity;
	    
	    states(this) << BareSoil << Sowing << Emergence << Closure
			<< Reproductive << Maturity;
	    
	    transition(this, BareSoil, Sowing) << after(10.0);
				   
	    transition(this, Sowing, Emergence) << after(10.0);
				   
	    transition(this, Emergence, Closure) << after(10.0);
				   
	    transition(this, Closure, Reproductive) << after(10.0)
				   << send(&CropState::reproductive);
				   
	    transition(this, Reproductive, Maturity) << after(10.0);

	    initialState(BareSoil);
	}

    virtual ~CropState() { }

    void reproductive (const vd::Time& /* time */, vd::ExternalEventList& output) const
	{
	    // Remplace la valeur d'une variable d'�tat dans un mod�le "Unit"
	    output << (ve::DifferenceEquation::Var("CropState") = 4.0);
	}
	
    
private:

    // Acc�s variables Diff Equation
     void in(const vd::Time& /* time */, const vd::ExternalEvent* event )
    { ThermalTime << ve::DifferenceEquation::Var("ThermalTime", event); }

	// Varibles d'�tat
    double ThermalTime;

}; // namespace Crop

} 

DECLARE_DYNAMICS(Crop::CropState)

