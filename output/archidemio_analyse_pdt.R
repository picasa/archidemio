# Script utilisé pour l'utilisation du modèle appliqué au pathosystème Pomme de terre - Mildiou

## Bibliotheques & chemins
library(gdata)
library(ggplot2)
library(lattice)
library(igraph)
library(Cairo)
## Fonctions de simulation
source("archidemio_fonctions.R")


## Modèle et paramétrage
f <- new("Rvle", file = "potato.vpz", pkg = "archidemio")	

## Simulation et mise en forme des résultats 
d <- results(run(f))
d <- rvle.shape(d, view="sensitivity", nExec=100)

## Visualisation 1D
# cinétique de l'épidémie à la parcelle (Da : pente de la courbe de progression de la maladie)
dm <- aggregate(value ~ time, data=d, mean)
ggplot(data=dm, aes(x=time, y=value)) + geom_line() + theme_bw() + ylab("Mean diseased area")
# cinétiques des unités
ggplot(data=d, aes(x=time, y=value, group=unit)) + geom_line(size=0.1, alpha=0.5) + theme_bw()

## Visualisation 2D : Cartographie maladie 
d <- results(run(f))
d <- rvle.shape.grid(d, nExec=100)
# Visualisation : matrices + contour
ggplot(data=dg, aes(x, y, z = value)) +
	geom_tile(aes(fill = value)) +
	stat_contour(bins = sqrt(n)/10) +
	scale_fill_gradient(low="white", high="black") +
	scale_y_reverse() + opts(aspect.ratio = 1) + theme_bw()
	
	
## Multi-simulation
# distribution des AUDPC : bimodalité selon résistance totale (R) et partielle (s)

## Impact du paramétrage de voisinage
# Voisinages unitaires
X1 <- matrix(c(1,0, 1,-1, 0,-1, -1,-1, -1,0, -1,1, 0,1, 1,1), ncol=2,byrow=TRUE) # 8 voisins, réguliers
X2 <- matrix(c(1,0, 0,-1, 0,-2, 0,-3, -1,0, 0,1, 0,2, 0,3), ncol=2,byrow=TRUE) # 8 voisins, rangs
X3 = matrix(c(0,1, 0,-1, 0,-2, -1,0, -1,-1 ,1,0 ,1,-1 , 3,-4),ncol=2,byrow=TRUE) # 8 voisins, orienté

network <- list(
	N1 = getAdjacency(n = 2500, type="custom", neighbour=X1),
	N2 = getAdjacency(n = 2500, type="custom", neighbour=X2),
	N3 = getAdjacency(n = 2500, type="custom", neighbour=X3)
)

init <- list(
	I1 = round(runif(10,1,2500)),
	I2 = round(runif(10,1,2500)),
	I3 = round(runif(10,1,2500))
)

plan <- list(
	number = 2500,
	network = network,
	init = init
)

# Simulation
f <- new("Rvle", file = "potato.vpz", pkg = "archidemio")
	config(f)["proc"] <- "thread"
	config(f)["thread"] <- 4
	config(f)["plan"] <- "linear"
	
r <- results(run(
	f,
	condParametres.E_GridMatrix = plan$network,
	condParametres.E_GridNumber = 2500,
	condParametres.E_InitSpace = plan$init,
	condParametres.E_RateAlloDeposition = 0.3
))

# Mise en forme
d <- llply(r, rvle.shape.grid, nExec=2500)
names(d) <- names(network) ; d <- ldply(d) ; colnames(d)[1] <- "network" 

# Graphique
ggplot(data=d, aes(x, y, z = value)) +
	geom_tile(aes(fill = value)) +
	facet_wrap(~ network, nrow=1) +
	stat_contour(binwidth = 0.2) +
	scale_fill_gradient(low="white", high="black") +
	scale_y_reverse() + opts(aspect.ratio = 1) + theme_bw()

