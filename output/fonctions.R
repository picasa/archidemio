## Bibliotheques & chemins
library(reshape)
library(rvle)


#### Extensions à Rvle, mise en forme ####
## rvle.sim() : simuler et indexer les données produites par un modèle VLE
rvle.sim <- function (
	self,			# pointeur vers le modèle VLE
	index = c("time","Top.model.Crop.CropPhenology.ThermalTime"),	# variables d'index temporels
	nVarNormal = 2, # variables non Executive (sans les index de temps)
	nVarExec = 11,	# variables observées par modèle Executive
	nExec = 25,		# nombre de modèles créés par Executive
	resume = F		# résume les sorties pour effectuer une analyse de sensibilité
	) {
	# durée de la simulation, attention que des variables soient bien observées sur cette durée 
	simLength=(rvle.getDuration(self) + 1)
	# 1 seule simulation  
	sim <- rvle.run(self)
	sim <- as.data.frame(sim)
	# remplacer le code de date pour time
	sim$time <- 1:simLength
	# passage au format "long" : index = time & ThermalTime
	m <- melt(sim, id=index) 
	# Construction des index manquants : numero d'unité et type de variable (culture / unité) 
	# TODO : en fonction du nom de colonne
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
	
	# Fonction de post-traitement pour résumer les sorties
	if (resume == T) {
		# somme de la colonne value
		r <-  sum(d$value, na.rm=T)
		return(r)
	} else return(d)
}

## rvle.shape() : indexer les données produites par un modèle VLE
rvle.shape <- function (
	object,			# Objet de classe "rvle", après run
	index = c("time","Top.model.Crop.CropPhenology.ThermalTime"),	# variables d'index temporels
	nVarNormal = 2, # variables non Executive (sans les index de temps)
	nVarExec = 12,	# variables observées par modèle Executive
	nExec = 25		# nombre de modèles créés par Executive
	) {
	# durée de la simulation, attention que des variables soient bien observées sur cette durée 
	simLength=rvle.getDuration(object@sim) + 1
	# dataframe des sorties
	sim <- as.data.frame(object@outlist)
	# remplacer le code de date pour time
	sim$time <- 1:simLength
	# passage au format "long" : index = time & ThermalTime
	m <- melt(sim, id=index) 
	# Construction des index manquants : numero d'unité et type de variable (culture / unité) 
	# TODO : en fonction du nom de colonne
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






#### Analyse de sensibilité ####

### lhs2bounds : Transformer un plan linéaire centré en un plan entre deux bornes
lhs2bounds <- function (M, bounds, factors) {
    mat = matrix(rep(bounds$min, rep(nrow(M), ncol(M))), 
        nrow = nrow(M))
    mat = mat + as.matrix(M) %*% diag(bounds$max - bounds$min)
    dimnames(mat)[[2]] = factors
    return(mat)
}

## getPlanMorris : Obtenir un objet adapté à la méthode Morris.
getPlanMorris <- function(factors,  binf=bounds$min, bsup=bounds$max, S=10, K=6, delta=K/(2*(K-1))) {
	# S : pas suffisamment grand pour être sensible aux effets des facteurs
	# K : nombre de niveau de la grille
	
	# Construction du plan d'expérience	
	m <- morris(model=NULL, factors=factors, r = S, scale=F,
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
	m <- sobol(model = NULL, M1.i, M2.i, order, nboot, conf)
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

## Méthodes graphiques comparables selon les méthodes d'analyse de sensibilité
## plot.morris
plot.morris <- function (x, factors, ...) {

	# Calcul indices (cf. getS3method("plot","morris"))
	index <- data.frame(
		labels = factor(colnames(x$ee), levels=factors),
		mu.star = apply(x$ee, 2, function(x) mean(abs(x))),
		sigma = apply(x$ee, 2, sd)
		
	)
	
	# Dotplot
	trellis.par.set(canonical.theme(color = FALSE))
	dotplot(labels ~ mu.star + sigma, data=index, auto.key=list(space="bottom"), 
		xlab="Sensitivity indexes"
	)
}

## plot.fast
plot.fast <- function (x, factors, ...) {

	# Calcul indices (cf. getS3method("plot","fast99"))
	index <- data.frame(
		labels = factor(colnames(x$X), levels=factors),
		first = x$D1/x$V,
		total = 1 - x$Dt/x$V	
	)
	
	# Dotplot
	trellis.par.set(canonical.theme(color = FALSE))
	dotplot(labels ~ first + total, data=index, auto.key=list(space="bottom"), 
		xlim=c(-0.1,1.1), xlab="Sensitivity indexes"
	)
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

