# Traiter les sorties de VLE pour permettre un debug rapide du modèle.

## Bibliotheques & chemins
#library(gdata)
library(ggplot2)
library(lattice)

## Fonctions de simulation
source("fonctions.R")


#### Analyse et graphiques des sorties ####

## Paramétrage R
SimLength = 150
HLIR = c("AreaHealthy","AreaLatent","AreaInfectious","AreaRemoved")
ObsTime = c(30, 60, 90, 120, 150)

## Simulation
f <- rvle.open("archidemio_0.5.vpz", "archidemio")
sim.l <- rvle.sim(model=f)

### Graphiques
## Dynamiques des variables f(temps)
# Variables "culture"
#xyplot(value ~ time | variable, data=sim.l, scale="free", subset=scale=="crop", type="l")
c <- sim.l[sim.l$scale=="crop",]
crop <- ggplot(c, aes(time, value))
crop.gfx <- crop + geom_line() + facet_wrap(~ variable, scales="free", ncol=2) + theme_bw()


# Variables "unite"
#xyplot(value ~ time | variable, groups=unit, data=sim.l, subset=scale=="unit", scale="free", type="l")
#Seulement les variables HLIR
d <- sim.l[sim.l$variable %in% HLIR==T,]
dynamique <- ggplot(d, aes(time, value, group=unit))
dynamique.gfx <- dynamique + geom_line() + facet_wrap(~ variable, scales="free", ncol=2) + theme_bw()


## Profils d'infections
# 5 date sur le cycle, en jours : A FAIRE, position relatives dans le cycle
p <- sim.l[(sim.l$variable %in% HLIR==T & sim.l$time %in% ObsTime==T),]
p$time <- as.factor(p$time)
p$unit <- as.numeric(p$unit)
profils <- ggplot(p, aes(unit, value, group=time))
profils.gfx <- profils + geom_line(aes(colour=time)) + facet_wrap(~ variable, scales="free", ncol=2) + theme_bw() + coord_flip() + scale_colour_grey()


## Visualisation : les deux type de vues dynamique + profils
png(file="units.png", width=12, height=6, units="in", res=200, pointsize = 10)
grid.newpage()
pushViewport(viewport(layout = grid.layout(1, 2)))
vplayout <- function(x, y)
  viewport(layout.pos.row = x, layout.pos.col = y)
print(dynamique.gfx, vp = vplayout(1, 1))
print(profils.gfx, vp = vplayout(1, 2))
dev.off()

png(file="crop.png", width=6, height=6, units="in", res=200, pointsize = 10)
print(crop.gfx)
dev.off()


### Cinétiques de progression de la maladie
# TODO : ggplot ne gère pas les NA ?, soit résoudre, soit commencer plus tard.
# Données
sim.l <- rvle.sim()
sim.l$unit <- as.numeric(sim.l$unit)
sim.l$variable <- factor (sim.l$variable, levels=HLIR)
for (i in seq(5,SimLength, by=5)) {
	# Selection
	p <- sim.l[(sim.l$variable %in% HLIR==T & sim.l$time==i),]
	
	# Graphique
	png(file=paste("profil_",i,".png", sep=""), width=8, height=3, units="in", res=100, pointsize = 10)
	trellis.par.set(canonical.theme(color = FALSE))
	print(xyplot(unit ~ value | variable, data=p, type="l", xlim=c(-0.1,1.1), ylim=c(-1,26), layout=c(4,1)))
	dev.off()	
}




#### DEBUG ####

### Analyse simple : un seul paramètre à la fois
f <- rvle.open("archidemio_0.5.vpz", "archidemio")
rvle.setRealCondition(f, "condParametres", "E_RateAlloDeposition", 0.3)
sim.l <- rvle.sim()
xyplot(value ~ time | variable, groups=unit, data=sim.l, subset=scale=="unit", scale="free", type="l")

### Mise en forme sorties numériques
cast(sim.l, subset=sim.l$variable=="AreaHealthy", time ~ unit)



### Debug sur modèle spatial 2D
f <- rvle.open("archidemio_0.6.vpz", "archidemio")
sim.l<-rvle.sim(nExec=6, nVarNormal=1, nVarExec=8)
xyplot(value ~ time | variable, groups=unit, data=sim.l, subset=scale=="unit", scale="free", type="l")




### Bazar
## Import
# strsplit(colnames(sim), "\\.")
# sim <- read.table("exp_vueDebug.csv", sep=";", dec=".", header=T, colClasses="numeric") 

