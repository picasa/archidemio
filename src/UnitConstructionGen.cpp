#include <vle/devs/Executive.hpp>
#include <vle/devs/ExecutiveDbg.hpp>
#include <vle/translator/GraphTranslator.hpp>
#include <vle/utils.hpp>
#include <vle/devs.hpp>
#include <vle/vpz.hpp>
#include <boost/numeric/conversion/cast.hpp>



// #####################################################################
/* Méthode pour utilisée quand le graphe du modèle est dynamique.
 * Rôle tampon pour les sorties de modèles existants destinées à un modèle
 * en cours de création.
 * 
 */ 

class ExecutiveFSA : public vle::devs::Executive
{
public:
    ExecutiveFSA(const vle::devs::ExecutiveInit& init,
              const vle::devs::InitEventList& events) :
	vle::devs::Executive(init, events)
    { }

    virtual ~ExecutiveFSA() { }

    virtual vle::devs::Time init(const vle::devs::Time&  time)
    {
        if (getModel().getOutputPortNumber() !=
            getModel().getInputPortNumber()) {
            throw vle::utils::InternalError(
                vle::fmt(_("[%1%] Buffer: wrong output port number %2% %3%")) %
                getModelName() % getModel().getOutputPortNumber() % getModel().getInputPortNumber());
        }

        mLastTime = time;
        mPhase = IDLE;
        mWaiting = getModel().getInputPortNumber() - 1;
        mAddModel = false;
        mNumModel = 1;
        P_ExpansionTT=0;
        P_SenescenceTT=0;
        return vle::devs::Time::infinity;
    }

    virtual vle::devs::Time timeAdvance() const
    {
        if (mPhase == IDLE) {
            return vle::devs::Time::infinity;
        } else {
            return 0;
        }
    }

    virtual void output(const vle::devs::Time& /* time */,
                        vle::devs::ExternalEventList& output) const
    {
        if (mPhase == ADDED) {
            std::map < std::string, double >::const_iterator it;
            
			std::string modelName((boost::format("Unit_%1%_AreaActive") % (mNumModel - 1)).str());
			output.addEvent(buildEventWithAString("addModelSumAreaActive", "name", modelName));
            
			for (it = mBuffer.begin(); it != mBuffer.end(); it++) {
                vle::devs::ExternalEvent* ee = buildEvent(it->first);
                
                ee << vle::devs::attribute("name", it->first);
                ee << vle::devs::attribute("value", it->second);
                output.addEvent(ee);
            }
        } 
    }

    virtual void internalTransition(const vle::devs::Time& /* time */)
    {
		switch (mPhase) {
			case IDLE: 
	            throw vle::utils::InternalError(
	                vle::fmt(_("[%1%] Buffer: Phase IDLE")) %
	                getModelName());
			case ADDED: mPhase=SENDED; break;
			case SENDED: mPhase=RESTORE; etape3(); break;
			case RESTORE: mPhase=IDLE; mAddModel= false; break;
			}
    }

    virtual void externalTransition(const vle::devs::ExternalEventList& event,
                                    const vle::devs::Time& time)
    {
        typedef vle::devs::ExternalEventList::const_iterator const_iterator;
        typedef std::map < std::string, double >::iterator iterator;
        
        // Nettoyage de Buffer
		updateTime(time);

		// Construction du Buffer si les evts contiennent des valeurs
        for (const_iterator it = event.begin(); it != event.end(); ++it) {
			if ((*it)->existAttributeValue("value")) {
				std::string name = (*it)->getStringAttributeValue("name");
				double value = (*it)->getDoubleAttributeValue("value");
				
				std::pair < iterator, bool > r;
		   
				r = mBuffer.insert(std::make_pair < std::string, double >(name, value));
				if(r.second == true) {
					mWaiting --;
					//TraceModel("mBuffer add");
				} else {
					//TraceModel("mBuffer not add");
				}
				assert(mWaiting >= 0);
			}
		}

		// Separer le remplissage du buffer du fait de recevoir un evenement "add"
        for (const_iterator it = event.begin(); it != event.end(); ++it) {
            if ((*it)->onPort("add")) {
				mAddModel = true;
                P_ExpansionTT = (*it)->getDoubleAttributeValue("P_ExpansionTT");
                P_SenescenceTT = (*it)->getDoubleAttributeValue("P_SenescenceTT");
				//TraceModel("mAddModel: add");
            }
        }
        
        //TraceModel(mWaiting);
        
		if (mAddModel == true and mWaiting == 0) {
			mPhase = ADDED;
			etape1();
		} else {
			mPhase = IDLE;
		}

	}

	void updateTime(const vle::devs::Time& time) 
	{
		if (time > mLastTime) {
			mBuffer.clear();
			mAddModel = false;
			mWaiting = getModel().getInputPortNumber() - 1;
		} 
		mLastTime = time;
	}
	
	void etape1() {
		std::string previous((vle::fmt("Unit_%1%") % (mNumModel - 1)).str());
		std::string current((vle::fmt("Unit_%1%") % mNumModel).str());

		// Edition de la valeur d'un paramètre variable entre unité
		vle::vpz::Condition& cnd = conditions().get("condParametres");
		cnd.clearValueOfPort("P_ExpansionTT");
		cnd.addValueToPort("P_ExpansionTT", new vle::value::Double(P_ExpansionTT));
		cnd.clearValueOfPort("P_SenescenceTT");
		cnd.addValueToPort("P_SenescenceTT", new vle::value::Double(P_SenescenceTT));
		
		// Creation des unites fonctionnelles           
		createModelFromClass("Unit", current); 
		
		// Creation des connections modèles DE -> modèles EXE 
		addConnection("Initiation", "InitQuantity", current, "perturb");
		addConnection("CropClimate", "ActionTemp", current, "ActionTemp");
		addConnection("CropPhenology", "TempEff", current, "TempEff");
		addConnection("CropPhenology", "ThermalTime", current, "ThermalTime");
        
		// Creation des connexions avec Buffer -> modèles EXE | EXE -> Buffer
		addConnection("Buffer", "ThermalTime", current, "ThermalTime");
		addConnection("Buffer", "ActionTemp", current, "ActionTemp");
		addConnection("Buffer", "TempEff", current, "TempEff");
		addConnection("Buffer", "out", current, "in");
		addConnection(current, "out", "Buffer", "in");
        		
		// Creation des ports entrants sur le modèle de somme (passage unité -> couvert)
		addInputPort("SumAreaActive", (vle::fmt("%1%_AreaActive") % current).str());            
		
		// Creation des connexions sortantes
		addConnection(current, "AreaActive", "SumAreaActive", (vle::fmt("%1%_AreaActive") % current).str());
		
		// Suppression des connexions entre modèles EXE n-1 et Buffer
		if (mNumModel == 1) {
			removeConnection("Unit_0", "update", "Buffer", "in");
		} else {
			removeConnection(previous, "out", "Buffer", "in");
		}
			
		// Creation des connexions entre modèles EXE
		if (mNumModel != 1) {
			addConnection(previous, "out", current, "in");
		} else {
			addConnection("Unit_0", "update", current, "in");
		}
		
		mNumModel++;
	}

	void etape3() {
		std::string previous((vle::fmt("Unit_%1%") % (mNumModel - 1)).str());

		// Suppression des connexions Buffer -> modèle EXE précédent
		removeConnection("Buffer", "ThermalTime", previous, "ThermalTime");
		removeConnection("Buffer", "ActionTemp", previous, "ActionTemp");
		removeConnection("Buffer", "TempEff", previous, "TempEff");
		removeConnection("Buffer", "out", previous, "in");
		
	}

private:
    enum Phase {IDLE, ADDED, SENDED, RESTORE};
    std::map < std::string, double > mBuffer;
    Phase mPhase;
    vle::devs::Time mLastTime;
    int mWaiting;
    bool mAddModel;
    int mNumModel;
    double P_ExpansionTT;
    double P_SenescenceTT;
};









// #####################################################################
/* Méthode pour utilisée quand le graphe du modèle n'est pas dynamique :
 * tout les modèles sont créés au debut de la simulation.
 * 
 * 
 */ 

class ExecutiveGraph : public vle::devs::Executive
{
public:
    ExecutiveGraph(const vle::devs::ExecutiveInit& init,
              const vle::devs::InitEventList& events) :
		vle::devs::Executive(init, events)
	{
		const vle::value::TupleValue& tuple(vle::value::toTuple(events.get("E_InitSpace")));
		vle::value::TupleValue::const_iterator it;
		
		for (it = tuple.begin(); it != tuple.end(); ++it) {
			E_InitSpace.push_back(*it);
		}
	}
	
    virtual ~ExecutiveGraph() { }

    virtual vle::devs::Time init(const vle::devs::Time&  time)
    {
        if (getModel().getOutputPortNumber() !=
            getModel().getInputPortNumber()) {
            throw vle::utils::InternalError(
                vle::fmt(_("[%1%] Buffer: wrong output port number %2% %3%")) %
                getModelName() % getModel().getOutputPortNumber() % getModel().getInputPortNumber());
        }

        mLastTime = time;
        mPhase = IDLE;
        mWaiting = getModel().getInputPortNumber() - 1;
        mAddModel = false;
        mParameter = 0;
        
        return vle::devs::Time::infinity;
    }

    virtual vle::devs::Time timeAdvance() const
    {
        if (mPhase == IDLE) {
            return vle::devs::Time::infinity;
        } else {
            return 0;
        }
    }

    virtual void output(const vle::devs::Time& /* time */,
                        vle::devs::ExternalEventList& output) const
    {
        if (mPhase == ADDED) {
            std::map < std::string, double >::const_iterator it;
            
			for (int i = 0; i!=mNumber; i++) {
				std::string modelName((boost::format("%1%-%2%_AreaActive") % mPrefix % i).str());
				output.addEvent(buildEventWithAString("addModelSumAreaActive", "name", modelName));				
			}		
            
            
			for (it = mBuffer.begin(); it != mBuffer.end(); it++) {
                vle::devs::ExternalEvent* ee = buildEvent(it->first);
                
                ee << vle::devs::attribute("name", it->first);
                ee << vle::devs::attribute("value", it->second);
                output.addEvent(ee);
            }
        } 
    }

    virtual void internalTransition(const vle::devs::Time& /* time */)
    {
		switch (mPhase) {
			case IDLE: 
	            throw vle::utils::InternalError(
	                vle::fmt(_("[%1%] Buffer: Phase IDLE")) %
	                getModelName());
			case ADDED: mPhase=SENDED; break;
			case SENDED: mPhase=RESTORE; etape3(); break;
			case RESTORE: mPhase=IDLE; mAddModel= false; break;
			}
    }

    virtual void externalTransition(const vle::devs::ExternalEventList& event,
                                    const vle::devs::Time& time)
    {
        typedef vle::devs::ExternalEventList::const_iterator const_iterator;
        typedef std::map < std::string, double >::iterator iterator;
        
	// Nettoyage Buffer
	updateTime(time);

	// Construction du Buffer si les evts contiennent des valeurs
        for (const_iterator it = event.begin(); it != event.end(); ++it) {
	    if ((*it)->existAttributeValue("value")) {
		std::string name = (*it)->getStringAttributeValue("name");
		double value = (*it)->getDoubleAttributeValue("value");
		
		std::pair < iterator, bool > r;
   
		r = mBuffer.insert(std::make_pair < std::string, double >(name, value));
		if(r.second == true) {
		    mWaiting --;
		    //TraceModel("mBuffer add");
		} else {
		    //TraceModel("mBuffer not add");
		}
		assert(mWaiting >= 0);
	    }
	}
	
		// Separer le remplissage du buffer du fait de recevoir un evenement "add"
        for (const_iterator it = event.begin(); it != event.end(); ++it) {
            if ((*it)->onPort("add")) {
				mParameter = vle::value::toMapValue((*it)->getMapAttributeValue("parameter").clone());
				mAddModel = true;
            }
        }
        
        TraceModel(mWaiting);
        
		if (mAddModel == true and mWaiting == 0) {
			mPhase = ADDED;
			etape1();
		} else {
			mPhase = IDLE;
		}
	}

	void updateTime(const vle::devs::Time& time) 
	{
		if (time > mLastTime) {
			mBuffer.clear();
			mAddModel = false;
			mWaiting = getModel().getInputPortNumber() - 1;
		} 
		mLastTime = time;
	}
	
	void etape1()
	{		
	// Creation des modèles           
        vle::translator::GraphTranslator tr (*this);
        tr.translate(*mParameter);
		
	// Boucle pour creer les connexions 
	mNumber = mParameter->getInt("number");
	mPrefix = mParameter->getString("prefix");
		
        // Initiation sur certains noeuds selon condition E_InitSpace (tuple)       
        std::vector < unsigned int >::const_iterator it;
	    for (it = E_InitSpace.begin(); it != E_InitSpace.end(); ++it) {
		addConnection("Initiation", "InitQuantity", (vle::fmt("%1%-%2%") % mPrefix % (*it)).str(), "perturb");
	    }
		
        // Ajout des connexions modèles DE -> EXE
	for (int i = 0; i!=mNumber; i++) {
	    const std::string current = (vle::fmt("%1%-%2%") % mPrefix % i).str();
	    
	    addConnection("CropClimate", "ActionTemp", current, "ActionTemp");
	    addConnection("CropPhenology", "TempEff", current, "TempEff");
	    addConnection("CropPhenology", "ThermalTime", current, "ThermalTime");
		
	    // Creation des connexions avec Buffer -> modèles EXE 
	    addConnection("Buffer", "ThermalTime", current, "ThermalTime");
	    addConnection("Buffer", "ActionTemp", current, "ActionTemp");
	    addConnection("Buffer", "TempEff", current, "TempEff");
		
	    // Creation des ports entrants sur le modèle de somme (passage unité -> couvert)
	    addInputPort("SumAreaActive", (vle::fmt("%1%_AreaActive") % current).str());            
	    
	    // Creation des connexions sortantes
	    addConnection(current, "AreaActive", "SumAreaActive", (vle::fmt("%1%_AreaActive") % current).str());
	}		

 	}

	void etape3() {
		for (int i = 0; i!=mNumber; i++) {
			const std::string current = (vle::fmt("%1%-%2%") % mPrefix % i).str();
			
			removeConnection("Buffer", "ThermalTime", current, "ThermalTime");
			removeConnection("Buffer", "ActionTemp", current, "ActionTemp");
			removeConnection("Buffer", "TempEff", current, "TempEff");
		}
				
		std::ofstream file("output.vpz");
        dump(file);		

		// TODO delModel("Buffer");		
	}

private:
    enum Phase {IDLE, ADDED, SENDED, RESTORE};
    std::map < std::string, double > mBuffer;
    Phase mPhase;
    vle::devs::Time mLastTime;
    int mWaiting;
    bool mAddModel;
    std::string mPrefix;
    int mNumber;
    vle::value::Map* mParameter;
    std::vector < unsigned int > E_InitSpace;
    
};


DECLARE_NAMED_EXECUTIVE_DBG(ExecutiveFSA, ExecutiveFSA)
DECLARE_NAMED_EXECUTIVE_DBG(ExecutiveGraph, ExecutiveGraph)
