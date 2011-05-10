### Un script pour tester "facilement" la non-regression d'une nouvelle version du modèle

## Bibliotheques & chemins
library(gdata)
library(ggplot2)
library(lattice)
library(Cairo)
library(igraph)

## Fonctions de simulation
source("fonctions.R")

## Version Initiale
f <- new("Rvle", file = "1D_0.8.vpz", pkg = "archidemio")	# classe
old<-rvle.sim(f, nExec=25, nVarNormal=2, nVarExec=12)

## Modifications du code
# Faire des sauvegardes à la main ?

## Version modifiée
f <- new("Rvle", file = "1D_0.8.vpz", pkg = "archidemio")	# classe
new<-rvle.sim(f, nExec=25, nVarNormal=2, nVarExec=12)

## Résultats
# Graphique
xyplot(new$value ~ old$value, pch=".")

# Ecart moyen et RMSE
mean(old$value - new$value, na.rm=T)
rmse(old$value, new$value)
max(old$value - new$value, na.rm=T)


########################################################################
## Test FSA
# dates théoriques
development <- function(ttunits = c(70,70,70,50), nbunits = 25) {
	tt <- NULL
	for (u in 1:nbunits) {
		if (u <= length(ttunits)-1) t = u*ttunits[u]
		else t = sum(ttunits[1:length(ttunits)-1]) + (u-(length(ttunits)-1))*ttunits[length(ttunits)]
		tt <- c(tt, t) 
	}
	return(tt)
}

f <- new("Rvle", file = "1D_0.8.vpz", pkg = "archidemio")	# classe

# Combien d'unités
f<-run(f)
colnames(f@outlist[[1]])[length(colnames(f@outlist[[1]]))]

n = 28
sim.l<-rvle.sim(f, nExec=n, nVarNormal=2, nVarExec=12)
d <- sim.l[(sim.l$variable=="ThermalAge" & sim.l$value==0 & !is.na(sim.l$value)),]
d <- cbind(d, ThThermalTime = development(nbunits=n))

d$ThermalTime - d$ThThermalTime



########################################################################
## Test diffusion, quelle opération sur les événements entrants ?

f <- rvle.open("2D_0.8.vpz", "archidemio")
rvle.setOutputPlugin(f, "vueSensitivity", "dummy")
rvle.setOutputPlugin(f, "vueDebug", "storage")

# Condition de simulation
n=9 # sqrt(n) doit être entier
M0 <- matrix(rep(0,9^2), ncol=9, byrow=TRUE)
M2 <- M0; M2[5,2]=1; M2[5,8]=1
M4 <- M0; M4[5,2]=1; M4[5,8]=1; M4[5,4]=1; M4[5,6]=1

rvle.setIntegerCondition(f, condition="condParametres", "E_GridNumber", n)
rvle.setStringCondition(f, condition="condParametres", "E_GridClasses", paste(rep("Unit",n), collapse=" "))
rvle.setStringCondition(f, condition="condParametres", "E_GridMatrix", paste(as.vector(M4), collapse=" "))
rvle.setTupleCondition(f, condition="condParametres", "E_InitSpace", c(1,3,5,7))

sim <- rvle.run(f)
sim.l<-rvle.shape(sim, view="debug")

# Graphe
m.s <- aggregate(value ~ unit, data=sim.l, max, subset=sim.l$variable=="ScoreArea")
m.s <- data.frame(
	expand.grid(x=1:sqrt(n), y=1:sqrt(n)),
	score = m.s$value
)

v <- ggplot(data=m.s, aes(x, y, z = score))
grid.s <- v + geom_tile(aes(fill = score)) +
	scale_fill_gradient() +
	scale_y_reverse() + opts(aspect.ratio = 1) + theme_bw()

grid.s

# Reception
xyplot(value ~ time | unit, data=sim.l, subset=sim.l$variable=="InDeposition", type="l")
# Maladie
xyplot(value ~ time | unit, data=sim.l, subset=sim.l$variable=="AreaLatent", type="l")


########################################################################
## Test chargement de modèles

# Paramétrage R
HLIR = c("AreaHealthy","AreaLatent","AreaInfectious","AreaRemoved")
ObsTime = c(30, 60, 90, 120, 150)

# Simulation
f <- new("Rvle", file = "2D_0.8.vpz", pkg = "archidemio")	# classe
n=100 # sqrt(n) doit être entier
system.time(sim.l<-rvle.sim(f, nExec=n, nVarExec=1, index="time", view="sensitivity"))

# Graphe
m.s <- aggregate(value ~ unit, data=sim.l, max, subset=sim.l$variable=="ScoreArea")
m.s <- data.frame(
	expand.grid(x=1:sqrt(n), y=1:sqrt(n)),
	score = m.s$value
)

v <- ggplot(data=m.s, aes(x, y, z = score))
grid.s <- v + geom_tile(aes(fill = score)) +
	stat_contour(bins = sqrt(n)/10) +
	scale_fill_gradient(low="white", high="black") +
	scale_y_reverse() + opts(aspect.ratio = 1) + theme_bw()

grid.s


