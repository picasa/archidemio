# Methodes pour l'estimation de paramètres dans le modèle archidemio

# Dependances
library(rvle)
library(gdata)
library(ggplot2)
library(lattice)
library(Cairo)

source("fonctions.R")

# 1D : Fonction-code
mod_1D = function(tau, time=ObsTime, ) {
	# Modèle
	f = new("Rvle", file = "1D_0.8.vpz", pkg = "archidemio")
	
	# Config
	#config(f)["plan"] <- "linear"
	config(f)["restype"] <- "dataframe"
	config(f)["replicas"] <- 1 
	config(f)["proc"] <- "thread"
	config(f)["thread"] <- 2
	
	# Simulation
	# Le modèle doit proposer une vue contenant une seule variable
	f <- run(f, condParametres.E_RateAlloDeposition = tau)
	sim <- rvle.shape(f, nExec=25, view="sensitivity")	
	
	# Aggregation (Disease Progress Curve), moyenne sur les unités
	dpc <- aggregate(sim$value, by=list(time=sim$time), mean, na.rm=T)
	colnames(dpc) <- c("time","sim")
	
	# Merge sur les dates d'observations
	dpc <- dpc[(dpc$time %in% time==T),"sim"]
	
	return(dpc) 
}


# 2D : Fonction-code
# Utilisable avec le paramétrage par defaut du vpz pour la matrice de connection
# on ne peut pas utiliser la fonction de paramétrage de GraphTranslator avec la nouvelle
# couche objet de rvle...

mod_2D = function(tau_l, tau_a, time=ObsTime, init=50) {
	# Modèle
	f = new("Rvle", file = "2D_0.8.vpz", pkg = "archidemio")
	
	# Config
	#config(f)["plan"] <- "linear"
	config(f)["restype"] <- "dataframe"
	config(f)["replicas"] <- 1 
	config(f)["proc"] <- "thread"
	config(f)["thread"] <- 2
	
	# Simulation
	# Le modèle doit proposer une vue contenant une seule variable
	f <- run(f, 
		condParametres.E_RateAlloDeposition = tau_l,
		condParametres.E_RateDeseaseTransmission = tau_a,
		condParametres.E_InitTime = init 
	)
	sim <- rvle.shape(f, view="sensitivity", nExec = n)	
	
	# Aggregation (Disease Progress Curve), moyenne sur les unités
	dpc <- aggregate(sim$value, by=list(time=sim$time), mean, na.rm=T)
	colnames(dpc) <- c("time","sim")
	
	# Merge sur les dates d'observations
	dpc <- dpc[(dpc$time %in% time==T),"sim"]
	
	return(dpc) 
}

########################################################################

### Estimation modèle 1D
# Observations = simulations + bruit
ObsTime = seq(30,150, by=20)
obs <- mod_1D(0.5) + rnorm(length(ObsTime), 0, 0.03)

# Graphe
xyplot(mod_1D(0.5) ~ obs, ylab="Simulation", xlab="Observation")

# Ajustement
m <- nls(obs ~ mod_1D(tau), start=list(tau=0.2))


### Estimation modèle 2D
# Observations = simulations + bruit
ObsTime = seq(30,150, by=20)
severity <- mod_2D(0.5) + rnorm(length(ObsTime), 0, 0.03)
# Mesures de sévérité moyenne, conduite à plat
obs <- read.table(file="../data/observation/severity_2010.csv", header=T, sep=";")
ObsTime <- obs[obs$mgmt=="plat","jap"]
severity <- obs[obs$mgmt=="plat","severity"]

# Ajustement
m <- nls(severity ~ mod_2D(tau_l, tau_a), start=list(tau_l=0.4, tau_a=0.4))

# Graphe
# Dynamique
xyplot(mod_2D(tau_l=0.4, tau_a=-0.2, init=50, time=1:250) ~ 1:250, type="l")

# Sim/Obs
xyplot(mod_2D(tau_l=0.5, tau_a=0.5) ~ severity, ylab="Simulation", xlab="Observation")




