#include <vle/devs/Executive.hpp>
#include <vle/devs/ExecutiveDbg.hpp>
#include "GraphTranslator.hpp"
#include <vle/utils/DateTime.hpp>
#include <vle/devs/Executive.hpp>
#include <vle/vpz/Vpz.hpp>
#include <boost/numeric/conversion/cast.hpp>



// #####################################################################
/* M�thode pour utilis�e quand le graphe du mod�le est dynamique.
 * R�le tampon pour les sorties de mod�les existants destin�es � un mod�le
 * en cours de cr�ation.
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
            
	    // Cr�ation de l'�v�nement d'ajout de ports sur le mod�le de somme
	    std::string modelName((boost::format("Unit_%1%_AreaActive") % (mNumModel - 1)).str());
	    output.push_back(buildEventWithAString("addModelSumAreaActive", "name", modelName));

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
	    
	    // Creation des unites fonctionnelles           
	    createModelFromClass("Unit", current); 
	    
	    // Creation des connections mod�les DE -> mod�les EXEC 
	    addConnection("Initiation", "InitQuantity", current, "perturb");
	    addConnection("CropClimate", "ActionTemp", current, "ActionTemp");
	    addConnection("CropPhenology", "TempEff", current, "TempEff");
	    addConnection("CropPhenology", "ThermalTime", current, "ThermalTime");
    
	    // Creation des connexions avec Buffer -> mod�les EXEC | EXEC -> Buffer
	    addConnection("Buffer", "ThermalTime", current, "ThermalTime");
	    addConnection("Buffer", "ActionTemp", current, "ActionTemp");
	    addConnection("Buffer", "TempEff", current, "TempEff");
	    addConnection("Buffer", "out", current, "in");
	    addConnection(current, "out", "Buffer", "in");
		    
	    // Creation des ports entrants sur le mod�le de somme (passage unit� -> couvert)
	    addInputPort("SumAreaActive", (vle::fmt("%1%_AreaActive") % current).str());            
	    
	    // Creation des connexions sortantes
	    addConnection(current, "AreaActive", "SumAreaActive", (vle::fmt("%1%_AreaActive") % current).str());
	    
	    // Suppression des connexions entre mod�les EXE n-1 et Buffer
	    if (mNumModel == 1) {
		removeConnection("Unit_0", "update", "Buffer", "in");
	    } else {
		removeConnection(previous, "out", "Buffer", "in");
	    }
		    
	    // Creation des connexions entre mod�les EXE
	    if (mNumModel != 1) {
		addConnection(previous, "out", current, "in");
	    } else {
		addConnection("Unit_0", "update", current, "in");
	    }
	    
	    mNumModel++;
	}

	void etape3() {
	    std::string previous((vle::fmt("Unit_%1%") % (mNumModel - 1)).str());

	    // Suppression des connexions Buffer -> mod�le EXE pr�c�dent
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
};

DECLARE_EXECUTIVE_DBG(ExecutiveFSA)
