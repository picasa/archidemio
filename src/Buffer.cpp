/**
 * @file Buffer.cpp
 * @author The VLE Development Team
 * See the AUTHORS or Authors.txt file
 */

/*
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

#include <vle/devs/Dynamics.hpp>
#include <vle/utils/Debug.hpp>
#include <vle/utils/i18n.hpp>

#include <vle/devs/DynamicsDbg.hpp>


class Buffer : public vle::devs::Dynamics
{
public:
    Buffer(const vle::devs::DynamicsInit& init,
           const vle::devs::InitEventList& events) :
        vle::devs::Dynamics(init, events)
    { }

    virtual ~Buffer()
    { }

    virtual void output(const vle::devs::Time& /* time */,
                        vle::devs::ExternalEventList& output) const
    {
        if (mPhase == SEND) {
            std::map < std::string, double >::const_iterator it =
                mValues.begin();

            while (it != mValues.end()) {
                vle::devs::ExternalEvent* ee = buildEvent(it->first);
                
                ee << vle::devs::attribute("name", it->first);
                ee << vle::devs::attribute("value", it->second);
                output.addEvent(ee);
                ++it;
            }
        } else if (mPhase == FULL) {
            output.addEvent(buildEvent("ok"));
        } else if (mPhase == SEND_CANCEL) {
            output.addEvent(buildEvent("cancel"));
        }
    }

    virtual vle::devs::Time init(const vle::devs::Time& /* time */)
    {
        if (not getModel().existInputPort("send")) {
            throw vle::utils::InternalError(
                vle::fmt(_("[%1%] Buffer: send inport missing")) %
                getModelName());
        }

        if (not getModel().existOutputPort("ok")) {
            throw vle::utils::InternalError(
                vle::fmt(_("[%1%] Buffer: ok outport missing")) %
                getModelName());
        }

        if (not getModel().existOutputPort("cancel")) {
            throw vle::utils::InternalError(
                vle::fmt(_("[%1%] Buffer: cancel outport missing")) %
                getModelName());
        }

        if (getModel().getOutputPortNumber() !=
            getModel().getInputPortNumber() + 1) {
            throw vle::utils::InternalError(
                vle::fmt(_("[%1%] Buffer: wrong output port number")) %
                getModelName());
        }

        mVariableNumber = getModel().getInputPortNumber() - 1;
        mLastTime = vle::devs::Time::negativeInfinity;
        mPhase = IDLE;
        mWaiting = mVariableNumber;
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

    virtual void internalTransition(const vle::devs::Time& /* time */)
    {
        if (mPhase == SEND) {
            if (mWaiting == 0) {
                mPhase = IDLE;
            }
        } else if (mPhase == FULL) {
            mPhase = IDLE;
        } else if (mPhase == SEND_CANCEL) {
            mPhase = IDLE;
        }
    }

    virtual void externalTransition(const vle::devs::ExternalEventList& event,
                                    const vle::devs::Time& time)
    {
        vle::devs::ExternalEventList::const_iterator it = event.begin();

        while (it != event.end()) {
            if ((*it)->onPort("send")) {
                if (mWaiting == 0) {
                    mPhase = SEND;
                }
            } else {
                if ((*it)->existAttributeValue("value")) {
                    std::string name = (*it)->getStringAttributeValue("name");
                    double value = (*it)->getDoubleAttributeValue("value");

                    if (mLastTime < time) {
                        mValues.clear();
                        mLastTime = time;
                        mWaiting = mVariableNumber;
                        mPhase = SEND_CANCEL;
                    }
                    mValues[name] = value;
                    --mWaiting;
                    if (mWaiting == 0) {
                        mPhase = FULL;
                    }
                }
            }
            ++it;
        }
    }

private:
    enum Phase { IDLE, SEND, FULL, SEND_CANCEL };

    std::map < std::string, double > mValues;
    Phase mPhase;
    vle::devs::Time mLastTime;
    unsigned int mVariableNumber;
    unsigned int mWaiting;
};


//DECLARE_NAMED_DYNAMICS(Buffer, Buffer);
DECLARE_NAMED_DYNAMICS_DBG(Buffer, Buffer);
