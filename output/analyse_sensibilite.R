### Regroupe les méthodes pour réaliser une analyse de sensibilité du modèle 

### Bibliothèques & chemins
#library(gdata)
library(ggplot2)
library(lattice)
library(sensitivity)
library(lhs)
library(rgl)
library(igraph)
library(Cairo)

### Fonctions de simulation & Paramétrage
source("fonctions.R")
#rvle.getDuration(f)

### Sensibilité : plutôt utiliser le gestionnaire de simulation de VLE
### puis calculer des indices depuis les sorties.

## 1. Plan d'expérience géré par VLE
# Pour chaque paramètre, on fixe son intervalle d'incertitude, 
# efface la valeur nominale et assigne la gamme au vpz.
# rvle.listConditionPorts(f,"condParametres")
#f.factors.name <- rvle.listConditionPorts(f,"condParametres")

factors.name <- c(
	"E_InitTime",
	"E_InfectiousPeriod",
	"E_RateDeseaseTransmission",
	"E_RateAlloDeposition",
	"E_InitQuantity",
	"E_LatentPeriod",
	"P_UnitTTSen"
)

factors.bounds <- data.frame(
	name = factors.name,
	def = c(50, 10, 0.5, 0.2, 0.01, 5, 1000),
	min = c(30, 1, 0.05, 0.05, 0.05, 1, 700),
	max = c(140, 15, 1, 1, 0.3, 15, 1300)
)

factors <- factors.name[1:7]
bounds <- factors.bounds[1:7,]


# Construire le plan
# plan complet : f.plan <- expand.grid(va, vb)
f.as <- getPlanMorris(factors, binf=bounds$min, bsup=bounds$max, S=200, K=6)
f.as <- getPlanFast(factors, bounds, n=300)
f.as <- getPlanSobol(factors, bounds, n=300)
f.plan <- getPlanLHS(factors, bounds, n=1000)

# Mise en place dans VLE
f <- rvle.open("1D_0.7.vpz", "archidemio")

rvle.addPlanCondition(f, "condParametres", plan=f.as$X, factors=factors)
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
# Calcul de la quantité d'interet : Intégration : somme sur t & unité
f.out <- compute.output(plan=f.as$X, data=d)

# Decoupling
tell(f.as, f.out$y)


# Graphiques
# Images du plan d'échantillonage
plan <- as.data.frame(f.as$X)
apply(plan, 2, table)
splom(plan)
parallel(~ plan, data=plan, alpha=0.05, lty=1, col="black")

plan.l <- stack(as.data.frame(f.as$X))
bwplot(~ values | ind, data=plan.l, scale="free")

g <- ggplot(plan.l, aes(x=values))
g + geom_histogram() + 
	facet_wrap(~ ind, scales="free", ncol=2) +
	theme_bw()

# Sorties Brutes
xyplot(data=x[[1]], value ~ time, groups=variable, type="l")
xyplot(data=f.out, y ~ 1:dim(f.out)[1] , type="l")
histogram(~ y, data=f.out, type="count")

# Indices
plot(f.as)

plot.morris(f.as, factors)
plot.fast(f.as, factors)
plot.sobol(f.as.sobol, factors)




### Comparaison des méthodes
## Facteurs d'incertitude
factors.name <- c(
	"E_InitTime",
	"E_InfectiousPeriod",
	"E_RateDeseaseTransmission",
	"E_RateAlloDeposition",
	"E_InitQuantity",
	"E_LatentPeriod",
	"P_UnitTTSen"
)

factors.bounds <- data.frame(
	name = factors.name,
	def = c(50, 10, 0.5, 0.2, 0.01, 5, 1000),
	min = c(30, 1, 0.05, 0.05, 0.05, 1, 700),
	max = c(140, 15, 1, 1, 0.3, 15, 1300)
)

factors <- factors.name[1:7]
bounds <- factors.bounds[1:7,]

## Morris
f.morris <- rvle.open("1D_0.7.vpz", "archidemio")

f.as.morris <- getPlanMorris(factors, binf=bounds$min, bsup=bounds$max, S=200, K=6)
rvle.addPlanCondition(f.morris, "condParametres", plan=f.as.morris$X, factors=factors)
rvle.setLinearCombination(f.morris,1,1) 

system.time(d.morris <- rvle.runManagerThread(f.morris,2))
f.out.morris <- compute.output(plan=f.as.morris$X, data=d.morris)

tell(f.as.morris, f.out.morris$y)
plot.morris(f.as.morris)


## FAST	
f.fast <- rvle.open("1D_0.7.vpz", "archidemio")

f.as.fast <- getPlanFast(factors, bounds, n=200)
rvle.addPlanCondition(f.fast, "condParametres", plan=f.as.fast$X, factors=factors)
rvle.setLinearCombination(f.fast,1,1) 

system.time(d.fast <- rvle.runManagerThread(f.fast,2))
f.out.fast <- compute.output(plan=f.as.fast$X, data=d.fast)

tell(f.as.fast, f.out.fast$y)
plot.fast(f.as.fast)

## Sobol
f.sobol <- rvle.open("1D_0.7.vpz", "archidemio")

f.as.sobol <- getPlanSobol(factors, bounds, n=300)
rvle.addPlanCondition(f.sobol, "condParametres", plan=f.as.sobol$X, factors=factors)
rvle.setLinearCombination(f.sobol,1,1) 

system.time(d.sobol <- rvle.runManagerThread(f.sobol,2))
f.out.sobol <- compute.output(plan=f.as.sobol$X, data=d.sobol)

tell(f.as.sobol, f.out.sobol$y)
plot.sobol(f.as.sobol)

## Graphique comparatif
# plans
plans <- rbind(
	data.frame(f.as.morris$X, type="morris"),
	data.frame(f.as.fast$X, type="fast"),
	data.frame(f.as.sobol$X, type="sobol"),
	data.frame(getPlanLHS(factors, bounds, n=2000), type="lhs")
)
png(file="plans.png", width=12, height=6, units="in", res=200, pointsize = 10)
trellis.par.set(canonical.theme(color = FALSE))
parallel(~ plans[1:7] | type, data=plans, alpha=0.05, lty=1, col="black", layout=c(4,1))


# sorties
index <- rbind(
	plot.morris(f.as.morris, gfx=F),
	plot.fast(f.as.fast, gfx=F),
	plot.sobol(f.as.sobol, gfx=F)	
)

png(file="sensitivity.png", width=6, height=12, units="in", res=200, pointsize = 10)
trellis.par.set(canonical.theme(color = FALSE))
dotplot(labels ~ first + total | type, data=index, auto.key=list(space="bottom"), 
	xlab="Sensitivity indexes", scales="free", layout=c(1,3)
)	
dev.off()


## LHS
f <- rvle.open("1D_0.7.vpz", "archidemio")

f.plan <- getPlanLHS(factors, bounds, n=1000)
rvle.addPlanCondition(f, "condParametres", plan=f.plan, factors=factors)
rvle.setLinearCombination(f,1,1)

system.time(d <- rvle.runManagerThread(f,2))
f.out.lhs <- compute.output(plan=f.plan, d.raw=d)

# Plan & Surface de réponse
splom(f.plan, pch=".")

f.plan.l <- stack(as.data.frame(f.plan))
bwplot(~ values | ind, data=f.plan.l, scale="free")

g <- ggplot(f.plan.l, aes(x=values))
g + geom_histogram() + 
	facet_wrap(~ ind, scales="free", ncol=2) +
	theme_bw()

histogram(~ y, data=f.out.lhs, type="count")
with(f.out, perspPlus(E_InitTime, E_InitQuantity, y, nomx="InitTime", nomy="InitQuantity",nomz="y"))

parallel(~ f.out[c(8,1,4,3,2)], data=f.out, alpha=0.05, lty=1, col="black")

# Metamodèle
f.m = with(f.out, lm( y ~ polym(E_InitTime, E_RateAlloDeposition, E_InitQuantity, degree=3)))
summary(f.m)
anova(f.m)






### Analyse de sensibilité sur modèle 2D
f <- rvle.open("2D_0.7.vpz", "archidemio")
rvle.setTranslator(f, condition="condParametres", class="Unit", n=100, init=3)

f.as <- getPlanFast(factors, bounds, n=100)
rvle.addPlanCondition(f, condition="condParametres", plan=f.as$X, factors=factors)
rvle.setLinearCombination(f,1,1) 

# ~ 14 min pour 700 simulations de 100 modèles (0.8 sim/s)
system.time(d <- rvle.runManagerThread(f,2))
f.out <- compute.output(plan=f.as$X, data=d, type="2D")

tell(f.as, f.out$y)




### Comparaison de la sensibilité des modèle 1D/2D
index <- rbind(
	cbind(type="1D", plot.fast(f.as.1D, gfx=F)[,2:4]),
	cbind(type="2D", plot.fast(f.as.2D, gfx=F)[,2:4])
)

Cairo(file="sensitivity.pdf", width = 12, height = 6, units="in", type="pdf", pointsize=10) 
trellis.par.set(canonical.theme(color = FALSE))
dotplot(labels ~ first + total | type, data=index, auto.key=list(space="bottom"), 
	xlab="Sensitivity indexes", scales="free", layout=c(2,1)
)	
dev.off()



