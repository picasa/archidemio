/**
  * @file Unit.cpp
  * @author P. Casadebaig
  * \brief Classe de modèle d'unité fonctionnelle.
  * Calcule la croissance de l'hôte et celle du pathogène
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

#include <vle/extension/difference-equation/Multiple.hpp>
#include <vle/devs/DynamicsDbg.hpp>

namespace vd = vle::devs;
namespace ve = vle::extension;
namespace vv = vle::value;

namespace Crop {

class Unit : public ve::DifferenceEquation::Multiple
{
public:
    Unit(
       const vd::DynamicsInit& atom,
       const vd::InitEventList& events)
        : ve::DifferenceEquation::Multiple(atom, events)
    {
        // Parametres
        // exemple de tests sur la presence de la valeur des paramètres paquet 2CV
        // = (events.getInt("C_Crop") =! NULL) ? events.getInt("C_Crop"):1 
        C_Crop = events.getInt("C_Crop");
        P_AreaMax = events.getDouble("P_AreaMax");
        P_ExpansionTT = events.getDouble("P_ExpansionTT");
		P_ExpansionSlope = events.getDouble("P_ExpansionSlope");
		P_SenescenceTT = events.getDouble("P_SenescenceTT");
		P_SenescenceSlope = events.getDouble("P_ExpansionSlope");
        P_ElongationMax = events.getDouble("P_ElongationMax");
		P_ElongationSlope = events.getDouble("P_ElongationSlope");
		P_ElongationTT = events.getDouble("P_ElongationTT");
        P_Porosity = events.getDouble("P_Porosity");
        P_ReceptivityTT = events.getDouble("P_ReceptivityTT");
        E_RateAutoDeposition = events.getDouble("E_RateAutoDeposition");
        E_RateAlloDeposition = events.getDouble("E_RateAlloDeposition");
        E_InfectiousPeriod = events.getDouble("E_InfectiousPeriod");
        E_LatentPeriod = events.getDouble("E_LatentPeriod");
        E_OutDegree = events.getInt("E_OutDegree");
        
        // Variables d'entrée synchrones/asynchrone
        // si ThermalTime n'est plus utilisé, enlever les connections dynamiques depuis UnitConstruction
		TempEff = createSync("TempEff"); 
		ThermalTime = createSync("ThermalTime"); 
        ActionTemp = createSync("ActionTemp");
        In = createNosync("in");
        
        // Variables d'état
        RateAreaExpansion = createVar("RateAreaExpansion");
        RateAreaSenescence = createVar("RateAreaSenescence");
		AreaExpansion = createVar("AreaExpansion");
		AreaActive = createVar("AreaActive");
        AreaHealthy = createVar("AreaHealthy");
        AreaLatent = createVar("AreaLatent");
        AreaInfectious = createVar("AreaInfectious");        
        AreaRemoved = createVar("AreaRemoved");
        AreaRemovedByDesease = createVar("AreaRemovedByDesease");
        ScoreArea = createVar("ScoreArea");
        AreaSenescence = createVar("AreaSenescence");
        AreaDeseased = createVar("AreaDeseased");
        RateAutoDeposition = createVar("RateAutoDeposition");
        RateAlloDeposition = createVar("RateAlloDeposition");
        LatentPeriod = createVar("LatentPeriod");
        InfectiousPeriod = createVar("InfectiousPeriod");
        Out = createVar("out");
        OutDeposition = createVar("OutDeposition");
        InDeposition = createVar("InDeposition");
        Receptivity = createVar("Receptivity");
        ThermalAge = createVar("ThermalAge");
        InitQuantity = createVar("InitQuantity");
        CropState = createVar("CropState");
        Porosity = createVar("Porosity");
        Elongation = createVar("Elongation");
        RateElongation = createVar("RateElongation");
    }

    virtual ~Unit()
    {}

//@@begin:compute@@
virtual void compute(const vd::Time& /*time*/)
{
    // Quantité d'inoculum primaire (perturbé par modèle Initation)
    InitQuantity = 0;
    
    /* Réception de spores : dépendante de la veille (t-1) 
     * Géré par 4 variables (pour permettre les perturbations) :
     * In/Out : assurent la connexion entre unités, mappés vers les ports in/out
     * InDeposition/OutDeposition : variables "biologiques", decrit le fonctionnement
     */
    
    /* 2 solutions pour agir sur les connexions entre unités :
     * - modifier le graphe de connexions, et les connexions entre modèle en conséquence
     * - ponderer les flux dejà existants (ici un test sur une condition environnementale)
     */ 
    double InDeposition_tmp = In(-1);
    if (TempEff() > 40) {               // en attendant de faire référence à un état du système.
        InDeposition_tmp = 0;
    }
    InDeposition = InDeposition_tmp;  

    // Age thermique de l'unité
    ThermalAge = TempEff() + ThermalAge(-1);
    
    /* Croissance en hauteur de l'unité
     * - Cinétique identique à l'expansion foliaire
     * - Synchrone avec l'expansion (P_ExpansionTT == P_ElongationTT)
     */ 
    RateElongation = TempEff() * (P_ElongationMax * P_ElongationSlope) * exp(-P_ElongationSlope * (ThermalAge() - P_ElongationTT)) / pow((1 + exp(-P_ElongationSlope * (ThermalAge() - P_ElongationTT))),2);
         
    Elongation = Elongation(-1) + RateElongation();
    
    /* Receptivité de tissus : Réponse identique pour toute les unités
     * Linéaire decroissant : Receptivity = fmax(- 1/1000 * ThermalAge() +1, 0);
     * Linéaire croissant : Receptivity = fmin(1/1000 * ThermalAge() + 0.2, 1.2);
     * Sigmoide : Receptivity = 1 / (1 + exp(-0.005 * (ThermalAge() - 500))) + 0.2;
     * R : rho <- function(x, a, b) {1 / (1 + exp(-a * (x - b))) + 0.2}
     * (paramètres : pente, asymptotes (haut et bas), abscisse pt d'inflexion)
     */
    switch (C_Crop) {
        case 1: Receptivity = 1 - (1 / (1 + exp(-0.001 * (ThermalAge() - P_ReceptivityTT)))); // Pdt (Décroissante)
        case 2: Receptivity = 1 / (1 + exp(-0.005 * (ThermalAge() - P_ReceptivityTT))) + 0.2; // Pois (Croissante)
        case 3: Receptivity = 0;
    } 
    
    /* Periode de latence : durée en j.
     * Pour les parasites biotrophes, cette variable pèse bcp dans le dev de l'épidémie (Rapilly1990, Zadoks1971)
     * Cible de contraintes env & resistance plante. 
     */
    LatentPeriod = E_LatentPeriod; 
     
    /* Periode infectieuse
     */
    InfectiousPeriod = E_InfectiousPeriod;
    
    /* Vitesse de croissance
     * - Si ThermalTime() est utilisé la valeur de P_ExpansionTT doit être recalculée (Pilote.cpp) en temps thermique
     * - sinon on utilise ThermalAge()
     */
    RateAreaExpansion = TempEff() * (P_AreaMax * P_ExpansionSlope) * exp(-P_ExpansionSlope * (ThermalAge() - P_ExpansionTT)) / pow((1 + exp(-P_ExpansionSlope * (ThermalAge() - P_ExpansionTT))),2);
    
    /* Vitesse de senescence : pose un problème pour déterminer l'asymptote
     * (paramètre) alors qu'il depends de l'évolution de la maladie. 
     * Plutôt que de le résoudre analytiquement, on conditionne cette dérivée.  
     */ 
    double RateAreaSenescence_tmp = TempEff() * (P_AreaMax * P_SenescenceSlope) * exp(-P_SenescenceSlope * (ThermalAge() - P_SenescenceTT)) / pow((1 + exp(-P_SenescenceSlope * (ThermalAge() - P_SenescenceTT))),2);
    if ((AreaHealthy(-1) <= 0) and (AreaRemoved(-1) > P_AreaMax)) {
        RateAreaSenescence_tmp = 0;
    }
    RateAreaSenescence = RateAreaSenescence_tmp;
    
    // Surface totale : intégration de RateAreaExpansion
    AreaExpansion = AreaExpansion(-1) + RateAreaExpansion();
    
    // Surface senescente : intégration de RateAreaSenescence
    AreaSenescence = fmin(AreaSenescence(-1) + RateAreaSenescence(),P_AreaMax);
    
    // Vitesse d'infection au sein de l'unité (autodeposition)
    RateAutoDeposition = E_RateAutoDeposition * ActionTemp();
    
    // Vitesse d'infection entre unités (allodeposition)
    RateAlloDeposition = E_RateAlloDeposition * ActionTemp();
        
    // Surface saine, création par la croissance, destruction par l'infection (intra / extra) et la senescence naturelle
    AreaHealthy = fmax(
        AreaHealthy(-1)
        -InitQuantity() 
        +RateAreaExpansion()
        -(RateAutoDeposition() * AreaInfectious(-1) * AreaHealthy(-1)/AreaExpansion(-1)) * Receptivity()
        -(InDeposition() * AreaHealthy(-1)/AreaExpansion(-1)) * Receptivity()
        -(RateAreaSenescence() * AreaHealthy(-1)/AreaActive(-1)),0);
        
    // Surface latente, création par l'infection, destruction par la propagation du pathogene et la senescence
    AreaLatent = fmax(
        AreaLatent(-1)
        +InitQuantity() 
        +(RateAutoDeposition() * AreaInfectious(-1) * AreaHealthy(-1)/AreaExpansion(-1)) * Receptivity()
        +(InDeposition()* AreaHealthy(-1)/AreaExpansion(-1)) * Receptivity()
        -(1/LatentPeriod() * AreaLatent(-1)) 
        -(RateAreaSenescence() * AreaLatent(-1)/AreaActive(-1)),0);
        
    // Surface infectieuse, creation par la propagation du pathogene, destruction par le pathogene
    AreaInfectious = fmax(
        AreaInfectious(-1) 
        +(1/LatentPeriod() * AreaLatent(-1)) 
        -(1/InfectiousPeriod() * AreaInfectious(-1)) 
        -(RateAreaSenescence() * AreaInfectious(-1)/AreaActive(-1)),0);
    
    // Surface détruite par le pathogène et la senescence naturelle
    AreaRemoved = fmin( 
        AreaRemoved(-1) 
        +(1/InfectiousPeriod() * AreaInfectious(-1)) 
        +RateAreaSenescence(),P_AreaMax);  
        
    // Surface détruite par la maladie
    AreaRemovedByDesease = 
        AreaRemovedByDesease(-1) 
        +(1/InfectiousPeriod() * AreaInfectious(-1));
        
    // % de Surface détruite par la maladie = note de surface
    ScoreArea = AreaRemovedByDesease() / AreaExpansion();
    
    // Surface active : différence totale - senescence
    AreaActive = fmax(AreaExpansion() - AreaRemoved(),0);
    
    /* Porosité : 
     *  - action sur l'émission de spores
     *  - fonction de la surface / encombrement dans l'unité
     *  - bornée à 1
     *  - plus la pente est faible (P_ElongationSlope), plus la transition poreux -> dense est rapide
     */ 
    Porosity = fmin(
        P_Porosity * AreaActive() / Elongation(),1);
    
    // Surface malade = total des surfaces infectée (L + I + R causé par maladie)
    AreaDeseased =
        + AreaLatent() 
        + AreaInfectious() 
        + (1/InfectiousPeriod() * AreaInfectious(-1));
        
    /* Emission de spores totale de l'unité
     * 1. proportion de la surface infectieuse de l'unité
     * 2. réduite par la porosité de l'unité / couvert
     */
    OutDeposition = RateAlloDeposition() * AreaInfectious() * Porosity();
    
    // Emission de spores pondérée 
    // C'est reçu par chacun des voisins, donc ce flux est fonction du voisinage sortant
    Out = OutDeposition() / E_OutDegree;
    
    
}
//@@end:compute@@

//@@begin:initValue@@
virtual void initValue(const vd::Time& /*time*/)
{
    RateAreaExpansion = 0.0;
    RateAreaSenescence = 0.0;
    AreaExpansion = 0.001;
    AreaActive = 0.001;
    AreaHealthy = 0.001;
    AreaLatent = 0.0;
    AreaInfectious = 0.0;
    AreaRemoved = 0.0;
    AreaRemovedByDesease = 0.0;
    ScoreArea = 0.0;
    AreaSenescence = 0.0;
    AreaDeseased = 0.0;
    RateAutoDeposition = E_RateAutoDeposition;    
    RateAlloDeposition = E_RateAlloDeposition;    
    LatentPeriod = E_LatentPeriod;
    InfectiousPeriod = E_InfectiousPeriod;
    OutDeposition = 0.0;
    InDeposition = 0.0;
    Out = 0.0;
    Receptivity = 0.2;
    ThermalAge = 0.0;
    InitQuantity = 0.0;
    CropState = 0.0;
    Porosity = 1.0;
    RateElongation = 0.0;
    Elongation = 0.001;
    
}
//@@end:initValue@@

private:
//@@begin:user@@
//@@end:user@@

    // Parametres
    int C_Crop; /** Paramètre : Type de culture */
    double P_AreaMax; /** Paramètre : surface potentielle d'une unité, Unité : m^2 */
    double P_ExpansionTT; /** Paramètre : date de demi-expansion de la surface d'une unité */
    double P_ExpansionSlope; /**<Paramètre : acceleration de l'expansion de la surface d'une unité */    
    double P_SenescenceTT; /** Paramètre : date de demi-senescence de la surface d'une unité */
    double P_SenescenceSlope ; /** Paramètre : acceleration de la senescence de la surface d'une unité */  
    double P_ElongationMax; /** Paramètre : longueur potentielle d'une unité, Unité : m */
    double P_ElongationSlope ; /** Paramètre : acceleration de la croissance en hauteur d'une unité */  
    double P_ElongationTT ; /** Paramètre : date de demi-elongation d'une unité */  
    double P_Porosity ; /** Paramètre : Porosité de l'unité fonctionnelle */  
    double P_ReceptivityTT ; /** Paramètre : temps de déclenchement de la receptivité de l'unité */  
    double E_RateAutoDeposition; /** Paramètre : Vitesse de transmission de la maladie */
    double E_RateAlloDeposition; /** Paramètre : Modulation du taux d'allodeposition */
    double E_InfectiousPeriod; /** Paramètre : durée de la période infectieuse */
    double E_LatentPeriod; /** Paramètre : durée de la période de latence */
    int E_OutDegree; /** Paramètre : nombre de voisins "sortants" */
    
    // Entrées 
    Sync ActionTemp;
    Sync TempEff;
    Sync ThermalTime;
    Nosync In;
    
    // Variables d'état
    Var RateAreaExpansion;
    Var RateAreaSenescence;
    Var AreaExpansion;
    Var AreaActive;
    Var AreaHealthy;
    Var AreaLatent;
    Var AreaInfectious;
    Var AreaRemoved;
    Var AreaRemovedByDesease;
    Var ScoreArea;
    Var AreaSenescence;
    Var AreaDeseased;
    Var RateAutoDeposition;
    Var RateAlloDeposition;
    Var LatentPeriod;
    Var InfectiousPeriod;
    Var OutDeposition;
    Var InDeposition;
    Var Receptivity;
    Var ThermalAge;
    Var InitQuantity;
    Var CropState;
    Var Porosity;
    Var RateElongation;
    Var Elongation;
    Var Out;
};

} // namespace Unit

//DECLARE_DIFFERENCE_EQUATION_MULTIPLE_DBG(Crop::Unit)
DECLARE_DYNAMICS_DBG(Crop::Unit)
