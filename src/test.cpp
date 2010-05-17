/**
  * @file test.cpp
  * @author ...
  * ...
  * @@tag DifferenceEquationMultiple@@namespace:archidemio;class:test;par:;sync:;nosync:@@end tag@@
  */

#include <vle/extension/DifferenceEquation.hpp>

namespace vd = vle::devs;
namespace ve = vle::extension;
namespace vv = vle::value;

namespace archidemio {

class test : public ve::DifferenceEquation::Multiple
{
public:
    test(
       const vd::DynamicsInit& atom,
       const vd::InitEventList& evts)
        : ve::DifferenceEquation::Multiple(atom, evts)
    {
    }

    virtual ~test()
    {}

//@@begin:compute@@
virtual void compute(const vd::Time& /*time*/)
{ }
//@@end:compute@@

//@@begin:initValue@@
virtual void initValue(const vd::Time& /*time*/)
{ }
//@@end:initValue@@

private:
//@@begin:user@@
//@@end:user@@

};

} // namespace archidemio

DECLARE_DYNAMICS(archidemio::test)

