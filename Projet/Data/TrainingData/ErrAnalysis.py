 # coding: utf-8
import matplotlib
import matplotlib.pyplot as plt

file1 = open("Error.txt", "r")
file2 = open("Error2.txt", "r")
# liste des abscisses et ordonnees succesives
absc = []
ordo = []
ordo2 = []
currabsc = 0

#parcours du fichier
for line in file1:
    currabsc+=1
    absc.append(currabsc)
    ordo.append(float(line))
    
for lineOne in file2:
    x = float(lineOne)
    ordo2.append(x)

# plot graph
plt.plot(absc,ordo, label = "Error(trainingNumber)")
plt.plot(absc,ordo2, label = "Error(trainingNumber2)")
plt.legend()
plt.grid(color='b',linestyle='-',linewidth=0.01)
plt.show()

file1.close()
file2.close()
