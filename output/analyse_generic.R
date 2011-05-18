# Script utilisé pour l'utilisation du modèle appliqué au pathosystème Igname - Antracnose

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


##################   Exploration du modèle   ########################

### Simulation
# Nombre de noeuds (n=400 ds le vpz)
n=100 # sqrt(n) doit être entier

## couche classique (ok pour exploration)
f <- rvle.open("2D_0.8_NG.vpz", "archidemio")
rvle.setOutputPlugin(f, "vueSensitivity", "dummy")
rvle.setOutputPlugin(f, "vueDebug", "storage")

## Paramétrage de GraphTranslator (graphe de connection)
# graphe : grille 4 voisins 
A <- rvle.setTranslatorNG(f, condition="condParametres", class="Unit", n, init=1, type="lattice")
#rvle.setIntegerCondition(f, "condParametres", "E_OutDegree", 4)

# graphe construit depuis un voisinage (emission) défini dans une matrice (X)
X <- matrix(c(0,1, 0,-1, 1,0, -1,0), ncol=2, byrow=T) # 4 voisins
X <- matrix(c(1,0, 1,-1, 0,-1, -1,-1, -1,0, -1,1, 0,1, 1,1), ncol=2,byrow=TRUE) # 8 Voisins
A <- rvle.setTranslatorNG(f, condition="condParametres", class="Unit", n, init=1, type="custom", neighbour=X)
#rvle.save(f, "2D_0.8_IA.vpz")

# Simulation
system.time(sim <- rvle.run(f))
sim.l<-rvle.shape(sim, view="sensitivity")
sim.l<-rvle.shape(sim, view="debug")


## couche objet (ok pour optimisation)
f <- new("Rvle", file = "2D_0.8_NG.vpz", pkg = "archidemio")	

# Ce qu'il faut faire si veut modifier les condition de simulation
# TODO Cf. Bug patrak pour les condition Tuples
run(f, 
	condParametres.E_GridMatrix = as.vector(A),
	condParametres.E_GridNumber = n,
	condParametres.E_InitSpace = 10
)


# Simulation ScoreArea (11s pour 20x20)
#system.time(sim.l<-rvle.sim(f, nExec=n, nVarNormal=2, nVarExec=12, view="debug")) # ! si n > 400
system.time(f<-run(f))
rvle.shape(f, nExec=n, index="time", view="sensitivity")

system.time(sim.l<-rvle.sim(f, nExec=n, nVarExec=1, index="time", view="sensitivity"))



### Graphes
## Variables d'états "culture"
c <- sim.l[sim.l$scale=="crop",]
crop <- ggplot(c, aes(time, value))
crop.gfx <- crop + geom_line() + facet_wrap(~ variable, scales="free", ncol=2) + theme_bw()

## Variables d'états "unité"
trellis.par.set(canonical.theme(color = FALSE))
xyplot(value ~ time | variable, groups=unit, data=sim.l, subset=scale=="unit", scale="free", type="l", alpha=0.05, lty=1)


## Progression de la maladie dans le temps
dpc <- ggplot(data=aggregate(value ~ time, data=sim.l, mean, subset=sim.l$variable=="ScoreArea"), aes(x=time, y=value))
dpc.gfx <- dpc + geom_line() + theme_bw() + ylab("Mean diseased area")

## Evolution du partitionnement du type de surface malade : histogrammes + densité
h <- sim.l[(sim.l$variable %in% HLIR==T),]
h <- drop.levels(h)
ggplot(h, aes(time, weight=value, fill=variable)) + geom_bar(binwidth=10, position="fill") + theme_bw() 
ggplot(h, aes(time, ..density.., weight=value, colour=variable)) + geom_freqpoly(aes(group=variable), binwidth=1) + theme_bw()


## Cartographie maladie : Mise en forme des sortie : table 3D  {x, y, valeur de sortie}
# Valeur finale (utiliser vue "finish" si l'on a besoin que de cette variable)
m.s <- aggregate(value ~ unit, data=sim.l, max, subset=sim.l$variable=="ScoreArea")
m.s <- data.frame(
	expand.grid(x=1:sqrt(n), y=1:sqrt(n)),
	score = m.s$value
)
# Visualisation : matrices + contour
v <- ggplot(data=m.s, aes(x, y, z = score))
grid.s <- v + geom_tile(aes(fill = score)) +
	stat_contour(bins = sqrt(n)/10) +
	scale_fill_gradient(low="white", high="black") +
	scale_y_reverse() + opts(aspect.ratio = 1) + theme_bw()
	
	



############# Impact du paramétrage de voisinage   #####################

n=400 # sqrt(n) doit être entier

# 1. 4 voisins réguliers
f <- rvle.open("2D_0.8_IA.vpz", "archidemio")
system.time(sim <- rvle.run(f))
sim.l<-rvle.shape(sim, view="sensitivity")

v1 <- aggregate(value ~ unit, data=sim.l, max, subset=sim.l$variable=="ScoreArea")
v1 <- data.frame(
	expand.grid(x=1:sqrt(n), y=1:sqrt(n)),
	score = v1$value
)

# 2. 8 voisins réguliers
f <- rvle.open("2D_0.8_IA.vpz", "archidemio")
X <- matrix(c(1,0, 1,-1, 0,-1, -1,-1, -1,0, -1,1, 0,1, 1,1), ncol=2,byrow=TRUE) # 8 Voisins
A <- rvle.setTranslator(f, condition="condParametres", class="Unit", n, init=n/100, type="custom", neighbour=X)

system.time(sim <- rvle.run(f))
sim.l<-rvle.shape(sim, view="sensitivity")

v2 <- aggregate(value ~ unit, data=sim.l, max, subset=sim.l$variable=="ScoreArea")
v2 <- data.frame(
	expand.grid(x=1:sqrt(n), y=1:sqrt(n)),
	score = v2$value
)

# 3. 4 voisins, effet rang
f <- rvle.open("2D_0.8_IA.vpz", "archidemio")
X <- matrix(c(0,-1, 0,-2, 0,1, 0,2), ncol=2,byrow=TRUE) # 8 Voisins
A <- rvle.setTranslator(f, condition="condParametres", class="Unit", n, init=n/100, type="custom", neighbour=X)

system.time(sim <- rvle.run(f))
sim.l<-rvle.shape(sim, view="sensitivity")

v3 <- aggregate(value ~ unit, data=sim.l, max, subset=sim.l$variable=="ScoreArea")
v3 <- data.frame(
	expand.grid(x=1:sqrt(n), y=1:sqrt(n)),
	score = v3$value
)
# 4. 8 voisins, effet rang
f <- rvle.open("2D_0.8_IA.vpz", "archidemio")
X <- matrix(c(1,0, 0,-1, 0,-2, 0,-3, -1,0, 0,1, 0,2, 0,3), ncol=2,byrow=TRUE) # 8 Voisins
A <- rvle.setTranslator(f, condition="condParametres", class="Unit", n, init=n/100, type="custom", neighbour=X)

system.time(sim <- rvle.run(f))
sim.l<-rvle.shape(sim, view="sensitivity")

v4 <- aggregate(value ~ unit, data=sim.l, max, subset=sim.l$variable=="ScoreArea")
v4 <- data.frame(
	expand.grid(x=1:sqrt(n), y=1:sqrt(n)),
	score = v4$value
)


# Graphe
d <- rbind(
	data.frame(v1, graph="4, lattice"),
	data.frame(v2, graph="8, lattice"),
	data.frame(v3, graph="4, rangs"),
	data.frame(v4, graph="8, rangs")
)


v <- ggplot(data=d, aes(x, y, z = score))
v.gfx <- v + geom_tile(aes(fill = score)) +
	facet_wrap(~ graph, nrow=1) +
	stat_contour(bins = sqrt(n)/10) +
	scale_fill_gradient(low="white", high="black") +
	scale_y_reverse() + opts(aspect.ratio = 1) + theme_bw()

Cairo(file="neighbours.pdf", width = 16, height = 5, units="in", type="pdf", pointsize=10)
print(v.gfx)
dev.off()


