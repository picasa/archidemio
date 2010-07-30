### Regroupe les méthodes pour réaliser une analyse de sensibilité du modèle 

### Bibliothèques & chemins
#library(gdata)
library(ggplot2)
library(lattice)
library(sensitivity)

### Fonctions de simulation & Paramétrage
source("fonctions.R")
#rvle.getDuration(f)

### Sensibilité : plutôt utiliser le gestionnaire de simulation de VLE
### puis calculer des indices depuis les sorties.

## 1. Plan d'expérience géré par VLE
f <- rvle.open("1D_0.7.vpz", "archidemio")
# Pour chaque paramètre, on fixe son intervalle d'incertitude, 
# efface la valeur nominale et assigne la gamme au vpz.
# rvle.listConditionPorts(f,"condParametres")
#f.factors.name <- rvle.listConditionPorts(f,"condParametres")

factors.name <- c("E_InitTime","E_InfectiousPeriod",
	"E_RateDeseaseTransmission","E_RateAlloDeposition","E_InitQuantity",
	"E_LatentPeriod","P_UnitTTSen")

factors.bounds <- data.frame(
	name = factors.name,
	def = c(50, 10, 0.5, 0.2, 0.01, 5, 1000),
	min = c(10, 1, 0, 0, 0, 1, 700),
	max = c(140, 15, 1, 1, 0.3, 15, 1300)
)

factors <- factors.name[1:7]
bounds <- factors.bounds[1:7,]


# Construire le plan
# plan complet : f.plan <- expand.grid(va, vb)
f.as <- getPlanMorris(factors, binf=bounds$min, bsup=bounds$max, S=100, K=5)
f.as <- getPlanFast(factors, bounds, n=200)
f.as <- getPlanSobol(factors, bounds, n=100)

f.plan <- f.as$X

# Mise en place dans VLE
rvle.addPlanCondition(f, "condParametres", factors=factors)
#rvle.setTotalCombination(f,1,1) 	# Complet
#regular.fraction() 				# Fractionné
rvle.setLinearCombination(f,1,1) 	# Linéaire
# On vérifie le plan dans le vpz.
# rvle.getAllConditionPortValues(f, "condParametres")


## 2. Simulation (~ 8 sim/s)
# rvle.save(f, "plan.vpz") 
# La simulation depuis ce vpz permet de générer les vpz unitaires
# vle -m -l -o 2 -z -P archidemio plan.vpz
system.time(d <- rvle.runManagerThread(f,2))


## 3. Indices de sensibilité
# Calcul de la quantité d'interet
# TODO : pas de boucles, une seule fonction
x <- lapply(d, melt, id=c("time","Top model,Crop:CropPhenology.ThermalTime"))

# Intégration : somme sur t & unité
y=NULL
for (i in 1:length(x)) {
	y=c(y, sum(x[[i]]$value, na.rm=T))
}

f.out = data.frame(f.plan, y)

# Decoupling
tell(f.as, f.out$y)

# Graphiques
xyplot(data=x[[1]], value ~ time, groups=variable, type="l")
xyplot(data=f.out, y ~ 1:dim(f.out)[1] , type="l")

plot(f.as)

plot.morris(f.as, factors)
plot.fast(f.as, factors)







### Bazar

n=100
X1 <- matrix(runif(length(factors) * n), nrow=n)
X2 <- matrix(runif(length(factors) * n), nrow=n)
X1.i <- lhs2bounds(X1, bounds, factors)
X2.i <- lhs2bounds(X2, bounds, factors)


f.as <- sobol(model = NULL, X1.i, X2.i, order = 1, nboot = 0, conf = 0.95)
	



