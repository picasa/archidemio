## Bibliotheques & chemins
library(reshape)
library(rvle)


#### Extensions à Rvle, mise en forme ####
## rvle.sim() : simuler et indexer les données produites par un modèle VLE
rvle.sim <- function (
	object,			# pointeur vers le modèle VLE
	index = c("time","Top.model.Crop.CropPhenology.ThermalTime"),	# variables d'index temporels
	nVarNormal = 2, # variables non Executive (sans les index de temps)
	nVarExec = 11,	# variables observées par modèle Executive
	nExec = n,		# nombre de modèles créés par Executive
	resume = FALSE,	# résume les sorties pour effectuer une analyse de sensibilité
	view = "debug"
	) {
	# durée de la simulation, attention que des variables soient bien observées sur cette durée 
	simLength=rvle.getDuration(object@sim) + 1
	
	# 1 seule simulation 
	object <- run(object)
	sim <- as.data.frame(object@outlist) 
	
	# remplacer le code de date pour time
	sim$time <- 1:simLength
	
	if (view=="debug") {
		m <- melt(sim, id=index) 
		unit <- c(rep(rep(NA, each=simLength), each=nVarNormal), rep(rep(1:nExec, each=simLength), each=nVarExec)) 
		scale <- c(rep("crop",nVarNormal*simLength), rep("unit", simLength*nVarExec*nExec))
		# Tout rassembler dans un dataframe
		d <- data.frame(
			time=m$time,
			ThermalTime=m$Top.model.Crop.CropPhenology.ThermalTime, 
			scale=as.factor(scale), 
			variable=sub(".*\\.","", m$variable), 
			unit=as.factor(unit), 
			value=m$value
		)
	}
	
	if (view=="sensitivity") {
		m <- melt(sim, id="time") 
		unit <- c(rep(rep(1:nExec, each=simLength), each=1)) 
		# Tout rassembler dans un dataframe
		d <- data.frame(
			time=m$time,
			variable=sub(".*\\.","", m$variable), 
			unit=as.factor(unit), 
			value=m$value
		)	
	}
	
	# Fonction de post-traitement pour résumer les sorties
	if (resume == TRUE) {
		# somme de la colonne value
		r <-  sum(d$value, na.rm=TRUE)
		return(r)
	} else return(d)
}

## rvle.shape() : indexer les données produites par un modèle VLE
rvle.shape <- function (
	object,			# Objet de classe "rvle", après run
	index = c("time","Top.model.Crop.CropPhenology.ThermalTime"),	# variables d'index temporels
	nVarNormal = 2, # variables non Executive (sans les index de temps)
	nVarExec = 12,	# variables observées par modèle Executive
	nExec = n,		# nombre de modèles créés par Executive
	view = "debug"  # nom de la vue active dans le modèle
	) {
	
	if (class(object)!="list") {
		# durée de la simulation, attention que des variables soient bien observées sur cette durée 
		simLength=rvle.getDuration(object@sim) + 1
		# dataframe des sorties
		sim <- as.data.frame(object@outlist)
	} else {
		# dataframe des sorties (objet directement passé à la fonction)
		sim <- as.data.frame(object)
		# durée selon la taille du data.frame()
		simLength=length(sim$time)
	}
	
	# remplacer le code de date pour time
	sim$time <- 1:simLength
	
	# passage au format "long" : index = time | ThermalTime
	# Construction des index manquants : numero d'unité et type de variable (culture / unité) 
	# [TODO] : en fonction du nom de colonne
	# [TODO] : detecter la vue active dans le modèle
	if (view=="debug") {
		m <- melt(sim, id=index) 
		unit <- c(rep(rep(NA, each=simLength), each=nVarNormal), rep(rep(1:nExec, each=simLength), each=nVarExec)) 
		scale <- c(rep("crop",nVarNormal*simLength), rep("unit", simLength*nVarExec*nExec))
		# Tout rassembler dans un dataframe
		d <- data.frame(
			time=m$time,
			ThermalTime=m$Top.model.Crop.CropPhenology.ThermalTime, 
			scale=as.factor(scale), 
			variable=sub(".*\\.","", m$variable), 
			unit=as.factor(unit), 
			value=m$value
		)
	}
	
	if (view=="sensitivity") {
		m <- melt(sim, id="time") 
		unit <- c(rep(rep(1:nExec, each=simLength), each=1)) 
		# Tout rassembler dans un dataframe
		d <- data.frame(
			time=m$time,
			variable=sub(".*\\.","", m$variable), 
			unit=as.factor(unit), 
			value=m$value
		)	
	}
	
	return(d)
}


## rvle.getAllConditionPortValues() : Obtenir les valeurs nominales des paramètres
## TODO : Adapter aux vpz contenant un plan linéaire.
rvle.getAllConditionPortValues <- function(self, condition) {
	stopifnot(is.rvle(self))
    stopifnot(is.character(condition))
    
	ports <- rvle.listConditionPorts(self, condition)
	t <- NULL
	for (p in ports) {
		t <- c(t,rvle.getConditionPortValues(self,condition,p))
	}
	names(t)<- ports
	return(t)
}


## rvle.setTranslator() : attribue des conditions pour l'extension GraphTranslator à un objet VLE
## graphe : matrice de n noeuds à 4 voisins

rvle.setTranslator <- function (
	object, condition, class, n, init, type="lattice", 
	neighbour=matrix(c(0,1,0,-1,1,0,-1,0),ncol=2,byrow=TRUE) # 4 voisins
	) {

	## Construction de la matrice d'adjacence (A)
	# Grille 4 voisins, dirigé
	if (type=="lattice") {
		G <- graph.lattice(c(sqrt(n),sqrt(n)), directed=T, mutual=T)
		A <- get.adjacency(G)

	}
	
	# Graphe complet
	if (type=="full") {
		G <- graph.full(n, directed = F, loops = F)
		A <- get.adjacency(G)

	}	
	
	# Grille selon un voisinage (emission) défini.
	if (type=="custom") {
		A = voisinage(neighbour, nbcolonne=sqrt(n), nbligne=sqrt(n))
	}
		
	rownames(A)<-c(1:n)-1
	colnames(A)<-c(1:n)-1
	
	
	## Mise en place des conditions de GraphTranslator
	# n : nombre de noeuds (modèles) du graphe	
	rvle.setIntegerCondition(object, condition, "E_GridNumber", n)
	#object@sim
	# Vecteur (string) : modèles à instancier à chaque noeud
	rvle.setStringCondition(object, condition, "E_GridClasses", paste(rep(class,n), collapse=" "))
	# Vecteur (string) : Matrice d'adjacence 
	rvle.setStringCondition(object, condition, "E_GridMatrix", paste(as.vector(A), collapse=" "))
	# Tuple : noeuds d'infection, tirage uniforme dans 1:n
	rvle.setTupleCondition(object, condition, "E_InitSpace", round(runif(init,1,n)))
	#rvle.setTupleCondition(object, condition, "E_InitSpace", init)
	
	# Sortie
	return(A)
}
 




#### Analyse de sensibilité ####

### lhs2bounds : Transformer un plan linéaire centré en un plan entre deux bornes
lhs2bounds <- function (M, bounds, factors) {
    mat = matrix(rep(bounds$min, rep(nrow(M), ncol(M))), 
        nrow = nrow(M))
    mat = mat + as.matrix(M) %*% diag(bounds$max - bounds$min)
    dimnames(mat)[[2]] = factors
    return(mat)
}

### lhs.plan : Construire un plan d'expérience de type hypercube latin
getPlanLHS <- function (factors, bounds, n, repet = NULL, tout = FALSE) 
{
    plan = randomLHS(n, nrow(bounds))
    tirage.lhs = as.data.frame(plan)
    names(tirage.lhs) = bounds$name
    plan = lhs2bounds(plan, bounds=bounds[, c("min", "max")], factors=factors)
    plan = as.data.frame(plan)
    if (!is.null(repet)) {
        rep = sample(seq(repet[, "min"], repet[, "max"]), n, 
            replace = TRUE)
        plan = cbind(plan, rep)
        names(plan) = c(bounds$name, repet[, "name"])
    }
    if (tout) 
        retour = list(plan = plan, tirage.lhs = tirage.lhs)
    else retour = plan
    return(retour)
}

## getPlanMorris : Obtenir un objet adapté à la méthode Morris.
getPlanMorris <- function(factors,  binf=bounds$min, bsup=bounds$max, S=100, K=6, delta=K/(2*(K-1))) {
	# S : pas suffisamment grand pour être sensible aux effets des facteurs
	# K : nombre de niveau de la grille
	
	# Construction du plan d'expérience	
	m <- morris(model=NULL, factors=factors, r = S, scale=T,
				design = list(type="oat", levels=K, grid.jump=delta*(K-1)),
				binf = binf,
				bsup = bsup
	)
	return(m)
}

## getPlanFast: Obtenir un objet adapté à la méthode FAST
getPlanFast <- function(factors,  bounds, n, M=4) {

	# Mise en forme des bornes des facteurs pour FAST
	bounds.fast <- apply(
		cbind(bounds$min, bounds$max), 1,
	    function(x){list(min=x[1],max=x[2])} 
	)
	
	# Construction du plan d'expérience
	m <- fast99(model=NULL, factors=factors, n=n, M, q="qunif", q.arg=bounds.fast)
	
	return(m)
}

## getPlanSobol: Obtenir un objet adapté à la méthode Sobol
getPlanSobol <- function(factors,  bounds, n, order = 1, nboot = 0, conf = 0.95) {

	# Tirage de Monte-Carlo
	M1 <- matrix(runif(length(factors) * n), nrow=n)
	M2 <- matrix(runif(length(factors) * n), nrow=n)
	# Ajustement des matrices aux bornes des facteurs
	M1.i <- lhs2bounds(M1, bounds, factors)
	M2.i <- lhs2bounds(M2, bounds, factors)

	
	# Construction du plan d'expérience
	m <- sobol2002(model = NULL, M1.i, M2.i, order = order, nboot = nboot, conf = conf)
	return(m)
}


## rvle.addPlanCondition : Ajouter le plan défini à la condition du modèle.
## /!\ fonctionne seulement pour des conditions de reels.
rvle.addPlanCondition <- function(self, condition, plan=f.plan, factors) {
	for (p in factors) {
		rvle.clearConditionPort(self, condition, p)
		lapply(plan[,p], function(value) {rvle.addRealCondition(self, condition, p, value)})
	}
}

## compute.output : Résumer les sorties brutes du modèle (d.raw) en une variable d'interet
# pour l'analyse de sensibilité : pour modèle 1D & 2D
# TODO : une seule méthode selon le type de modèle (plus de particularité pour graphe dynamique)
compute.output <- function (plan, data, type="1D") {
	
	if (type == "1D") {
		# Mise en forme des données brutes de simulation 1D
		d.long <- lapply(data, melt, id=c("time","Top model,Crop:CropPhenology.ThermalTime"))
		
		# Résumé des données brutes : intégration sur la durée de simulation et sur les instances
		y=NULL
		for (i in 1:length(d.long)) {
			y=c(y, sum(d.long[[i]]$value, na.rm=T))
		}
	} else {
		# Mise en forme
		d.long <- lapply(data, melt, id="time")
		
		# Intégration
		y=NULL
		for (i in 1:length(d.long)) {
			y=c(y, sum(d.long[[i]]$value, na.rm=T))
		}
	}
	
	# Construction d'un objet contenant le plan d'expérience (X) et la réponse du modèle
	return(data.frame(plan, y))
}


## Méthodes graphiques comparables selon les méthodes d'analyse de sensibilité
## plot.morris
plot.morris <- function (x, factors, gfx = TRUE) {

	# Calcul indices (cf. getS3method("plot","morris"))
	index <- data.frame(
		type = "Morris",
		labels = factor(colnames(x$ee), levels=factors),
		first = apply(x$ee, 2, function(x) mean(abs(x))),
		total = apply(x$ee, 2, sd)
		
	)
	
	# Dotplot	
	if (gfx == TRUE) {
		trellis.par.set(canonical.theme(color = FALSE))
		dotplot(labels ~ first + total, data=index, auto.key=list(space="bottom"), 
			xlab="Sensitivity indexes (Morris)"
		)	
	} else {
		return(index)
	}	
}

## plot.fast
plot.fast <- function (x, factors, gfx = TRUE) {

	# Calcul indices (cf. getS3method("plot","fast99"))
	index <- data.frame(
		type = "FAST",
		labels = factor(colnames(x$X), levels=factors),
		first = x$D1/x$V,
		total = 1 - x$Dt/x$V	
	)
	
	# Dotplot	
	if (gfx == TRUE) {
		trellis.par.set(canonical.theme(color = FALSE))
		dotplot(labels ~ first + total, data=index, auto.key=list(space="bottom"), 
			xlim=c(-0.1,1.1), xlab="Sensitivity indexes (FAST)"
		)
	} else {
		return(index)
	}		
}

## plot.sobol
plot.sobol <- function (x, factors, gfx = TRUE) {

	# Calcul indices (cf. getS3method("plot","sobol"))
	index <- data.frame(
		type = "Sobol",
		labels = factor(colnames(x$X), levels=factors),
		first = x$S[,1],
		total = x$T[,1]	
	)
	#index <- melt(index)
	
	# Dotplot
	if (gfx == TRUE) {
		trellis.par.set(canonical.theme(color = FALSE))
		dotplot(labels ~ first + total, data=index, auto.key=list(space="bottom"), 
			xlim=c(-0.1,1.1), xlab="Sensitivity indexes (Sobol)"
		)
	} else {
		return(index)
	}	
	
	# Dotplot bibliothèque ggplot
	#src <- ggplot(index, aes(labels, value))
	#src + geom_point(aes(shape = variable)) + 
	#	scale_x_discrete("") +
	#	scale_y_continuous(limits=c(0, 1), "Sensitivity indexes (Sobol)") + 
	#	theme_bw() + coord_flip()
}





##### Fonctions de construction de matrices d'adjacence et de graphes #####
#### R. Faivre 01/2011
voisinage= function( X = matrix(c(0,1,0,-1,1,0,-1,0),ncol=2,byrow=TRUE), nbligne = 5, nbcolonne = 4, verbose = FALSE) {

# X = matrice à 2 colonnes caractérisant les connexions
# nbligne  = nombre de lignes du réseau
# nbcolonne = nombre de colonnes du réseau

nbl.expand = nbligne+diff(range(X[,2]))
nbc.expand = nbcolonne+diff(range(X[,1]))
dim.max = nbc.expand * nbl.expand
vecteur = rep(0, dim.max)
decal.col = - min(X[,1]) + 1
decal.lig = + max(X[,2]) + 1

Xb = X
Xb[,1] = Xb[,1] + decal.col
Xb[,2] = - Xb[,2] + decal.lig

# En fait ce sont les numeros des indices
decalage = c(decal.col, decal.lig)

# Numéros des points appartenant au domaine d'étude à l'intérieur du domaine étendu
numeros.valide = matrix(seq(1,dim.max),ncol= nbc.expand, byrow=TRUE)
numeros.valide = numeros.valide[seq(decalage[2],length= nbligne),seq(decalage[1],length = nbcolonne)]

# suite est dans l'ordre de lecture de gauche à droite, de haut en bas
suite = (Xb[,2] - 1)* nbc.expand  + Xb[,1] 
# récupération des seuls points valides toujours dans l'ordre de lecture
# vecteur contient les connexions du point supérieur gauche du domaine avec les points du domaine étendu
vecteur[suite] = 1

# matrice des connexions 
# indices dans l'ordre de lecture
connexion = list()
connexion[[1]] = NULL

for(i in 2:(nbligne*nbc.expand))  connexion[[i]] = rep(0,length=i-1)
connexion = t(sapply(connexion,function(v) c(v,vecteur)[1:dim.max]))

lignes.valide = matrix(FALSE,ncol = nbc.expand, nrow= nbligne)
lignes.valide[seq(1,length=nbligne),seq(1,length=nbcolonne)] = TRUE
lignes.valide = c(t(lignes.valide))

connexion.fin = connexion[lignes.valide,c(t(numeros.valide))]

if(verbose) 
list(numeros.valide = c(t(numeros.valide)),  connexion = connexion.fin, vecteur = vecteur, voisins = X, nbligne = nbligne, nbcolonne = nbcolonne, Xb=Xb, suite=suite, nbl.expand = nbl.expand, nbc.expand = nbc.expand, decalage=decalage, dim.max=dim.max)
else
connexion.fin
}



#### Fonction de qualité de prédiction ####
# Calculer r² entre observé et simulé
rsq<-function (sim, obs, digits=2) {
	round(cor(sim, obs)^2, digits=digits)
}
# Calculer un biais
biais<-function (sim, obs, digits=2) {
	round(mean(sim - obs), digits=digits)
}
# Calculer RMSE
rmse<- function(sim, obs, digits=2) {
	round(sqrt(mean((sim - obs)^2)), digits=digits)
}
# Calculer l'Efficience du modèle
efficience<- function (sim, obs, digits=2) {
	round(1 - (sum((sim - obs)^2)/sum((obs - mean(obs))^2)), digits=digits)
}

