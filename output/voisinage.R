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


verifvoisinage = function(fic = result) 
{
#mat = matrix(seq(1,to = fic$nbl.expand*fic$nbc.expand),ncol = fic$nbc.expand, byrow=TRUE)
#print(mat)
#suite = result$suite,nbligne = result$nbl.expand, nbcolonne = result$nbc.expand, decalage=result$decalage
plot(c(1,fic$nbc.expand),c(1,fic$nbl.expand) , type="n", axes=FALSE , xlab="",ylab="")

text (expand.grid(1:fic$nbc.expand,fic$nbl.expand:1),as.character(seq(1,to = fic$nbl.expand*fic$nbc.expand)))
text (expand.grid(1:fic$nbc.expand,fic$nbl.expand:1)[fic$suite,],as.character(seq(1,to = fic$nbl.expand*fic$nbc.expand)
[fic$suite]),col="red")
hg = c(fic$decalage[1],fic$nbl.expand - fic$decalage[2]+1)
hd = hg + c(fic$nbcolonne-1,0)
bd= hd - c(0,fic$nbligne -1)
bg = bd - c(fic$nbcolonne-1,0)
#print(rbind(hg,hd,bd,bg,hg))
lines(rbind(hg,hd,bd,bg,hg))
#lines( c(decalage[1],decalage[1],nbcolonne-decalage[1]+1,nbcolonne-decalage[1]+1,decalage[1]), 
# c(nbligne-decalage[2]+1,decalage[2],decalage[2], nbligne-decalage[2]+1,nbligne-decalage[2]+1))
}


voisinage.plot = function(fic = result,numero=1, emission = TRUE) 
{
#mat = matrix(seq(1,to = fic$nbl.expand*fic$nbc.expand),ncol = fic$nbc.expand, byrow=TRUE)
#print(mat)
#suite = result$suite,nbligne = result$nbl.expand, nbcolonne = result$nbc.expand, decalage=result$decalage
plot(c(1,fic$nbcolonne),c(1,fic$nbligne) , type="n", axes=FALSE , xlab="",ylab="")

text (expand.grid(1:fic$nbcolonne,fic$nbligne:1),as.character(seq(1,to = fic$nbligne*fic$nbcolonne)))
if (emission) connex = as.logical(fic$connexion[numero,]) else connex = as.logical(fic$connexion[,numero])
text (expand.grid(1:fic$nbcolonne,fic$nbligne:1)[connex,],as.character(seq(1,to = fic$nbligne*fic$nbcolonne)
[connex]),col="red")
}
