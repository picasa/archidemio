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

class PiloteFSA : public ve::fsa::Statechart
{

public:
    PiloteFSA(const vd::DynamicsInit& init,
           const vd::InitEventList& events) :
            ve::fsa::Statechart(init, events)
    {
        mIndex = 0;
        //ThermalTime0 = 0;
        ThermalTime = 0;
        PreformedTT = 0;
        //P_ReproductiveTT = 0;        
		P_ReproductiveTT = events.getDouble("P_ReproductiveTT");
        

        /* Définition de la liste des conditions gérant l'initiation des unités (GVLE, condition type "Set")
        const vv::Set& p = toSetValue(events.get("P_InitiationTT"));

        for (unsigned int index = 0; index < p.size(); ++index) {
            P_InitiationTT.push_back(vv::toDouble(p.get(index)));
        }*/
        
        // Tuple pour une liste de dates d'initiation (P_InitiationTT)
		const vle::value::TupleValue& p = (vle::value::toTuple(events.get("P_InitiationTT")));
		vle::value::TupleValue::const_iterator it;
		
		for (it = p.begin(); it != p.end(); ++it) {
			P_InitiationTT.push_back(*it);
		}
        
        // Construction dynamique du graphe des unités : autant d'états que la longueur du paramétrage. 
        for (unsigned int index = 0; index < P_InitiationTT.size(); ++index) {
            states(this) << index;
        }
        
        states(this) << P_InitiationTT.size();        
        
        
        // Transition et actions entre états (unités) 
        for (unsigned int index = 0; index < P_InitiationTT.size()-1; ++index) {
            transition(this, index, index+1) << guard(&PiloteFSA::development) 
                                             << send(&PiloteFSA::add);
        }
        
        // Transition at action pour l'état final (.size commence à 0, on parle bien du dernier état)
        transition(this, P_InitiationTT.size()-1, P_InitiationTT.size()-1) << guard(&PiloteFSA::vegetative)
                                                                       << send(&PiloteFSA::add);
                                                               
        // Transition pour l'arret du développement
        transition(this, P_InitiationTT.size()-1, P_InitiationTT.size()) << guard(&PiloteFSA::reproductive);
                                                               
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

    // incrémente l'index du modèle 
    void a(const vd::Time& /* time */)
    {
        // Stocke le temps thermique lors de l'entrée dans un état
        //ThermalTime0 = ThermalTime;
        mIndex++;
    }

    // Condition pour permettre la transition entre état 
    // soit : somme de T° > paramètre
    // soit : somme de T° > somme théorique 
    bool development(const vd::Time& /* time */)
    { 
        if (P_InitiationTT.size() >= 2) {
            if (mIndex <= P_InitiationTT.size()-1) {
                // On compte le tps passé dans un état et le compare à un paramètre
                //return ThermalTime-ThermalTime0 >= P_InitiationTT[mIndex-1];
                
                // On calcule une date thermique théorique en testant directement le TT   
                return ThermalTime > mIndex * P_InitiationTT[mIndex-1]; //  numérotation depuis 0 dans un vecteur        
            
            } else {
                //return ThermalTime-ThermalTime0 >= P_InitiationTT.back();
                PreformedTT = std::accumulate(P_InitiationTT.begin(),P_InitiationTT.end()-1,0);
                return ThermalTime >  PreformedTT + (mIndex-3)*P_InitiationTT.back();
            }
        } else {
            return ThermalTime > mIndex * P_InitiationTT[0];
        }
    }
    
    // Condition pour stopper le developpement : somme de T° > floraison 
    bool flowering(const vd::Time& /* time */)
    { 
        return ThermalTime > P_ReproductiveTT;
        //} 
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

    // Création de l'événement pour creer un modèle atomique
    // Modification des paramètres de phénologie
    void add(const vd::Time& /* time */,
             vd::ExternalEventList& output) const
    { 
      vd::ExternalEvent* ee=new vd::ExternalEvent("add");
      ee << vd::attribute("index", new vv::Integer(mIndex));
      output.push_back(ee);
    }
   
    // Accès à la variable ThermalTime
     void in(const vd::Time& /* time */, const vd::ExternalEvent* event )
    { ThermalTime << ve::DifferenceEquation::Var("ThermalTime", event); }

private:
    unsigned int mIndex;
    std::vector < unsigned int > P_InitiationTT;
    int PreformedTT;
    //double ThermalTime0;
    double ThermalTime;
    double P_ReproductiveTT;
};


DECLARE_DYNAMICS(PiloteFSA)

