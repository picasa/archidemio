# Traiter les sorties de VLE pour permettre un debug rapide du modèle.

## Bibliotheques & chemins
#library(gdata)
library(ggplot2)
library(lattice)
library(reshape)
library(rvle)


## Paramétrage R
HLIR = c("AreaHealthy","AreaLatent","AreaInfectious","AreaRemoved")
ObsTime = c(30, 60, 90, 120, 150)

### Fonction pour recuperer les données simulées
## Simulation
# setOutputPlugin() : changer le type de la vue
f <- rvle.open("archidemio_0.5.vpz", "archidemio")

rvle.sim <- function (
	SimLength = 150,
	nVarNormal = 3, # variables non Executive (sans les index de temps)
	nVarExec = 9,
	nExec = 24
	) {
	sim <- rvle.run(f)
	sim <- as.data.frame(sim)
	# remplacer le code de date pour time
	sim$time <- 1:length(sim$time)
	# passage au format "long" : index = time & ThermalTime
	m <- melt(sim, id=c("time","Top.model.Crop.CropPhenology.ThermalTime")) 
	# extraction du numéro d'unité : A FAIRE, en fonction du nom de colonne
	# Construction des index manquants : numero d'unité et type de variable (culture / unité)
	unit <- c(rep(rep(NA, each=SimLength), each=nVarNormal), rep(rep(1:nExec, each=SimLength), each=nVarExec)) 
	scale <- c(rep("crop",nVarNormal*SimLength), rep("unit", SimLength*nVarExec*nExec))
	# Tout rassembler dans un dataframe
	d <- data.frame(
		time=m$time,
		ThermalTime=m$Top.model.Crop.CropPhenology.ThermalTime, 
		scale=as.factor(scale), 
		variable=sub(".*\\.","", m$variable), 
		unit=as.factor(unit), 
		value=m$value
	)
	return(d)
}

sim.l <- rvle.sim()

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


### Analyse simple : un seul paramètre à la fois
f <- rvle.open("archidemio_0.5.vpz", "archidemio")
rvle.setRealCondition(f, "condParametres", "E_LatentPeriod", 30)
sim.l <- rvle.sim()
xyplot(value ~ time | variable, groups=unit, data=sim.l, subset=scale=="unit", scale="free", type="l")


### Mise en forme sorties numériques
cast(sim.l, subset=sim.l$variable=="AreaHealthy", time ~ unit)

### Bazar
## Import
# strsplit(colnames(sim), "\\.")
# sim <- read.table("exp_vueDebug.csv", sep=";", dec=".", header=T, colClasses="numeric") 
