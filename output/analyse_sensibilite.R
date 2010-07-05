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
f <- rvle.open("archidemio_0.5a.vpz", "archidemio")
# Pour chaque paramètre, on fixe son intervalle d'incertitude, 
# efface la valeur nominale et assigne la gamme au vpz.
# rvle.listConditionPorts(f,"condParametres")
#f.factors.name <- rvle.listConditionPorts(f,"condParametres")

factors.name <- c("E_Init","E_InfectiousPeriod","E_RateDeseaseTransmission","E_RateAlloDeposition")

factors.bounds <- data.frame(
	name = factors.name,
	def = c(50, 10, 0.5, 0.2),
	min = c(10, 1, 0, 0),
	max = c(140, 15, 1, 1)
)

factors <- factors.name[1:4]
bounds <- factors.bounds[1:4,]


# Construire le plan
# f.plan <- expand.grid(va, vb)
f.as <- getPlanMorris(factors, binf=bounds$min, bsup=bounds$max, S=80, K=14)
f.plan <- f.as$X

# Mise en place dans VLE
rvle.addPlanCondition(f, "condParametres", factors=factors)
#rvle.setTotalCombination(f,1,1) 	# Complet
#regular.fraction() 				# Fractionné
rvle.setLinearCombination(f,1,1) 	# Linéaire
# On vérifie le plan dans le vpz.
rvle.getAllConditionPortValues(f, "condParametres")


## 2. Simulation
# La simulation depuis ce vpz permet de générer les vpz unitaires
# vle -m -l -o 2 -z -P archidemio plan.vpz
# rvle.save(f, "plan.vpz") 
system.time(d <- rvle.runManagerThread(f,2))


## 3. Indices de sensibilité
# Calcul de la quantité d'interet
# TODO : pas de boucles, un seule fonction
x <- lapply(d, melt, id=c("time","Top model,Crop:CropPhenology.ThermalTime"))
y=NULL
for (i in 1:length(x)) {
	y=c(y, sum(x[[i]]$value, na.rm=T))
}
# Sortie
f.out = data.frame(f.plan, y)
xyplot(data=x[[1]], value ~ time, groups=variable, type="l")
xyplot(data=f.as, y ~ 1:dim(f.plan)[1] , type="l")

# Decoupling
tell(f.as, f.out$y)
plot(f.as)









### Bazar
for (p in factors.name) {
	p.levels <- with(bounds, seq(bounds[name==p,"min"], bounds[name==p,"max"], length.out=10))
	f.plan <- data.frame(f.plan, p.levels)
}


	



