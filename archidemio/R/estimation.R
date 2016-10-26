library(dplyr)
library(tidyr)
library(rvle)

setwd("~/vle-record/archidemio/archidemio/R_scripts")


archidemio <- new("Rvle", file="yam.vpz", pkg="archidemio")
setDefault(archidemio, outputplugin=c(view="vle.output/storage"))


## Fonction d'erreur

error_yam <- function(params){
  # params[1]=E_RateAlloDeposition, params[2]=E_RateAutoDeposition
  
  setDefault(archidemio,
             cParams.E_RateAlloDeposition=params[1],
             cParams.E_RateAutoDeposition=params[2],
             cParams.P_Porosity=params[3])
  
  # donnees brutes issues de la simulation
  res_raw <- results(run(archidemio)) %>% rvle.shape(nExec=100, nVarExec = 23)
  
  # extraire la variable ScoreArea puis calculer la moyenne des unites
  res_all <- res_raw %>% filter(variable=="AreaRemovedByDesease")
  mean_ScoreArea <- res_all %>% group_by(time) %>% summarise(ScoreArea=mean(value))
  
  # importer les donnees experimentales pour le mode de conduite a plat
  exp_data <- read.table("../data/yam_ScoreArea_2010.csv", sep=";", header=T) %>% filter(mgmt=="plat")
  
  # 
  init_sim_doy <- getDateNum("2010-06-01") - getDateNum("2010-01-010")
  # Ajoute les "time" aux résultats de simulation
  exp_data$time <- getDateNum(getDate(getDateNum("2010-01-01") + (exp_data$doy - 1)))
  
  sim <- mean_ScoreArea[is.element(mean_ScoreArea$time, exp_data$time),]

  r <- sum((sim$ScoreArea - exp_data$ScoreArea)^2)
  print(r)
  return(r)
  }

# Algo 1
library(rgenoud)

Domains = matrix(rep(0,6), ncol=2)
Domains[1,] = c(0.01,0.5)
Domains[2,] = c(0.01,0.5)
Domains[3,] = c(0,1)

# genoud(fn=error_yam, nvars=2, pop.size=10, max.generations=2,
#        boundary.enforcement=2)

# Algo 2
library(cmaes)
optimum = cma_es(par=0.5*(Domains[,1]+Domains[,2]), fn=error_yam, lower=Domains[,1], upper=Domains[,2],
                 control=list(maxit=5, mu=floor(10/2), lambda=10))
optimum
