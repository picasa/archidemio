## A Generic Model to Simulate Air-Borne Diseases as a Function of Crop Architecture
* full research paper on [PLoS One](http://www.plosone.org/article/info%3Adoi%2F10.1371%2Fjournal.pone.0049406)

### Abstract
In a context of pesticide use reduction, alternatives to chemical-based crop protection strategies are needed to control diseases. Crop and plant architectures can be viewed as levers to control disease outbreaks by affecting microclimate within the canopy or pathogen transmission between plants.  
Modeling and simulation is a key approach to help analyzing the behaviour of such systems where direct observations are difficult and tedious. Modeling permits to join concepts from ecophysiology and epidemiology to define structures and functions generic enough to describe a wide range of epidemiological dynamics. Additionally, this conception should minimize computing time by both limiting the complexity and setting an efficient software implementation.
In this paper, our aim was to present a model that suited these constraints so it can first be used as a research and teaching tool to promote discussions about epidemic management in cropping systems.

The system was modelled as a combination of individual hosts (population of plants or organs) and infectious agents (pathogens) whose contacts are restricted through a network of connections. The system dynamics were described at an individual scale. Additional attention was given to the identification of generic properties of host-pathogen systems to widen the model's applicability domain. Two specific pathosystems with contrasted crop architectures were considered: ascochyta blight on pea (homogeneously layered canopy) and potato late blight (lattice of individualized plants).

The model behavior was assessed by simulation and sensitivity analysis and these results were discussed against the model ability to discriminate between the defined types of epidemics. Crop traits related to disease avoidance resulting in a low exposure, a slow dispersal or a de-synchronization of plant and pathogen cycles were shown to strongly impact the disease severity at the crop scale.


## Installation

### Install VLE simulation platform (1.0.3)
project repository : [https://github.com/vle-forge/vle](https://github.com/vle-forge/vle)

```
git clone git://github.com/vle-forge/vle.git

cd vle
git checkout master1.0

mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
make
cpack -G DEB
sudo dpkg -i vle-1.0.3-*.deb
```

### Install archidemio model

```
git clone git://github.com/picasa/archidemio.git

```
