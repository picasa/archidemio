## rvle.setTranslator() : attribue des conditions pour l'extension GraphTranslator ? un objet VLE
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
  
  # Grille selon un voisinage (?mission) d?fini.
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
