# Bilbiothèques
library(ggplot2)
library(Rgraphviz)
library(igraph)

source("fonctions.R")

## Méthodes pour la génération d'un graphe dynamique : dynamique() & structure()

# 1D
# dynamique : retourne les dates thermiques d'ajout de noeuds
dynamique <- function(ttunits = c(70,70,70,50), nbunits = 25) {
	tt <- NULL
	for (u in 1:nbunits) {
		if (u <= length(ttunits)-1) t = u*ttunits[u]
		else t = sum(ttunits[1:length(ttunits)-1]) + (u-(length(ttunits)-1))*ttunits[length(ttunits)]
		tt <- c(tt, t) 
	}
	return(tt)
}
D <- dynamique(nbunits = 30)

# structure : retourne les matrices d'adjacence des graphes
# voisinage unitaire
X <- matrix(c(0,1), ncol=2, byrow=T)

# liste de matrice d'adjacence
A = list()
for (u in 1:length(D)) {
	A[[u]] <- voisinage(X, nbcolonne=1, nbligne=u)
}


## Fonction de dispersion [Scherm1992]
# en 1D
fd <- function(s, t, r, v) {1/(2*pi*(v/r)^2) * exp(r*t - (s*r/v))}

out <- NULL
for (t in 1:150) {	
	s <- 0:30	
	out.d <- data.frame(
		t = t,
		s = s,
		z = fd(s, t=t, r=0.6, v=3.7)
	) 	
	out <- rbind(out, out.d) 
}

xyplot(z ~ s, groups=t, data=out, alpha=0.05, type="l", lty=1)

# sur une grille (distance cellule-source calculée comme le rayon.)
fd.xy <- function(x, y, t, r, v) {1/(2*pi*(v/r)^2) * exp(r*t - ((sqrt(x^2 + y^2))*r/v))}

fd.grid <- function (s, t, r=0.6, v=4) { 
	size <- -s:s
	df <- expand.grid(x=size, y=size) 
	df$z <- fd.xy(x=df$x, y=df$y, t=t, r=r, v=v) 
	df 
} 

ggplot(data=fd.grid(s=100, t=150, r=0.6, v=50), aes(x=x, y=y, z=z)) + geom_tile(aes(fill=z))




## Génération de graphe 

# Bibliothèque Graphviz
rownames(M)<-c(1:dim(M)[1])
colnames(M)<-c(1:dim(M)[1])
G <-new("graphAM", adjMat=M, edgemode="directed")
G <- layoutGraph(G)
graph.par(list(nodes=list(textCol="black", fontsize=10)))
renderGraph(G)

# Biobliothèque igraph
# Paramétrage du voisinage
N = matrix(c(0,1,0,-1,1,0,-1,0),ncol=2,byrow=TRUE) # 4 voisins
N = matrix(c(0,1, 0,2, 0,-1, 0,-2, 1,0, 2,0, -1,0, -2,0),ncol=2,byrow=TRUE) # 4+4 voisins
N = matrix(c(1,0, 1,-1, 0,-1, -1,-1, -1,0, -1,1, 0,1, 1,1), ncol=2,byrow=TRUE) # 8 Voisins

A = voisinage(N, nbcolonne=10, nbligne=10)

# Création d'un objet de type graphe
G <- graph.adjacency(A, mode="directed", weighted=NULL, diag=F)
write.graph(G, format="graphml", file="G.graphml")

# Lecture depuis un générateur externe
G <- read.graph(file="G.graphml", format="graphml")



## structures de données additionnelles pour le graphe
# Adjacence
N <- matrix(c(0,1,0,-1,1,0,-1,0),ncol=2,byrow=TRUE) # 4 voisins
A <- voisinage(N, nbcolonne=10, nbligne=10)
# Distance
D <- as.matrix()
# Proba 


## Autre types de graphes
graph.tree(n, children = 2, mode="out")
graph.full(n, directed = FALSE, loops = FALSE)
graph.full.citation(n, directed = TRUE)

G = watts.strogatz.game(dim=1, size=n, nei=4, p=0.01)
write.graph(G, format="graphml", file="G.graphml")


## Génération de graphes selon un voisinage unitaire
X1 <- matrix(c(0,1, 0,-1, 1,0, -1,0, 2,0, -2,0), ncol=2, byrow=T)
X1 = matrix(c(0,1,0,-1,1,0,-1,0),ncol=2,byrow=TRUE) # 4 voisins
X1 = matrix(c(0,1, 0,2, 0,-1, 0,-2, 1,0, 2,0, -1,0, -2,0),ncol=2,byrow=TRUE) # 8 voisins

X1 = matrix(c(0,-1,1,0,-1,0),ncol=2,byrow=TRUE)
X1 = matrix(c(0,1,0,-1,1,0,-1,0,-1,-1,0,-2),ncol=2,byrow=TRUE)
X1 = matrix(c(0,1,0,-1,0,-2,-1,0,-1,-1,1,0,1,-1),ncol=2,byrow=TRUE)
X1 = matrix(c(0,1,0,-1,0,-2,-1,0,-1,-1,1,0,1,-1, 3,-4),ncol=2,byrow=TRUE)
plot(X1)
# pour modifier les connexions
fix(X1)
X1croix2a = X1

m = voisinage(X1, verbose = TRUE, nbcolonne=10, nbligne=10)
verifvoisinage(m)

dim(result$connexion)
voisinage.plot(numero=20)
voisinage.plot(numero=74)


## Exemples
adjm <- matrix(sample(0:1, 100, replace=TRUE, prob=c(0.9,0.1)), nc=10)
g1 <- graph.adjacency( adjm )
adjm <- matrix(sample(0:5, 100, replace=TRUE, prob=c(0.9,0.02,0.02,0.02,0.02,0.02)), nc=10)
g2 <- graph.adjacency(adjm, weighted=TRUE)
E(g2)$weight

n = ln(0.1) / ln(1-F)

nb <- function (F) {log(0.1) / log(1-F)}
