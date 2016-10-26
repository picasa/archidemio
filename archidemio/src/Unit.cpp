/**
 * @file Unit.cpp
 * @author P. Casadebaig
 * \brief Classe de modele d'unite fonctionnelle.
 * Calcule la croissance de l'hote et celle du pathogene
 */

/*
 * @@tagdynamic@@
 * @@tagdepends: vle.discrete-time @@endtagdepends
 */

#include <vle/DiscreteTime.hpp>

#include <vle/utils/DateTime.hpp>
#include <iomanip>

namespace vd = vle::devs;
namespace vv = vle::value;

namespace vle {
namespace discrete_time {
namespace archidemio {

class Unit : public DiscreteTimeDyn
{
public:
    Unit(const vd::DynamicsInit& init, const vd::InitEventList& events)
    : DiscreteTimeDyn(init, events)
    {
        // Parametres
        C_Crop = events.getInt("C_Crop");
        P_AreaMax = events.getDouble("P_AreaMax");
        P_ExpansionTT = events.getDouble("P_ExpansionTT");
        P_ExpansionSlope = events.getDouble("P_ExpansionSlope");
        P_SenescenceTT = events.getDouble("P_SenescenceTT");
        P_SenescenceSlope = events.getDouble("P_SenescenceSlope");
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

        // Inputs
        // si ThermalTime n'est plus utilisé, enlever
        //les connections dynamiques depuis UnitConstruction
        TempEff.init(this, "TempEff", events);
        ThermalTime.init(this, "ThermalTime", events);
        ActionTemp.init(this, "ActionTemp", events);
        InitQuantity.init(this, "InitQuantity", events);
        In.init(this, "In", events);

        // Variables d'état
        RateAreaExpansion.init(this, "RateAreaExpansion", events);
        RateAreaSenescence.init(this, "RateAreaSenescence", events);
        AreaExpansion.init(this, "AreaExpansion", events);
        AreaActive.init(this, "AreaActive", events);
        AreaHealthy.init(this, "AreaHealthy", events);
        AreaLatent.init(this, "AreaLatent", events);
        AreaInfectious.init(this, "AreaInfectious", events);
        AreaRemoved.init(this, "AreaRemoved", events);
        AreaRemovedByDesease.init(this, "AreaRemovedByDesease", events);
        ScoreArea.init(this, "ScoreArea", events);
        AreaSenescence.init(this, "AreaSenescence", events);
        AreaDeseased.init(this, "AreaDeseased", events);
        RateAutoDeposition.init(this, "RateAutoDeposition", events);
        RateAlloDeposition.init(this, "RateAlloDeposition", events);
        LatentPeriod.init(this, "LatentPeriod", events);
        InfectiousPeriod.init(this, "InfectiousPeriod", events);
        Out.init(this, "Out", events);
        OutDeposition.init(this, "OutDeposition", events);
        InDeposition.init(this, "InDeposition", events);
        Receptivity.init(this, "Receptivity", events);
        ThermalAge.init(this, "ThermalAge", events);
        CropState.init(this, "CropState", events);
        Porosity.init(this, "Porosity", events);
        Elongation.init(this, "Elongation", events);
        RateElongation.init(this, "RateElongation", events);

        //initialization
        RateAreaExpansion.init_value(0.0);
        RateAreaSenescence.init_value(0.0);
        AreaExpansion.init_value(0.001);
        AreaActive.init_value(0.001);
        AreaHealthy.init_value(0.001);
        AreaLatent.init_value(0.0);
        AreaInfectious.init_value(0.0);
        AreaRemoved.init_value(0.0);
        AreaRemovedByDesease.init_value(0.0);
        ScoreArea.init_value(0.0);
        AreaSenescence.init_value(0.0);
        AreaDeseased.init_value(0.0);
        RateAutoDeposition.init_value(E_RateAutoDeposition);
        RateAlloDeposition.init_value(E_RateAlloDeposition);
        LatentPeriod.init_value(E_LatentPeriod);
        InfectiousPeriod.init_value(E_InfectiousPeriod);
        OutDeposition.init_value(0.0);
        InDeposition.init_value(0.0);
        Out.init_value(0.0);
        Receptivity.init_value(0.2);
        ThermalAge.init_value(0.0);
        CropState.init_value(0.0);
        Porosity.init_value(1.0);
        RateElongation.init_value(0.0);
        Elongation.init_value(0.001);
    }

    virtual ~Unit()
    {}

    void compute(const vle::devs::Time& t)
    {
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
        RateElongation = TempEff() * (P_ElongationMax * P_ElongationSlope) *
                exp(-P_ElongationSlope * (ThermalAge() - P_ElongationTT)) /
                pow((1 + exp(-P_ElongationSlope * (ThermalAge() - P_ElongationTT))),2);

        Elongation = Elongation(-1) + RateElongation();

        /* Receptivité de tissus : Réponse identique pour toute les unités
         * Linéaire decroissant : Receptivity = fmax(- 1/1000 * ThermalAge() +1, 0);
         * Linéaire croissant : Receptivity = fmin(1/1000 * ThermalAge() + 0.2, 1.2);
         * Sigmoide : Receptivity = 1 / (1 + exp(-0.005 * (ThermalAge() - 500))) + 0.2;
         * R : rho <- function(x, a, b) {1 / (1 + exp(-a * (x - b))) + 0.2}
         * (paramètres : pente, asymptotes (haut et bas), abscisse pt d'inflexion)
         */
        switch (C_Crop) {
        case 1:
            Receptivity = 1 - (1 / (1 + exp(-0.001 * (ThermalAge() - P_ReceptivityTT)))); // Pdt (Décroissante)
            break;
        case 2:
            Receptivity = 1 / (1 + exp(-0.005 * (ThermalAge() - P_ReceptivityTT))) + 0.2; // Pois (Croissante)
            break;
        case 3:
            Receptivity = 0;
            break;
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
        RateAreaExpansion = TempEff() * (P_AreaMax * P_ExpansionSlope) *
                exp(-P_ExpansionSlope * (ThermalAge() - P_ExpansionTT)) /
                pow((1 + exp(-P_ExpansionSlope * (ThermalAge() - P_ExpansionTT))),2);



        /* Vitesse de senescence : pose un problème pour déterminer l'asymptote
         * (paramètre) alors qu'il depends de l'évolution de la maladie.
         * Plutôt que de le résoudre analytiquement, on conditionne cette dérivée.
         */
        double RateAreaSenescence_tmp = TempEff() * (P_AreaMax * P_SenescenceSlope) *
                exp(-P_SenescenceSlope * (ThermalAge() - P_SenescenceTT)) /
                pow((1 + exp(-P_SenescenceSlope * (ThermalAge() - P_SenescenceTT))),2);
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

    // Parameters
    int C_Crop; /** Param : Type de culture */
    double P_AreaMax; /** Param : surface potentielle d'une unité, Unité : m^2 */
    double P_ExpansionTT; /** Param : date de demi-expansion de la surface d'une unité */
    double P_ExpansionSlope; /**<Param : acceleration de l'expansion de la surface d'une unité */
    double P_SenescenceTT; /** Param : date de demi-senescence de la surface d'une unité */
    double P_SenescenceSlope ; /** Param : acceleration de la senescence de la surface d'une unité */
    double P_ElongationMax; /** Param : longueur potentielle d'une unité, Unité : m */
    double P_ElongationSlope ; /** Param : acceleration de la croissance en hauteur d'une unité */
    double P_ElongationTT ; /** Param : date de demi-elongation d'une unité */
    double P_Porosity ; /** Param : Porosité de l'unité fonctionnelle */
    double P_ReceptivityTT ; /** Param : temps de déclenchement de la receptivité de l'unité */
    double E_RateAutoDeposition; /** Param : Vitesse de transmission de la maladie */
    double E_RateAlloDeposition; /** Param : Modulation du taux d'allodeposition */
    double E_InfectiousPeriod; /** Param : durée de la période infectieuse */
    double E_LatentPeriod; /** Param : durée de la période de latence */
    int E_OutDegree; /** Param : nombre de voisins "sortants" */

    // Inputs
    Var ActionTemp;//sync
    Var TempEff;//sync
    Var ThermalTime;//sync
    Var InitQuantity;//sync
    Var In;//no sync

    // State variables
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
    Var CropState;
    Var Porosity;
    Var RateElongation;
    Var Elongation;
    Var Out;
};

} // namespace archidemio
} // namespace discrete_time
} // namespace vle

DECLARE_DYNAMICS(vle::discrete_time::archidemio::Unit)

