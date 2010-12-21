
#include <vle/extension/DifferenceEquationDbg.hpp>

namespace vd = vle::devs;
namespace ve = vle::extension;
namespace vv = vle::value;

namespace Crop {

class CropLeafArea : public ve::DifferenceEquation::Multiple
{
public:
    CropLeafArea(
       const vd::DynamicsInit& atom,
       const vd::InitEventList& events)
        : ve::DifferenceEquation::Multiple(atom, events)
    {
        C_Density = events.getDouble("C_Density");
        
        LAI = createVar("LAI");
        CropAreaActive = createSync("CropAreaActive");
    }

    virtual ~CropLeafArea()
    {}

virtual void compute(const vd::Time& /*time*/)
{
    // On utilise directement la surface d'une unité à ce stade, 
    // il faudra sommer ces valeurs quand il y aura n strates.
    
    // Indice de surface foliaire de la culture
    LAI = CropAreaActive() * C_Density;
    
    // Indice relatif de surface foliaire : indication de l'état de dev de la culture
    //RLAI = CropAreaActive() / P_AreaMax;
    
    // Surface totale atteinte par le pathogène (infectée, infectieuse, détruite)
    //CropDeseased = AreaDeseased();
    
     

}
virtual void initValue(const vd::Time& /*time*/)
{
	LAI = 0.0;
}

private:

    double C_Density;
    Var LAI;
    Sync CropAreaActive;
};

} // namespace Crop
DECLARE_DIFFERENCE_EQUATION_MULTIPLE_DBG(Crop::CropLeafArea)

