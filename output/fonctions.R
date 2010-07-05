## Bibliotheques & chemins
library(reshape)
library(rvle)


## rvle.sim() : simuler et indexer les données produites par un modèle VLE
rvle.sim <- function (
	self,			# pointeur vers le modèle VLE
	index = c("time","Top.model.Crop.CropPhenology.ThermalTime"),	# variables d'index temporels
	nVarNormal = 3, # variables non Executive (sans les index de temps)
	nVarExec = 9,	# variables observées par modèle Executive
	nExec = 25,		# nombre de modèles créés par Executive
	resume = F		# résume les sorties pour effectuer une analyse de sensibilité
	) {
	# durée de la simulation, attention que des variables soient bien observées sur cette durée 
	simLength=(rvle.getDuration(self) + 1)
	# 1 seule simulation  
	sim <- rvle.run(self)
	sim <- as.data.frame(sim)
	# remplacer le code de date pour time
	sim$time <- 1:length(sim$time)
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



## rvle.getAllConditionPortValues() : Obtenir les valeurs nominales des paramètres
rvle.getAllConditionPortValues <- function(self, condition) {
	stopifnot(is.rvle(self))
    stopifnot(is.character(condition))
    
	ports <- rvle.listConditionPorts(self, condition)
	t <- NULL
	for (p in ports) {
		t <- c(t,rvle.getConditionPortValues(self,condition,p))
	}
	return(t)
}



## getPlanMorris : Obtenir un plan d'expérience adapté à la méthode Morris.
getPlanMorris <- function(factors,  binf=bounds$min, bsup=bounds$max, S=10, K=6, delta=K/(2*(K-1))) {
	# S : pas suffisamment grand pour être sensible aux effets des facteurs
	# K : nombre de niveau de la grille
		
	m <- morris(model=NULL, factors=factors, r = S, scale=F,
				design = list(type="oat", levels=K, grid.jump=delta*(K-1)),
				binf = binf,
				bsup = bsup
	)
	return(m)
}



## rvle.addPlanCondition : Ajouter le plan défini à la condition du modèle.
rvle.addPlanCondition <- function(self, condition, plan=f.plan, factors) {
	for (p in factors) {
		rvle.clearConditionPort(self, condition, p)
		lapply(plan[,p], function(value) {rvle.addRealCondition(self, condition, p, value)})
	}
}


## Fonction de qualité de prédiction
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

