/**
 * @file src/passive.cpp
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


#include <vle/extension/difference-equation/Multiple.hpp>

namespace Crop {

    class passive: public vle::extension::DifferenceEquation::Multiple
    {
	public:
	    passive(const vle::devs::DynamicsInit& init,
			  const vle::devs::InitEventList& events) :
	    vle::extension::DifferenceEquation::Multiple(init, events)
	    {
	    }


	    virtual ~passive() {};


    	virtual void compute(const vle::devs::Time& /*time*/)
	    {
	    }

    	virtual void initValue(const vle::devs::Time& /* time */)
    	{
	}


    private:

    };
}
DECLARE_DYNAMICS(Crop::passive);


