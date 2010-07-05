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
#include <vle/utils.hpp>
#include <vle/devs.hpp>

namespace vd = vle::devs;
namespace ve = vle::extension;
namespace vv = vle::value;

class PiloteFSA : public ve::fsa::Statechart
{

public:
    PiloteFSA(const vd::DynamicsInit& init,
           const vd::InitEventList& events) :
            ve::fsa::Statechart(init, events)
    {
	mIndex = 0;
	ThermalTime0 = 0;
	ThermalTime = 0;
	P_UnitTTExp = 0;
	P_UnitTTSen = 0;
	P_TTFlo = 0;
        
        P_UnitTTExp = vv::toDouble(events.get("P_UnitTTExp"));
		P_UnitTTSen = vv::toDouble(events.get("P_UnitTTSen"));
		P_TTFlo = vv::toDouble(events.get("P_TTFlo"));
        

        /* Définition de la liste des conditions gérant l'initiation des unités (GVLE, condition type "Set")
        const vv::Set& p = toSetValue(events.get("P_UnitTTInit"));

        for (unsigned int index = 0; index < p.size(); ++index) {
            P_UnitTTInit.push_back(vv::toDouble(p.get(index)));
        }*/
        
        // Tuple pour une liste de dates d'initiation (P_UnitTTInit)
		const vle::value::TupleValue& p = (vle::value::toTuple(events.get("P_UnitTTInit")));
		vle::value::TupleValue::const_iterator it;
		
		for (it = p.begin(); it != p.end(); ++it) {
			P_UnitTTInit.push_back(*it);
		}
        
        // Construction dynamique du graphe des unités : autant d'états que la longueur du paramétrage. 
        for (unsigned int index = 0; index < P_UnitTTInit.size(); ++index) {
            states(this) << index;
        }
        
        states(this) << P_UnitTTInit.size();        
        
        
        // Transition et actions entre états (unités) 
        for (unsigned int index = 0; index < P_UnitTTInit.size()-1; ++index) {
            transition(this, index, index+1) << guard(&PiloteFSA::development) 
                                             << send(&PiloteFSA::add);
        }
        
        // Transition at action pour l'état final (.size commence à 0, on parle bien du dernier état)
        transition(this, P_UnitTTInit.size()-1, P_UnitTTInit.size()-1) << guard(&PiloteFSA::vegetative)
                                                                       << send(&PiloteFSA::add);
                                                               
        // Transition pour l'arret du développement
        transition(this, P_UnitTTInit.size()-1, P_UnitTTInit.size()) << guard(&PiloteFSA::reproductive);
                                                               
        // Actions quand on rentre dans les états
        for (unsigned int index = 0; index < p.size(); ++index) {
            inAction(this, &PiloteFSA::a) >> index;
            eventInState(this, "ThermalTime", &PiloteFSA::in) >> index;
        }
        
        initialState(0);
        mIndex=0;
    }

    virtual ~PiloteFSA()
    { }

    // Stocke le temps thermique lors de l'entrée dans un état, incrémente le compteur de modèle 
    void a(const vd::Time& /* time */)
    {
        ThermalTime0 = ThermalTime;
        mIndex++;
    }

    // Condition pour permettre la transition entre état : somme de T° > paramètre
    bool development(const vd::Time& /* time */)
    { 
        if (mIndex <= P_UnitTTInit.size()) {
        return ThermalTime-ThermalTime0 > P_UnitTTInit[mIndex-1];
        } else {
        return ThermalTime-ThermalTime0 > P_UnitTTInit.back();            
        }
    }
    
    // Condition pour stopper le developpement : somme de T° > floraison 
    bool flowering(const vd::Time& /* time */)
    { 
        //if (mIndex > P_UnitTTInit.size()) {
        return ThermalTime > P_TTFlo;
        //} 
        //return false;
    }
 
    // condition de transition avant floraison
    bool vegetative(const vd::Time& time)
    { 
        if (development(time) and not flowering(time)) {
        return true;
        }
        return false;
    }
    
    // condition de transition après floraison
    bool reproductive(const vd::Time& time)
    { 
        if (development(time) and flowering(time)) {
        return true;
        }
        return false;
    }

    // Création de modèle atomique
    void add(const vd::Time& /* time */,
             vd::ExternalEventList& output) const
    { 
      vd::ExternalEvent* ee=new vd::ExternalEvent("add");
      ee << vd::attribute("index", new vv::Integer(mIndex));
      ee << vd::attribute("P_UnitTTExp", new vv::Double(ThermalTime + P_UnitTTExp));
      ee << vd::attribute("P_UnitTTSen", new vv::Double(ThermalTime + P_UnitTTSen));
      output.addEvent(ee);
    }
   
    // Accès à la variable ThermalTime
     void in(const vd::Time& /* time */, const vd::ExternalEvent* event )
    { ThermalTime << ve::DifferenceEquation::Var("ThermalTime", event); }

private:
    unsigned int mIndex;
    std::vector < unsigned int > P_UnitTTInit;
    double ThermalTime0;
    double ThermalTime;
    double P_UnitTTExp;
    double P_UnitTTSen;
    double P_TTFlo;
};






class PiloteGraph : public vle::devs::Dynamics
{
public:
    PiloteGraph(const vle::devs::DynamicsInit& init,
              const vle::devs::InitEventList& events) :
	vle::devs::Dynamics(init, events)
    {
        prefix = vv::toString(events.get("prefix"));
        port = vv::toString(events.get("port"));
        classes = vv::toString(events.get("E_GridClasses"));
        matrix = vv::toString(events.get("E_GridMatrix"));
        number = vv::toInteger(events.get("E_GridNumber"));
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
        mp->addString("adjacency matrix", matrix);
        mp->addString("classes", classes);
        		
        		
        ee << vd::attribute("parameter", mp);
   		output.addEvent(ee);
	}
		
	    vle::devs::Time timeAdvance() const
    {
		return vle::devs::Time::infinity;
    }
	
    
private:

std::string prefix;
std::string classes;
std::string port;
std::string matrix;
int number;
};

DECLARE_NAMED_DYNAMICS_DBG(PiloteFSA, PiloteFSA)
DECLARE_NAMED_DYNAMICS_DBG(PiloteGraph, PiloteGraph)

