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
decalage = c(decal.col, decal.lig)
suite = (Xb[,2] - 1)* nbc.expand  + Xb[,1] 
# récupération des seuls points valides
vecteur[suite] = 1
# matrice des connexions 
connexion = list()
connexion[[1]] = NULL

for(i in 2:dim.max)  connexion[[i]] = rep(0,length=i-1)
connexion = t(sapply(connexion,function(v) c(v,vecteur)[1:dim.max]))

##on attaque par la première ligne
#for(i in 2:nbcolonne)  connexion[[i]] = rep(0,length=i-1)
#connexion = t(sapply(connexion,function(v) c(v,vecteur)[1:(nbl.expand*nbcolonne)]))
##on fait les suivantes (nbligne)
#ajout = NULL
#for(j in 2:nbligne) {
#connexplus = cbind( matrix(0, nrow=nbcolonne,ncol= (j-1)*nbc.expand),connexion)[,1:(nbl.expand*nbcolonne)]
#ajout = rbind(ajout, connexplus)
#}
#connexion = rbind(connexion,ajout)

points.ext = matrix(rep(FALSE, length= dim.max), ncol = nbl.expand)
points.ext[ seq(decalage[2],length=nbcolonne) , seq(decalage[1],length=nbligne)] = TRUE
points.ext = c(points.ext)

points.valide = matrix(rep(FALSE, length= dim.max), ncol = nbl.expand)
points.valide[ seq(1,length=nbcolonne) , seq(1,length=nbligne)] = TRUE
points.valide = c(points.valide)

# attention on travaille sur la matrice transposée pour conserver l'ordre par colonne
valide = t(matrix(vecteur,ncol = nbl.expand)[ seq(decalage[2],length=nbcolonne) , seq(decalage[1],length=nbligne)])
#print(valide)
#[points.ext,points.ext]
if(verbose) 
list(valide = c(valide),  connexion = connexion[points.valide,points.ext], points.ext = points.ext, points.valide = points.valide, vecteur = vecteur, voisins = X, nbligne = nbligne, nbcolonne = nbcolonne, Xb=Xb, suite=suite, nbl.expand = nbl.expand, nbc.expand = nbc.expand, decalage=decalage, dim.max=dim.max)
else
connexion[points.valide,points.ext]
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


voisinage.plot = function(fic = result,numero=1) 
{
#mat = matrix(seq(1,to = fic$nbl.expand*fic$nbc.expand),ncol = fic$nbc.expand, byrow=TRUE)
#print(mat)
#suite = result$suite,nbligne = result$nbl.expand, nbcolonne = result$nbc.expand, decalage=result$decalage
plot(c(1,fic$nbcolonne),c(1,fic$nbligne) , type="n", axes=FALSE , xlab="",ylab="")

text (expand.grid(1:fic$nbcolonne,fic$nbligne:1),as.character(seq(1,to = fic$nbligne*fic$nbcolonne)))
text (expand.grid(1:fic$nbcolonne,fic$nbligne:1)[as.logical(fic$connexion[numero,]),],as.character(seq(1,to = fic$nbligne*fic$nbcolonne)
[as.logical(fic$connexion[numero,])]),col="red")
}
