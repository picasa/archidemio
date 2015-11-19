/**
 * @file Pilote.cpp
 * @author The RECORD Development Team (INRA) and the VLE Development Team
 * See the AUTHORS or Authors.txt file
 */

/*
 * Copyright (C) 2010 INRA
 * Copyright (C) 2010 ULCO http://www.univ-littoral.fr
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

#include <vle/extension/fsa/Statechart.hpp>
#include <vle/extension/DifferenceEquation.hpp>
#include <vle/devs/DynamicsDbg.hpp>
#include <vle/utils/DateTime.hpp>
#include <vle/devs/Time.hpp>

namespace vd = vle::devs;
namespace ve = vle::extension;
namespace vv = vle::value;


class PiloteGraph : public vle::devs::Dynamics
{
public:
    PiloteGraph(const vle::devs::DynamicsInit& init,
              const vle::devs::InitEventList& events) :
	vle::devs::Dynamics(init, events),
    matrix(events.getTuple("E_GridMatrix"))
    {
        prefix = events.getString("prefix");
        port = events.getString("port");
        classes = events.getString("E_GridClasses");
        number = events.getInt("E_GridNumber");
    }
    
    virtual ~PiloteGraph() { }
    
    vle::devs::Time init(const vle::devs::Time& /* time */)
    {
        return 0;
    }
    
    void output(const vle::devs::Time& /* time */,
                vle::devs::ExternalEventList& output) const
	{	
        
        
        vd::ExternalEvent* ee=new vd::ExternalEvent("add");

		vle::value::Map* mp = new vle::value::Map ();
        
        mp->addString("prefix", prefix);
        mp->addInt("number", number);
        mp->addString("port", "out");
        mp->add("adjacency matrix", matrix);
        mp->addString("default classname", classes);
        		
        		
        ee << vd::attribute("parameter", mp);
   		output.push_back(ee);
	}
		
	    vle::devs::Time timeAdvance() const
    {
		return vle::devs::infinity;
    }
	
    
private:

std::string prefix;
std::string classes;
std::string port;
vle::value::Tuple matrix;
int number;
};

DECLARE_DYNAMICS(PiloteGraph)

