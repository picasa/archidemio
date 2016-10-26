require(rvle)
require(reshape2)

rvle.shape <- function (
  object,    	# Objet de classe "rvle", après run
  nVarNormal = 6, # variables non Executive (sans les index de temps)
  nVarExec = 20,	# variables observées par modèle Executive
  nExec=100,  		# nombre de modèles créés par Executive
  view = "view"  # nom de la vue active dans le modèle
) {
  # dataframe des sorties (objet directement passé à la fonction)
  sim <- as.data.frame(object[[1]])
  # durée selon la taille du data_frame()
  simLength=nrow(sim)
  
  # remplacer le code de date pour time
  # sim$time <- 1:simLength

  # remplacer le nom des colonnes
  # names(sim) <- sub(".*\\.","", names(sim))
  # passage au format "long" : index = time | ThermalTime
  # Construction des index manquants : numero d'unité et type de variable (culture / unité) 
  # TODO index en fonction du nom de colonne
  # TODO detecter la vue active dans le modèle pour éviter les if
  m <- reshape2::melt(sim, id=names(sim)[c(1,6)])
  unit <- c(rep(rep(NA, each=simLength), each=nVarNormal), rep(rep(1:nExec, each=simLength), each=nVarExec), rep(NA, each=simLength)) 
  scale <- c(rep("crop",nVarNormal*simLength), rep("unit", simLength*nVarExec*nExec), rep("crop",simLength))
  # Tout rassembler dans un dataframe
  d <- data.frame(
      time=m$time,
      ThermalTime=m$"top:CropPhenology.ThermalTime", 
      scale=as.factor(scale), 
      variable=sub(".*\\.","", m$variable), 
      unit=as.factor(unit), 
      value=m$value
    )
  return(d)
}

rvle.shape.grid <- function (object, nExec) {
  
  # Dataframe des sorties du mod?le
  sim <- as_data_frame(object[[1]])
  simLength=length(sim$time)
  
  # Dataframe au format long  
  m <- reshape2::melt(sim, id="time") 
  unit <- c(rep(rep(1:nExec, each=simLength), each=1)) 
  d <- data_frame(
    time=m$time,
    variable=sub(".*\\.","", m$variable), 
    unit=as.factor(unit), 
    value=m$value
  )	
  
  # Dataframe au format grille : valeur finale de la variable dynamique
  d <- as_data_frame(cbind(expand.grid(x=1:sqrt(nExec), y=1:sqrt(nExec)),
                           aggregate(value ~ unit, data=d, max)))
  
  # Sortie
  return(d)
}

#'
#' transform a date in julian day
#'  input : "2014-01-01"  
#'  output : 2456659
#'
getDateNum = function (dateStr)
{
  return (as.numeric(as.Date(dateStr, format="%Y-%m-%d") + 2440588));
}

#'
#' transform a date of the form 2456659 into an object Date
#'  @param : dateNum eg:2456659  
#'  @return  : eg. as.Date("2014-01-01")
#' 
getDate = function(dateNum)
{
  return(as.Date(as.Date(dateNum, origin="1970-01-01") - 2440588));
}

#'
#' transform a date of the form 2456659 into string "2014-01-01"
#'  @param : dateNum eg:2456659  
#'  @param : format of output
#'  @return  : eg. "2014-01-01" 
#'
getDateStr = function(dateNum, outFormat="%Y-%m-%d")
{
  return (format(getDate(dateNum),
                 format= outFormat)) ;
  
}

