
########################################################################
## Bibliotheques & chemins
library(gdata)
library(ggplot2)
library(lattice)
library(Cairo)

setwd("~/Documents/Travail/2009_Archidemio/Modele/archidemio/output")
source("archidemio_fonctions.R")

## Fonctions pour la simulation
# Simulation
f <- new("Rvle", file = "1D_0.9.vpz", pkg = "archidemio")
f <- run(f)
# Mise en forme de données
sim.l<-rvle.shape(f, nExec=28, nVarNormal=2, nVarExec=12)

## Fonctions pour la représentation
# Paramétrage R
HLIR = c("AreaHealthy","AreaLatent","AreaInfectious","AreaRemoved")
ObsTime = c(30, 60, 90, 120, 150)

# Variables "unite"
xyplot(value ~ time | variable, groups=unit, data=sim.l, subset=scale=="unit", scale="free", type="l")

# Histogramme des classes de surface
h <- sim.l[(sim.l$variable %in% HLIR==T),]
h <- drop.levels(h)
h$variable <- factor(h$variable, levels=rev(HLIR))

ggplot(h, aes(time, weight=value, fill=variable)) + geom_bar(binwidth=1, position="fill") + theme_bw()
