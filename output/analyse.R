# Traiter les sorties de VLE pour permettre un debug rapide du modèle.

## Bibliotheques & chemins
library(gdata)
library(ggplot2)
library(lattice)
library(igraph)
library(Cairo)
## Fonctions de simulation
source("fonctions.R")

## Paramétrage R
HLIR = c("AreaHealthy","AreaLatent","AreaInfectious","AreaRemoved")
ObsTime = c(30, 60, 90, 120, 150)




##### Analyse et graphiques des sorties  modèle 1D #####
### Simulation
f <- rvle.open("1D_0.7.vpz", "archidemio")
sim.l<-rvle.sim(f, nExec=25, nVarNormal=2, nVarExec=13)

#### Graphiques
### Dynamiques des variables f(temps)
# Variables "culture"
#xyplot(value ~ time | variable, data=sim.l, scale="free", subset=scale=="crop", type="l")
c <- sim.l[sim.l$scale=="crop",]
crop <- ggplot(c, aes(time, value))
crop.gfx <- crop + geom_line() + facet_wrap(~ variable, scales="free", ncol=2) + theme_bw()


# Variables "unite"
#xyplot(value ~ time | variable, groups=unit, data=sim.l, subset=scale=="unit", scale="free", type="l")
#Seulement les variables HLIR
d <- drop.levels(sim.l[sim.l$variable %in% HLIR==T,])
d$variable <- factor(d$variable, levels=HLIR)
dynamique <- ggplot(d, aes(time, value, group=unit))
dynamique.gfx <- dynamique +
	geom_line() +
	facet_wrap(~ variable, scales="free", ncol=2) +
	theme_bw() + ylab("% Unit Area")


### Profils d'infections
# 5 date sur le cycle, en jours : TO DO, position relatives dans le cycle
p <- sim.l[(sim.l$variable %in% HLIR==T & sim.l$time %in% ObsTime==T),]
p$time <- as.factor(p$time)
p$unit <- as.numeric(p$unit)
profils <- ggplot(p, aes(unit, value, group=time))
profils.gfx <- profils + 
	geom_line(aes(colour=time)) +
	facet_wrap(~ variable, scales="free", ncol=2) +
	theme_bw() + coord_flip() + scale_colour_grey(start=0.8, end=0.2)

# Histogrammes cumulés
p <- sim.l[(sim.l$variable %in% HLIR==T & sim.l$time %in% ObsTime==T),]
p$time <- as.factor(p$time)
p$unit <- as.numeric(p$unit)
p <- drop.levels(p)
p$variable <- factor(p$variable, levels=HLIR)

col.hist <- c("darkgreen","orange","red","gray")
trellis.par.set(superpose.symbol=list(col=col.hist))
dotplot(unit ~ value | time, group=variable, data=p, auto.key=list(space="right"), type="l")

barchart(unit ~ value | time, group=variable, data=p, auto.key=list(space="right"))

# Profils de surface malade (%)
rd <-  sim.l[(sim.l$variable=="ScoreArea" & sim.l$time %in% ObsTime==T),]
rd$time <- as.factor(rd$time)
rd$unit <- as.numeric(rd$unit)

profil <- ggplot(rd, aes(unit, value, group=time))
profil.gfx <- profil + 
	geom_line(aes(colour=time)) +
	scale_x_continuous("Stem position") + 
	scale_y_continuous(limits=c(0, 1), "% Area removed by desease") +
	theme_bw() + coord_flip() + scale_colour_grey(start=0.8, end=0.2)



## Visualisation : deux types de vues : dynamique + profil de surface malade (%)
png(file="units.png", width=12, height=6, units="in", res=200, pointsize = 10)
Cairo(file="units.pdf", width = 12, height = 6, units="in", type="pdf", pointsize=10) 

grid.newpage()
pushViewport(viewport(layout = grid.layout(1, 2)))
vplayout <- function(x, y)
  viewport(layout.pos.row = x, layout.pos.col = y)
print(dynamique.gfx, vp = vplayout(1, 1))
print(profil.gfx, vp = vplayout(1, 2))
dev.off()

png(file="crop.png", width=6, height=6, units="in", res=200, pointsize = 10)
print(crop.gfx)
dev.off()


### Histogramme des classes de surface
h <- sim.l[(sim.l$variable %in% HLIR==T & sim.l$time %in% ObsTime==T),]
h <- drop.levels(h)
h$variable <- factor(h$variable, levels=rev(HLIR))

# Normalisation : somme ou LAI de la culture
h.sum <- aggregate(h$value, by=list(time=h$time), sum, na.rm=T)
colnames(h.sum) <- c("time","sum")
#h.lai <- sim.l[(sim.l$variable=="LAI" & sim.l$time %in% ObsTime==T),]
# Sommation sur les unités 
h <- aggregate(h$value, by=list(variable=h$variable, time=h$time), sum, na.rm=T)
h <- merge(h, h.sum)
h <- data.frame(h, value=(h$x/h$sum) * 100)

#barchart(variable ~ value | time, data=h, horiz=T)
ggplot(h, aes(variable, value)) + 
	geom_bar(stat="identity") + 
	facet_wrap(~ time, nrow=1) + 
	coord_flip() + 
	labs(y = "% LAI", x = "")



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
f <- rvle.open("2D_0.7.vpz", "archidemio")

rvle.setTranslator(f, condition="condParametres", class="Unit", n=100, init=3)

## Simulation (97s pour 20x20 (n=400))
#sim <- rvle.run(f)
system.time(sim.l<-rvle.sim(f, nExec=n, nVarNormal=2, nVarExec=10))

## Mise en forme des sortie : table 3D  {x, y, valeur de sortie}
# Sur le cycle
m.i <- aggregate(value ~ unit, data=sim.l, sum, subset=sim.l$variable=="AreaInfectious")
m.i <- data.frame(
	x = rep(sqrt(n):1,sqrt(n)),
	y = rep(1:sqrt(n), each=sqrt(n)),
	score = m.i$value
)
# Valeur finale
m.s <- aggregate(value ~ unit, data=sim.l, max, subset=sim.l$variable=="ScoreArea")
m.s <- data.frame(
	x = rep(sqrt(n):1,sqrt(n)),
	y = rep(1:sqrt(n), each=sqrt(n)),
	score = m.s$value
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
v <- ggplot(data=m.s, aes(x, y, z = score))
grid.s <- v + geom_tile(aes(fill = score)) +
	stat_contour(bins = sqrt(n)/2) +
	scale_fill_gradient(low="white", high="black") +
	scale_y_reverse() + opts(aspect.ratio = 1) + theme_bw()

Cairo(file="grid_final.pdf", width = 6, height = 6, units="in", type="pdf", pointsize=10) 	
png(file="grid_final.png", width=6, height=6, units="in", res=200, pointsize = 10)
print(grid.s)
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

### Patrak Super Object Oriented Framework
library(rvle)
f <- new("Rvle", file = "1D_0.7.vpz", pkg="archidemio")
f <- run(f)
sim.l <- rvle.shape(f)

			
### Analyse simple : un seul paramètre à la fois
f <- rvle.open("1D_0.7.vpz", "archidemio")

rvle.setRealCondition(f, "condParametres", "E_InitQuantity", 0.01)
rvle.setRealCondition(f, "condParametres", "E_InfectiousPeriod", 5)
# rvle.getAllConditionPortValues(f, "condParametres")

sim.l<-rvle.sim(f, nExec=25, nVarNormal=2, nVarExec=12)

trellis.par.set(
	superpose.line=list(col=topo.colors(25)), 
	superpose.symbol=list(col=topo.colors(25))
)

xyplot(value ~ time | variable, data=sim.l, 
	subset=scale=="crop", scale="free", type="l"
)

xyplot(value ~ time | variable, groups=unit, data=sim.l, 
	subset=scale=="unit", scale="free", type="l", col=topo.colors(25),
	auto.key=list(space="right")
)

xyplot(value ~ ThermalTime | variable, groups=unit, data=sim.l, 
	subset=scale=="unit", scale="free", type="l",
	auto.key=list(space="right")
)

# Une seule variable
xyplot(value ~ time, groups=unit, data=sim.l, 
	subset=variable=="AreaRemovedByDesease",
	scale="free", type="l", col=topo.colors(25),
	auto.key=list(space="right")
)

### Mise en forme sorties numériques
cast(sim.l, subset=sim.l$variable=="ThermalAge", time ~ unit)
cast(sim.l, subset=sim.l$variable=="RateAreaSenescence", time ~ unit)
cast(sim.l, subset=sim.l$variable=="InitQuantity", time ~ unit)
cast(sim.l, subset=sim.l$variable=="AreaHealthy", time ~ unit)


### Tests conservation de surface
H <- sim.l[sim.l$variable=="AreaHealthy",]
L <- sim.l[sim.l$variable=="AreaLatent",]
I <- sim.l[sim.l$variable=="AreaInfectious",]
A <- sim.l[sim.l$variable=="AreaActive",]
D <- sim.l[sim.l$variable=="AreaDeseased",]

# H + L + I = A
x <- H$value + L$value + I$value - A$value
x <- data.frame(H[,1:5], value= x)
plot(x$value)
na.omit(x[x$value > 0.01,])
na.omit(x[x$value < 0,])


xyplot(sim.l[sim.l$variable=="Receptivity","value"] ~ sim.l[sim.l$variable=="ThermalAge","value"])



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
