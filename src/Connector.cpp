#include <vle/devs/Executive.hpp>
#include <vle/devs/ExecutiveDbg.hpp>
#include <vle/vpz/CoupledModel.hpp>

class Connector : public vle::devs::Executive
{
public:
    Connector(const vle::devs::ExecutiveInit& init,
              const vle::devs::InitEventList& events) :
	vle::devs::Executive(init, events)
    { }

    virtual ~Connector() { }

    virtual vle::devs::Time init(const vle::devs::Time&  /*time*/)
    {
		mSended = false;
		return 0;
	}
	
	virtual void internalTransition(const vle::devs::Time& /*time*/)
	{
		if (mSended == false) {
			// Ajout des connexions mod�le coupl� -> mod�le sum
			const vle::vpz::ConnectionList& inputs = coupledmodel().getInputPortList();
			vle::vpz::ConnectionList::const_iterator it;
			
			/* DEBUG : affiche les connections
			for (it = inputs.begin(); it != inputs.end(); ++it) {
				std::cout << "model: " << getModel().getParentName() << it->first << std::endl;
			}*/
			
			for (it = inputs.begin(); it != inputs.end(); ++it) {
				if (it->first.size() >= 5 and it->first.compare(0, 5, "node-") == 0) {
					addInputPort("Deposition", it->first);
					addConnection(coupledmodelName(), it->first, "Deposition", it->first);
					mPortList.push_back(it->first);
				}
			}

			// Pour les mod�les sans d�pendances
			if (mPortList.empty()){
				createModelFromClass("node-init", "node-init");
				//delModel("Deposition");
				removeConnection("Deposition", "update", "Unit", "in");	
				addConnection("node-init", "update", "Unit", "in");
			}
			mSended = true;
		} else {
			mSended = false;
		}
    }
    
    virtual vle::devs::Time timeAdvance() const
    {
		if (mSended == true) {
			return 0;
		} else {
			return vle::devs::infinity;
		}
	}
    
    virtual void output(const vle::devs::Time& /* time */,
                        vle::devs::ExternalEventList& output) const
    {
		std::vector < std::string >::const_iterator it;
		
		for (it = mPortList.begin(); it != mPortList.end(); ++it) {		
			output.push_back(buildEventWithAString("add", "name", *it));
		}	
	}

	std::vector < std::string > mPortList;
	bool mSended;
};

DECLARE_EXECUTIVE_DBG(Connector)
