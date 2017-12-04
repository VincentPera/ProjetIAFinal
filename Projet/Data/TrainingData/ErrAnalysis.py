 # coding: utf-8
import matplotlib
import matplotlib.pyplot as plt

analysisFile = open("Error.txt", "r")
# liste des abscisses et ordonnees succesives
absc = []
ordo = []
currabsc = 0

#parcours du fichier
for line in analysisFile:
    absc.append(currabsc)
    ordo.append(float(line))
    currabsc+=1
    
# plot graph
plt.plot(absc, ordo, label = "Error(trainingNumber")
plt.legend()
plt.grid(color='b',linestyle='-',linewidth=0.5)
plt.show()

analysisFile.close()
