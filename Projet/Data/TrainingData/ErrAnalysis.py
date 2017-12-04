 # coding: utf-8
import matplotlib, sys
import matplotlib.pyplot as plt

listeFile = []

##collecte des fichiers passes en arguments
for arg in sys.argv:
    print(arg)
    listeFile.append(arg)
## le nom du script ne doit pas etre pris en compte
del listeFile[0]



for fileString in listeFile:
    file = open(fileString, "r")
    y = []
    x=[]
    cnt = 1
    for line in file:
        y.append(float(line))
        x.append(cnt)
        cnt+=1
    plt.plot(x,y, label = "Error("+ fileString +")")
    file.close()

plt.legend()
plt.grid(color='b',linestyle='-',linewidth=0.01)
plt.show()

