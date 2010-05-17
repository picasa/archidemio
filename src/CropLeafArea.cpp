/**
  * @file CropLeafArea.cpp
  * @author ...
  * ...
  * @@tag DifferenceEquationMultiple@@namespace:Crop;class:CropLeafArea;par:C_Density,*|P_AreaMax,*|;sync:AreaActive|AreaDeseased|;nosync:@@end tag@@
  */

#include <vle/extension/DifferenceEquation.hpp>

namespace vd = vle::devs;
namespace ve = vle::extension;
namespace vv = vle::value;

namespace Crop {

class CropLeafArea : public ve::DifferenceEquation::Multiple
{
public:
    CropLeafArea(
       const vd::DynamicsInit& atom,
       const vd::InitEventList& evts)
        : ve::DifferenceEquation::Multiple(atom, evts)
    {
        C_Density = vv::toDouble(evts.get("C_Density"));
        LAI = createVar("LAI");
        CropAreaActive = createSync("CropAreaActive");
    }

    virtual ~CropLeafArea()
    {}

//@@begin:compute@@
virtual void compute(const vd::Time& /*time*/)
{
    // On utilise directement la surface d'une strate à ce stade, 
    // il faudra sommer ces valeurs quand il y aura n strates.
    
    // Indice de surface foliaire de la culture
    LAI = CropAreaActive() * C_Density;
    
    // Indice relatif de surface foliaire : indication de l'état de dev de la culture
    //RLAI = CropAreaActive() / P_AreaMax;
    
    // Surface totale atteinte par le pathogène (infectée, infectieuse, détruite)
    //CropDeseased = AreaDeseased();
    
     

}
//@@end:compute@@

//@@begin:initValue@@
virtual void initValue(const vd::Time& /*time*/)
{ }
//@@end:initValue@@

private:
//@@begin:user@@
//@@end:user@@

    double C_Density;
    Var LAI;
    Sync CropAreaActive;
};

} // namespace Crop

DECLARE_DYNAMICS(Crop::CropLeafArea)

