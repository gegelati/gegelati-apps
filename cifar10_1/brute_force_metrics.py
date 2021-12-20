import os
import re
import statistics

# metrics : linear - kappa - MCC - Gscore - f1 
# 		 0       1     2        3    4

os.chdir("bin")
os.system("rm -f ../res/result_brute_force.md")
ratio_imb=[10,100]

for ratio in ratio_imb:
	for m in range(	5):
		fin = open("../res/result_brute_force.md", "a")
		fin.write("results for ratio 1:"+str(ratio)+" method : "+ str(m) +"\n")
		fin.close()
		for i in range(10) :
			#clean output temp file
			os.system('echo  > ../res/brute_force_tmp.md')
			#compile the program
			cmake_string = "cmake .. -DCMAKE_BUILD_TYPE=Release -DTARGET_CLASS="+str(i)+" -DRATIO_IMB="+str(ratio) +  " -DFITNESS="+str(m)
			os.system("rm -rf *")
			os.system(cmake_string)# + " > /dev/null 2>&1")
			os.system("make -j8")# > /dev/null 2>&1")
			os.system("cp -rf cifar-10-batches-bin cifar10")
			#start the learning
			os.system(r"./Release/cifar10 >> ../res/brute_force_tmp.md")
			#Process exit file
			#skip 6 first  lines
			f = open("../res/brute_force_tmp.md", "r")
			for j in range(7):
				f.readline()
			current_max = -1000
			res = ""
			for line in f :
				to_parse = line
				f.readline()
				f.readline()
				f.readline()
				tempres = re.findall(r"[-+]?\d*\.\d+|\d+", to_parse)
				if float(tempres[m]) >= float(current_max) :
					res = tempres
					current_max = float(tempres[0])
			f.close()
			fres = open("../res/result_brute_force.md", "a")
			fres.write("\t test " + str(i) + " with Minority_class : " + str(i) + " : " + str(res)+"\n")
			fres.close()
	
	
	
