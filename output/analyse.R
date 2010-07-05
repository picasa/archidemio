# Traiter les sorties de VLE pour permettre un debug rapide du modèle.

## Bibliotheques & chemins
#library(gdata)
library(ggplot2)
library(lattice)
library(igraph)

## Fonctions de simulation
source("fonctions.R")

## Paramétrage R
HLIR = c("AreaHealthy","AreaLatent","AreaInfectious","AreaRemoved")
ObsTime = c(30, 60, 90, 120, 150)


#### Analyse et graphiques des sorties  modèle 1D ####
## Simulation
f <- rvle.open("archidemio_0.5.vpz", "archidemio")
sim.l<-rvle.sim(f, nExec=25, nVarNormal=2, nVarExec=9)

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
profils.gfx <- profils + 
	geom_line(aes(colour=time)) +
	facet_wrap(~ variable, scales="free", ncol=2) +
	theme_bw() + coord_flip() + scale_colour_grey()


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





#### Analyse et graphiques des sorties  modèle 2D ####

## Generer la matrice d'adjacence
n = 400
G <- graph.lattice(c(sqrt(n),sqrt(n)))
M <- get.adjacency(G)
rownames(M)<-c(1:n)-1
colnames(M)<-c(1:n)-1

## Passer ces conditions dans VLE
f <- rvle.open("archidemio_0.6a.vpz", "archidemio")

rvle.setIntegerCondition(f, "condParametres", "E_GridNumber", n)
rvle.setStringCondition(f, "condParametres", "E_GridClasses", paste(rep("Unit",n), collapse=" "))
rvle.setStringCondition(f, "condParametres", "E_GridMatrix", paste(as.vector(M), collapse=" "))

rvle.setTupleCondition(f, "condParametres", "E_InitSpace", c(5,255,300))

## Simulation
#sim <- rvle.run(f)
system.time(sim.l<-rvle.sim(f, nExec=n, nVarNormal=2, nVarExec=9))

## Agrégation des sorties
# Sur le cycle
m.i <- aggregate(value ~ unit, data=sim.l, sum, subset=sim.l$variable=="AreaInfectious")
m.i <- data.frame(
	x = rep(sqrt(n):1,sqrt(n)),
	y = rep(1:sqrt(n), each=sqrt(n)),
	score = m.i$value
)
# intégration à différents pas de temps
m.t <- NULL
for (t in ObsTime) {
	d <- sim.l[(sim.l$variable=="AreaInfectious" & sim.l$time <= t ),]
	tmp <- data.frame(
		time = as.factor(t),
		x = rep(sqrt(n):1,sqrt(n)),
		y = rep(1:sqrt(n), each=sqrt(n)),
		score = aggregate(value ~ unit, data=d, sum)$value
	)
	m.t <- rbind(m.t,tmp)
}
# instantanés à différents pas de temps 
tmp <- sim.l[(sim.l$variable=="AreaInfectious"& sim.l$time %in% ObsTime==T),]
m.s <- data.frame(
	time = tmp$time,
	x = rep(rep(sqrt(n):1,sqrt(n)), each=length(ObsTime)),
	y = rep(rep(1:sqrt(n), each=sqrt(n)), each=length(ObsTime)),
	score = tmp$value

)

## Visualisation : matrices + contour
# intégration
v <- ggplot(data=m.i, aes(x, y, z = score))
grid.i <- v + geom_tile(aes(fill = score)) +
	stat_contour(bins = sqrt(n)/2) +
	scale_fill_gradient(low="white", high="black") +
	scale_y_reverse() + opts(aspect.ratio = 1)
	
png(file="grid_final.png", width=6, height=6, units="in", res=200, pointsize = 10)
print(grid.i)
dev.off()

# evolution & intégration
v <- ggplot(data=m.t, aes(x, y, z = score))
grid.t <- v + geom_tile(aes(fill = score)) +
	facet_wrap(~ time, nrow=1) +
	scale_fill_gradient(low="white", high="black") +
	scale_y_reverse() + opts(aspect.ratio = 1)
	
png(file="grid_integration.png", width=16, height=4, units="in", res=200, pointsize = 10)
print(grid.t)
dev.off()
	
# evolution
v <- ggplot(data=m.s, aes(x, y, z = score))
grid.s <- v + geom_tile(aes(fill = score)) +
	facet_wrap(~ time, nrow=1) +
	scale_fill_gradient(low="white", high="black") +
	scale_y_reverse() + opts(aspect.ratio = 1)
	
png(file="grid_snapshot.png", width=16, height=4, units="in", res=200, pointsize = 10)
print(grid.s)
dev.off()


## Visualisation : dynamiques
#xyplot(value ~ time | variable, groups=unit, data=sim.l,
#	subset=scale=="unit",
#	scale="free", type="l", auto.key=list(space="right")
#)















#### DEBUG ####

### Analyse simple : un seul paramètre à la fois
f <- rvle.open("archidemio_0.5.vpz", "archidemio")
rvle.setRealCondition(f, "condParametres", "E_RateAlloDeposition", 0.3)
sim.l <- rvle.sim()
xyplot(value ~ time | variable, groups=unit, data=sim.l, subset=scale=="unit", scale="free", type="l")

### Mise en forme sorties numériques
cast(sim.l, subset=sim.l$variable=="AreaHealthy", time ~ unit)


### Bazar
## Import
# strsplit(colnames(sim), "\\.")
# sim <- read.table("exp_vueDebug.csv", sep=";", dec=".", header=T, colClasses="numeric") 

# contour
# m <- melt(volcano)
# names(m) <- c("x", "y", "z")
# v <- ggplot(data=m, aes(x, y, z = z)) 
# v + stat_contour()
# v + geom_tile(aes(fill = z)) + stat_contour() + scale_fill_gradient(low="black", high="white")
