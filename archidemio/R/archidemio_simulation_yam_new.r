source("archidemio_fonctions_new.R")

archidemio <- new("Rvle", file="yam.vpz", pkg="archidemio")
#show(archidemio)

# Set the size of the grid (number of units)
unit.number <- as.integer(20*20)

# Set a random number (between 1 and 4) and placement of initial disease foci
nFoci <- sample(1:4, 1)
initSpace <- sample(1:unit.number, nFoci)

# Define a custom neighbourhood
adjmat <- getAdjacency(type="custom", n=unit.number,
                       neighbour=matrix(c(0, 1,
                                          0,-1,
                                         -1, 0,
                                         -1, 1
                                              ), ncol=2,byrow=TRUE))

# Plot the resulting network
plot(graph_from_adjacency_matrix(adjmat), layout=layout_on_grid,
     vertex.size = 4, vertex.color = NA, vertex.label = NA, edge.arrow.size = 0.2, edge.arrow.width = 1.5)

# Prepare the adjacency matrix for injection into the model by removing the "matrix" attributes
attributes(adjmat) <- NULL

# Set the new parameters, and the output plugin
setDefault(archidemio, outputplugin = c(view = "vle.output/storage"),
           cParams.E_RateAlloDeposition = 0.173,
           cParams.E_RateAutoDeposition = 0.324,
           cParams.P_Porosity = 0.97,
           cParams.E_InitSpace.as_single = initSpace,
           cUnitPilot.adjacencyMatrix.as_single = adjmat,
           cUnitPilot.number = unit.number)

# Lance la simulation
res <- results(run(archidemio))
#str(res)



### Graphe 1D (dynamique temporelle) ###

# Formate les données au format long
data_output <- rvle.shape(res, nExec=unit.number, nVarExec = 23)

# Sélectionne les données pour la variable "ScoreArea"
data_all <- data_output %>% dplyr::filter(variable=="ScoreArea")
data_mean <- data_all %>% group_by(time) %>% summarise(value=mean(value))
data_score <- data_output %>% dplyr::filter(variable=="AreaRemoved") %>% group_by(time) %>% summarise(value=mean(value))

# Charge les donnnées expérimentale
exp_data <- read.table("../data/yam_ScoreArea_2010.csv", sep=";", header=T) %>% dplyr::filter(mgmt=="plat")
init_sim_doy <- getDateNum("2010-06-01") - getDateNum("2010-01-010")
# Ajoute les "time" aux résultats de simulation
exp_data$time <- getDateNum(getDate(getDateNum("2010-01-01") + (exp_data$doy - 1)))

# Graphe des courbes de "ScoreArea" en fonction du temps, individuelles (gris) et moyenne (rouge) 
ggplot(data=exp_data, aes(x=time, y=ScoreArea), group=1) +
  geom_point() +
  geom_line(data=data_all, aes(x=time, y=value, group=unit, color="units"), alpha=0.1) +
  geom_line(data=data_mean, group=1, aes(x=time, y=value, color="mean diseased")) +
  geom_line(data=data_score, group=1, aes(x=time, y=value, color="mean removed")) +
  my_ggplot_theme() +
  scale_colour_manual(name = NULL, 
                      values =c("units"="black","mean diseased"="red","mean removed"="blue"),
                      labels = c("mean diseased","mean removed","units")) +
  theme(legend.position=c(0.2,0.8))

### Graphe 2D (spatial) ###

data_tfinal <- dplyr::filter(data_all, time==max(data_all$time)-30)
data_2D <- dplyr::as_data_frame(cbind(expand.grid(x=1:sqrt(unit.number), y=1:sqrt(unit.number)),
                                      aggregate(value ~ unit, data=data_tfinal, max)))

ggplot(data_2D, aes(x = x, y = y, z=value)) +
  scale_x_continuous(expand = c(0, 0), breaks=seq(1:max(data_2D$x))) + 
  scale_y_continuous(expand = c(0, 0), breaks=seq(1:max(data_2D$y))) +
  geom_tile(aes(fill=value)) +
  stat_contour(binwidth=0.1, size=0.5, colour="grey20", linetype=5) +
  stat_contour(binwidth=0.5, size=0.7, colour="black") +
  my_ggplot_theme() +
  scale_fill_gradient(low="white", high="black", limits=c(0,1))

### Ajouter une barrière à l'infection : 1 rang résistant
attributes(adjmat) <- NULL
dim(adjmat) <- c(unit.number,unit.number)

# Ajout de la barrière
adjmat[,181:200] <- 0
plot(graph_from_adjacency_matrix(adjmat), layout=layout_on_grid, vertex.size=4, vertex.color=NA, vertex.label=NA, edge.arrow.size=0.2, edge.arrow.width=1.5)

# Réinjection de la matrice dans archidemio
attributes(adjmat) <- NULL
setDefault(archidemio, cUnitPilot.adjacencyMatrix.as_single=adjmat)
