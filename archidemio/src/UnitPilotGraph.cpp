/*
 * @@tagdepends: vle.discrete-time, ext.Eigen @@endtagdepends
 */

#include <vle/utils/DateTime.hpp>
#include <vle/utils/Tools.hpp>
#include <vle/devs/Executive.hpp>
#include <vle/value/Tuple.hpp>

#include <vle/discrete-time/DiscreteTimeExec.hpp>
#include "GraphTranslator.hpp"

#include <fstream>

namespace vd = vle::devs;
namespace vv = vle::value;

namespace vle {
namespace discrete_time {
namespace archidemio {


class UnitPilotGraph : public DiscreteTimeExec
{
public:
    UnitPilotGraph(const vle::devs::ExecutiveInit& init,
            const vd::InitEventList& events): DiscreteTimeExec(init, events)
    {
        //init inputs
        ThermalTime.init(this, "ThermalTime", events);


        const vle::value::TupleValue& tuple =
                events.getTuple("E_InitSpace").value();
        vle::value::TupleValue::const_iterator it;
        int number;
        std::string prefix;
        std::vector < unsigned int > E_InitSpace;


        for (it = tuple.begin(); it != tuple.end(); ++it) {
            E_InitSpace.push_back(*it);
        }

        // models creation
        DynamicGraphTranslator tr (events);
        Eigen::MatrixXd A = tr.getMatrix();
        tr.setMatrix(A);
        tr.build(this);

        //create other connections
        number = events.getInt("number");
        prefix = events.getString("prefix");

        // Initiation sur certains noeuds selon condition E_InitSpace (tuple)
        std::vector < unsigned int >::const_iterator itb;
        for (itb = E_InitSpace.begin(); itb != E_InitSpace.end(); ++itb) {
            addConnection("Initialization", "InitQuantity",
                    vle::utils::format("%s-%d", prefix.c_str(), (*itb)), "InitQuantity");
        }

        // Ajout des connexions modeles DE -> EXE
        for (int i = 0; i!=number; i++) {
            const std::string current =
                    vle::utils::format("%s-%d", prefix.c_str(), i);

            addConnection("CropClimate", "ActionTemp", current, "ActionTemp");
            addConnection("CropPhenology", "TempEff", current, "TempEff");
            addConnection("CropPhenology", "ThermalTime", current, "ThermalTime");

            // Creation des ports entrants sur le modele de somme (passage unite -> couvert)
            addInputPort("GenericSum",
                    vle::utils::format("%s_AreaActive", current.c_str()));

            // Creation des connexions sortantes
            addConnection(current, "AreaActive", "GenericSum",
                    vle::utils::format("%s_AreaActive", current.c_str()));
        }

        std::ofstream file("archidemio/exp/output.vpz");
        dump(file, "dump");
    }

    virtual ~UnitPilotGraph() { }
    void compute(const vle::devs::Time& t){};

//    virtual vle::devs::Time init(const vle::devs::Time&  time)
//    {
//        if (getModel().getOutputPortNumber() !=
//                getModel().getInputPortNumber()) {
//            throw vle::utils::InternalError(
//                    vle::fmt(_("[%1%] Buffer: wrong output port number %2% %3%")) %
//                    getModelName() % getModel().getOutputPortNumber() % getModel().getInputPortNumber());
//        }
//
//        mLastTime = time;
//        mPhase = IDLE;
//        mWaiting = getModel().getInputPortNumber() - 1;
//        mAddModel = false;
//        mParameter = 0;
//
//        return vle::devs::infinity;
//    }
//
//    virtual vle::devs::Time timeAdvance() const
//    {
//        if (mPhase == IDLE) {
//            return vle::devs::infinity;
//        } else {
//            return 0;
//        }
//    }
//
//    virtual void output(const vle::devs::Time& /* time */,
//            vle::devs::ExternalEventList& output) const
//    {
//        if (mPhase == ADDED) {
//            std::map < std::string, double >::const_iterator it;
//
//            for (int i = 0; i!=mNumber; i++) {
//                std::string modelName((boost::format("%1%-%2%_AreaActive") % mPrefix % i).str());
//                output.push_back(buildEventWithAString("addModelSumAreaActive", "name", modelName));
//            }
//
//
//            for (it = mBuffer.begin(); it != mBuffer.end(); it++) {
//                vle::devs::ExternalEvent* ee = buildEvent(it->first);
//
//                ee << vle::devs::attribute("name", it->first);
//                ee << vle::devs::attribute("value", it->second);
//                output.push_back(ee);
//            }
//        }
//    }
//
//    virtual void internalTransition(const vle::devs::Time& /* time */)
//    {
//        switch (mPhase) {
//        case IDLE:
//            throw vle::utils::InternalError(
//                    vle::fmt(_("[%1%] Buffer: Phase IDLE")) %
//                    getModelName());
//        case ADDED: mPhase=SENDED; break;
//        case SENDED: mPhase=RESTORE; etape3(); break;
//        case RESTORE: mPhase=IDLE; mAddModel= false; break;
//        }
//    }
//
//    virtual void externalTransition(const vle::devs::ExternalEventList& event,
//            const vle::devs::Time& time)
//    {
//        typedef vle::devs::ExternalEventList::const_iterator const_iterator;
//        typedef std::map < std::string, double >::iterator iterator;
//
//        // Nettoyage Buffer
//        updateTime(time);
//
//        // Construction du Buffer si les evts contiennent des valeurs
//        for (const_iterator it = event.begin(); it != event.end(); ++it) {
//            if ((*it)->existAttributeValue("value")) {
//                std::string name = (*it)->getStringAttributeValue("name");
//                double value = (*it)->getDoubleAttributeValue("value");
//
//                std::pair < iterator, bool > r;
//
//                r = mBuffer.insert(std::make_pair < std::string, double >(name, value));
//                if(r.second == true) {
//                    mWaiting --;
//                    //TraceModel("mBuffer add");
//                } else {
//                    //TraceModel("mBuffer not add");
//                }
//                assert(mWaiting >= 0);
//            }
//        }
//
//        // Separer le remplissage du buffer du fait de recevoir un evenement "add"
//        for (const_iterator it = event.begin(); it != event.end(); ++it) {
//            if ((*it)->onPort("add")) {
//                mParameter = vle::value::toMapValue((*it)->getMapAttributeValue("parameter").clone());
//                mAddModel = true;
//            }
//        }
//
//        TraceModel(vle::fmt("%1%") % mWaiting);
//
//        if (mAddModel == true and mWaiting == 0) {
//            mPhase = ADDED;
//            etape1();
//        } else {
//            mPhase = IDLE;
//        }
//    }
//
//    void updateTime(const vle::devs::Time& time)
//    {
//        if (time > mLastTime) {
//            mBuffer.clear();
//            mAddModel = false;
//            mWaiting = getModel().getInputPortNumber() - 1;
//        }
//        mLastTime = time;
//    }
//
//    void etape1()
//    {
//
//
//    }
//
//    void etape3() {
//        for (int i = 0; i!=mNumber; i++) {
//            const std::string current = (vle::fmt("%1%-%2%") % mPrefix % i).str();
//
//            removeConnection("Buffer", "ThermalTime", current, "ThermalTime");
//            removeConnection("Buffer", "ActionTemp", current, "ActionTemp");
//            removeConnection("Buffer", "TempEff", current, "TempEff");
//        }
//
//
//        std::ofstream file("output.vpz");
//        dump(file);
//
//        // TODO delModel("Buffer");
//
//    }

private:
    //inputs
    Var ThermalTime;//sync

//    enum Phase {IDLE, ADDED, SENDED, RESTORE};
//    std::map < std::string, double > mBuffer;
//    Phase mPhase;
//    vle::devs::Time mLastTime;
//    int mWaiting;
//    bool mAddModel;
//    std::string mPrefix;
//    int mNumber;
//    vle::value::Map* mParameter;
//    std::vector < unsigned int > E_InitSpace;

};

DECLARE_EXECUTIVE(UnitPilotGraph)

}}} //namespaces



