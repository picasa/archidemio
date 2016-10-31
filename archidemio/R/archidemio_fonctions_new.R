require(rvle)

### Mise en forme des résultats de simuation pour sortie graphique ###

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


#' transform a date in julian day
#'  input : "2014-01-01"  
#'  output : 2456659

getDateNum = function (dateStr)
{
  return (as.numeric(as.Date(dateStr, format="%Y-%m-%d") + 2440588));
}


#' transform a date of the form 2456659 into an object Date
#'  @param : dateNum eg:2456659  
#'  @return  : eg. as.Date("2014-01-01")
 
getDate = function(dateNum)
{
  return(as.Date(as.Date(dateNum, origin="1970-01-01") - 2440588));
}

#' transform a date of the form 2456659 into string "2014-01-01"
#'  @param : dateNum eg:2456659  
#'  @param : format of output
#'  @return  : eg. "2014-01-01" 

getDateStr = function(dateNum, outFormat="%Y-%m-%d")
{
  return (format(getDate(dateNum),
                 format= outFormat)) ;
}

### Production de matrices de voisinage pour UnitPilot ###
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
  # vecteur contient les connexions du point supérieur gauche du domaine
  # avec les points du domaine étendu
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

## rvle.setTranslator() : attribue des conditions pour l'extension GraphTranslator à un objet VLE
getAdjacency <- function (
  n, type="lattice", 
  neighbour=matrix(c(0,1,0,-1,1,0,-1,0),ncol=2,byrow=TRUE) # 4 voisins
) {
  
  ## Construction de la matrice d'adjacence (A)
  # Grille 4 voisins, dirig?
  if (type=="lattice") {
    G <- graph.lattice(c(sqrt(n),sqrt(n)), directed=T, mutual=T)
    A <- get.adjacency(G)
    
  }
  
  # Graphe complet
  if (type=="full") {
    G <- graph.full(n, directed = F, loops = F)
    A <- get.adjacency(G)
    
  }	
  
  # Grille selon un voisinage (émission) défini.
  if (type=="custom") {
    A = voisinage(neighbour, nbcolonne=sqrt(n), nbligne=sqrt(n))
  }
  
  # Small-World.
  if (type=="smallworld") {
    G = watts.strogatz.game(dim=1, size=n, nei=8, p=0.01)
    A = get.adjacency(G)
  }
  
  # Noms lignes + colonnes
  rownames(A)<-c(1:n)-1
  colnames(A)<-c(1:n)-1
  
  # Sortie
  return(A)
}
