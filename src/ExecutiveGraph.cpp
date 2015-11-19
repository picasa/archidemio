#include <vle/devs/Executive.hpp>
#include <vle/devs/ExecutiveDbg.hpp>
#include "GraphTranslator.hpp"
#include <vle/utils/DateTime.hpp>
#include <vle/devs/Time.hpp>
#include <vle/vpz/Vpz.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <fstream>


// #####################################################################
/* M�thode pour utilis�e quand le graphe du mod�le n'est pas dynamique :
 * tout les mod�les sont cr��s au debut de la simulation.
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
        
        return vle::devs::infinity;
    }

    virtual vle::devs::Time timeAdvance() const
    {
        if (mPhase == IDLE) {
            return vle::devs::infinity;
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
				output.push_back(buildEventWithAString("addModelSumAreaActive", "name", modelName));
			}		
            
            
			for (it = mBuffer.begin(); it != mBuffer.end(); it++) {
                vle::devs::ExternalEvent* ee = buildEvent(it->first);
                
                ee << vle::devs::attribute("name", it->first);
                ee << vle::devs::attribute("value", it->second);
                output.push_back(ee);
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
        
        TraceModel(vle::fmt("%1%") % mWaiting);
        
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
	// Creation des mod�les           
        DynamicGraphTranslator tr (*mParameter);
	
	Eigen::MatrixXd A = tr.getMatrix();
	
	tr.setMatrix(A);
	
    tr.build(this);
		
	// Boucle pour creer les connexions 
	mNumber = mParameter->getInt("number");
	mPrefix = mParameter->getString("prefix");
		
	// Initiation sur certains noeuds selon condition E_InitSpace (tuple)       
	std::vector < unsigned int >::const_iterator it;
    for (it = E_InitSpace.begin(); it != E_InitSpace.end(); ++it) {
	addConnection("Initiation", "InitQuantity", (vle::fmt("%1%-%2%") % mPrefix % (*it)).str(), "perturb");
    }
		
    // Ajout des connexions mod�les DE -> EXE
	for (int i = 0; i!=mNumber; i++) {
	    const std::string current = (vle::fmt("%1%-%2%") % mPrefix % i).str();
	    
	    addConnection("CropClimate", "ActionTemp", current, "ActionTemp");
	    addConnection("CropPhenology", "TempEff", current, "TempEff");
	    addConnection("CropPhenology", "ThermalTime", current, "ThermalTime");
		
	    // Creation des connexions avec Buffer -> mod�les EXE 
	    addConnection("Buffer", "ThermalTime", current, "ThermalTime");
	    addConnection("Buffer", "ActionTemp", current, "ActionTemp");
	    addConnection("Buffer", "TempEff", current, "TempEff");
		
	    // Creation des ports entrants sur le mod�le de somme (passage unit� -> couvert)
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

DECLARE_EXECUTIVE_DBG(ExecutiveGraph)
