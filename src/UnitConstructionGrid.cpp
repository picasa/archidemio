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

#include <vle/devs/Executive.hpp>
#include <vle/translator/MatrixTranslator.hpp>
#include <string>
#include <vector>
#include <vle/value/XML.hpp>
 
namespace vd = vle::devs;
namespace vv = vle::value;
namespace vu = vle::utils;
 
namespace  Crop {
 
    class UnitConstruction : public vd::Executive
    {
	public:
		UnitConstruction(
			const vd::ExecutiveInit& model,
			const vd::InitEventList& events)
			 : vd::Executive(model, events)
		{
			m_buffer = events.get("translate").clone();
		}
 
 
		virtual ~UnitConstruction() {}
 
 
		virtual vd::Time init(const vd::Time& /*time*/)
		{
			vle::translator::MatrixTranslator tr(*this);
			tr.translate(*m_buffer);
			delete m_buffer;
 
//			coupledmodel().writeXML(std::cout); 
 
			return vd::Time::infinity;
		}
 
 
    private:
        vv::Value* m_buffer;
    };
 
}

DECLARE_NAMED_EXECUTIVE(UnitConstruction, Crop::UnitConstruction);
