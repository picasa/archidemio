### Un script pour tester "facilement" la non-regression d'une nouvelle version du modèle

## Bibliotheques & chemins
library(gdata)
library(ggplot2)
library(lattice)
library(Cairo)

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


## Test FSA

# dates théoriques
development <- function(ttunits = c(70,70,70,50), nbunits = 25) {
	tt <- NULL
	for (u in 1:nbunits) {
		if (u <= length(ttunits)-1) t = u*ttunits[u]
		else t = sum(ttunits[1:length(ttunits)-1]) + (u-(length(ttunits)-1))*ttunits[4]
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

