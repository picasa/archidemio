/**
  * @file UnitConstruction.cpp
  * @author P. Casadebaig-(The RECORD team -INRA )
  */

/*
 * Copyright (C) 2010 INRA
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
#include <vle/devs/ExecutiveDbg.hpp>
#include <vle/translator/GraphTranslator.hpp>
#include <vle/utils.hpp>
#include <vle/devs.hpp>
#include <vle/vpz.hpp>
#include <boost/numeric/conversion/cast.hpp>

class Executive : public vle::devs::Executive
{
public:
    Executive(const vle::devs::ExecutiveInit& init,
              const vle::devs::InitEventList& events) :
	vle::devs::Executive(init, events)
    { }

    virtual ~Executive() { }

    void addConnectionsWithBuffer(unsigned int index,
                                  const std::string& modelName,
                                  const std::string& previousModelName)
    {
        if (index != 1) {
            removeConnection("Buffer", "ThermalTime", previousModelName, "ThermalTime");
            removeConnection("Buffer", "ActionTemp", previousModelName, "ActionTemp");
            removeConnection("Buffer", "TempEff", previousModelName, "TempEff");
            removeConnection("Buffer", "OutDeposition", previousModelName, "InDeposition");
        }
        if (index == 1) {
            removeConnection(previousModelName, "update", "Buffer", "InDeposition");
        } else {
            removeConnection(previousModelName, "OutDeposition", "Buffer", "InDeposition");
        }
        addConnection(modelName, "OutDeposition", "Buffer", "InDeposition");

        addConnection("Buffer", "ThermalTime", modelName, "ThermalTime");
        addConnection("Buffer", "ActionTemp", modelName, "ActionTemp");
        addConnection("Buffer", "TempEff", modelName, "TempEff");
        addConnection("Buffer", "OutDeposition", modelName, "InDeposition");
    }
    
    void addModel()
    {
        for (List::const_iterator it = mAddList.begin();
                 it != mAddList.end(); ++it) {
            std::string previous((vle::fmt("Unit_%1%") % ((*it) - 1)).str());
            std::string current((vle::fmt("Unit_%1%") % (*it)).str());

            // Edition de la valeur d'un paramètre variable entre unité
            vle::vpz::Condition& cnd = conditions().get("condParametres");
            cnd.clearValueOfPort("P_UnitTTExp");
            cnd.addValueToPort("P_UnitTTExp", new vle::value::Double(P_UnitTTExp));
            cnd.clearValueOfPort("P_UnitTTSen");
            cnd.addValueToPort("P_UnitTTSen", new vle::value::Double(P_UnitTTSen));
            
            // Creation des unites fonctionnelles           
            createModelFromClass("Unit", current); 
            
            // Creation des connections entrantes
            addConnection("Initiation", "AreaLatent", current, "perturb");
            addConnection("CropClimate", "ActionTemp", current, "ActionTemp");
            addConnection("CropPhenology", "TempEff", current, "TempEff");
            addConnection("CropPhenology", "ThermalTime", current, "ThermalTime");
            
            // Creation des ports entrants sur le modèle de somme (passage unité -> couvert)
            addInputPort("CropScaling", (vle::fmt("%1%_AreaActive") % current).str());            
            
            // Creation des connections sortantes
            addConnection(current, "AreaActive", "CropScaling", (vle::fmt("%1%_AreaActive") % current).str());
                        
            // Création des connections entre unités : flux de spores
    	    if (*it != 1) {
    	        addConnection(previous, "OutDeposition", current, "InDeposition");
    	    } else {
    	        addConnection("Unit_0", "update", current, "InDeposition");
    	    }
            
            // Creation des connections avec le buffer
            addConnectionsWithBuffer(*it, current, previous);
        }
    }
    
    vle::devs::Time init(const vle::devs::Time& /* time */)
    {
        mPhase = IDLE;
        mOk = false;
        return vle::devs::Time::infinity;
    }

    void output(const vle::devs::Time& /* time */,
                vle::devs::ExternalEventList& output) const
    {
        if (mPhase == SEND) {
            for (List::const_iterator it = mAddList.begin();
                 it != mAddList.end(); ++it) {
                std::string modelName((boost::format("Unit_%1%_AreaActive") %
                                       (*it)).str());
                output.addEvent(buildEventWithAString("add", "name",
                                                      modelName));
            }
            output.addEvent(buildEvent("send"));
        }
    }

    vle::devs::Time timeAdvance() const
    {
        if (mPhase == IDLE) {
            return vle::devs::Time::infinity;
        } else if (mPhase == SEND) {
            return 0;
        }
        return vle::devs::Time::infinity;
    }

    void internalTransition(const vle::devs::Time& /* time */)
    {
        if (mPhase == SEND) {
            mAddList.clear();
            mPhase = IDLE;
        }
    }

    // Recoit les demandes du pilote
    void externalTransition(const vle::devs::ExternalEventList& events,
                            const vle::devs::Time& /* time */)
    {
        vle::devs::ExternalEventList::const_iterator it = events.begin();

        while (it != events.end()) {
            if ((*it)->onPort("add")) {
                unsigned int index = (*it)->getIntegerAttributeValue("index");
                P_UnitTTExp = (*it)->getDoubleAttributeValue("P_UnitTTExp");
                P_UnitTTSen = (*it)->getDoubleAttributeValue("P_UnitTTSen");

                mAddList.push_back(index);
                if (mOk) {
                    addModel();
                    mPhase = SEND;
                    mOk = false;
                } else {
                    mPhase = WAITING;
                }
            } else if ((*it)->onPort("ok")) {
                mOk = true;
                if (mPhase == WAITING) {
                    addModel();
                    mPhase = SEND;
                    mOk = false;
                }
            } else if ((*it)->onPort("cancel")) {
                mOk = false;
            }
            ++it;
        }
    }

private:
    typedef std::vector < unsigned int > List;
    enum phase { IDLE, WAITING, SEND };

    phase mPhase;
    List mAddList;
    bool mOk;
    double P_UnitTTExp;
    double P_UnitTTSen;
};



class ExecutiveGraph : public vle::devs::Executive
{
public:
    ExecutiveGraph(const vle::devs::ExecutiveInit& mdl,
                   const vle::devs::InitEventList& events)
        : vle::devs::Executive(mdl, events)
    {
    }

    virtual ~ExecutiveGraph()
    {}

    virtual vle::devs::Time init(const vle::devs::Time& /* time */)
    {
        vle::translator::GraphTranslator tr(*this);

        vle::value::Map mp;
        mp.addString("prefix", "node");
        mp.addInt("number", 6);
        mp.addString("port", "in-out");
        
        // 6 noeuds, 1 avec 2 voisins
        mp.addString("adjacency matrix",
                     " 0 1 0 0 0 0"
                     " 0 0 1 0 0 0"
                     " 0 0 0 1 0 0"
                     " 0 0 0 0 1 0"
                     " 0 0 0 0 0 1"
                     " 0 0 0 0 0 0");
        mp.addString("classes", "Unit Unit Unit Unit Unit Unit");
	
        tr.translate(mp); 
	
        // Connexions  
        // Initiation sur certains noeuds
        addConnection("Initiation", "AreaLatent", "node-0", "perturb");
        addConnection("Initiation", "AreaLatent", "node-5", "perturb");
        
        // Connexions autres modèles -> noeuds
        // TODO : boucle
        addConnection("CropClimate", "ActionTemp", "node-0", "ActionTemp");
        addConnection("CropPhenology", "TempEff", "node-0", "TempEff");
        addConnection("CropPhenology", "ThermalTime", "node-0", "ThermalTime");
        
        addConnection("CropClimate", "ActionTemp", "node-1", "ActionTemp");
        addConnection("CropPhenology", "TempEff", "node-1", "TempEff");
        addConnection("CropPhenology", "ThermalTime", "node-1", "ThermalTime");
        
        addConnection("CropClimate", "ActionTemp", "node-2", "ActionTemp");
        addConnection("CropPhenology", "TempEff", "node-2", "TempEff");
        addConnection("CropPhenology", "ThermalTime", "node-2", "ThermalTime");
        
        addConnection("CropClimate", "ActionTemp", "node-3", "ActionTemp");
        addConnection("CropPhenology", "TempEff", "node-3", "TempEff");
        addConnection("CropPhenology", "ThermalTime", "node-3", "ThermalTime");
        
        addConnection("CropClimate", "ActionTemp", "node-4", "ActionTemp");
        addConnection("CropPhenology", "TempEff", "node-4", "TempEff");
        addConnection("CropPhenology", "ThermalTime", "node-4", "ThermalTime");
        
        addConnection("CropClimate", "ActionTemp", "node-5", "ActionTemp");
        addConnection("CropPhenology", "TempEff", "node-5", "TempEff");
        addConnection("CropPhenology", "ThermalTime", "node-5", "ThermalTime");
        
        // Visualiser le VPZ qui correspond à celui crée par Executive
        std::ofstream file("output.vpz");
        dump(file);

        return vle::devs::Time::infinity;
    }
};

DECLARE_NAMED_EXECUTIVE_DBG(UnitConstruction, Executive)
DECLARE_NAMED_EXECUTIVE_DBG(UnitConstructionGraph, ExecutiveGraph)  
