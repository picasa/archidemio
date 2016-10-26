library(rvle)
library(dplyr)
library(tidyr)
library(ggplot2)


archidemio <- new("Rvle", file="yam.vpz", pkg="archidemio")
#show(archidemio)

setDefault(archidemio, outputplugin=c(view="vle.output/storage"),
           cParams.E_RateAlloDeposition=0.173,
           cParams.E_RateAutoDeposition=0.324,
           cParams.P_Porosity=0.97)

# Lance la simulation
res <- results(run(archidemio))
#str(res)



### Graphe 1D (dynamique temporelle) ###

# Formate les données au format long
data_output <- rvle.shape(res, nExec=100, nVarExec = 23)

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
ggplot(data=data_all, aes(x=time, y=value, group=unit)) +
  geom_line(alpha=0.1) +
  geom_line(data=data_mean, group=1, color="red") +
  geom_line(data=data_score, group=1, color="blue") +
  geom_point(data=exp_data, aes(x=time, y=ScoreArea), group=1) +
  my_ggplot_theme()




### Graphe 2D (spatial) ###

data_tfinal <- dplyr::filter(data_all, time==max(data_all$time))
nExec=nrow(data_tfinal)
data_2D <- as_data_frame(cbind(expand.grid(x=1:sqrt(nExec), y=1:sqrt(nExec)),
                                    aggregate(value ~ unit, data=data_tfinal, max)))

ggplot(data_2D, aes(x = x, y = y, z=value)) +
  scale_x_continuous(expand = c(0, 0)) + 
  scale_y_continuous(expand = c(0, 0)) +
  geom_tile(aes(fill=value)) +
  stat_contour(binwidth=0.1, size=0.5, colour="grey50") +
  stat_contour(binwidth=0.5, size=1, colour="black") +
  my_ggplot_theme() +
  scale_fill_gradient(limits = c(0, 1), low = "white", high = "black")


