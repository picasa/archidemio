## Bibliotheques & chemins
library(reshape)
library(rvle)


## rvle.sim() : simuler et indexer les données produites par un modèle VLE
rvle.sim <- function (
	model,			# pointeur vers le modèle VLE
	simLength,		# durée de la simulation
	index = c("time","Top.model.Crop.CropPhenology.ThermalTime"),		# variables d'index temporels
	nVarNormal = 3, # variables non Executive (sans les index de temps)
	nVarExec = 10,	# variables observées par modèle Executive
	nExec = 24,		# nombre de modèles créés par Executive
	resume = F		# résume les sorties pour effectuer une analyse de sensibilité
	) {
	sim <- rvle.run(f)
	sim <- as.data.frame(sim)
	# remplacer le code de date pour time
	sim$time <- 1:length(sim$time)
	# passage au format "long" : index = time & ThermalTime
	m <- melt(sim, id=index) 
	# Construction des index manquants : numero d'unité et type de variable (culture / unité) 
	# TODO : en fonction du nom de colonne
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


## rvle.addPlanCondition
