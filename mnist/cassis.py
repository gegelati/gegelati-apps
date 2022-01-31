import os
import re
import statistics
import math

# metrics : linear - kappa - MCC - Gscore - f1 - Basic
# 		 0       1     2        3    4       5

def mcc(tp,tn,fp,fn):
	if((tp+fp)*(tp+fn)*(tn+fp)*(tn+fn) == 0):
		return tp*tn-fp-fn
	return (tp*tn-fp*fn)/math.sqrt((tp+fp)*(tp+fn)*(tn+fp)*(tn+fn))

def kappa(tp,tn,fp,fn):
	tot = tp + tn + fp + fn
	pagree=(tp+tn)/tot
	prand=(fn+tp)/tot*(tp+fp)/tot - (tn+fp)/tot*(tn+fn)/tot
	if(prand == 1):
		return 0;
	return (pagree-prand)/(1-prand)

os.chdir("bin")
os.system("rm -f ../res/result_cassis.md")
# 1 - 3 - 5 - 10 -30 - 100 -300 -1000 -3000 - 10000
ratio_imb=[1,3,10,30,100,300,1000,3000,10000]

for ratio in range(9):
	#M is the fitness used
	for m in range(1):
		m = 2
		fin = open("../res/result_cassis.md", "a")
		fin.write("results for ratio 1:"+str(ratio_imb[ratio])+" method : "+ str(m) +"\n")
		fin.close()
		for i in range(10) :
			#clean output temp file
			os.system('echo  > ../res/result_cassis_tmp.md')
			#compile the program
			cmake_string = "cmake .. -DCMAKE_BUILD_TYPE=Release -DTARGET_CLASS="+str(i)+" -DRATIO_IMB="+str(ratio) +  " -DFITNESS="+str(m)
			os.system("rm -rf *")
			os.system(cmake_string)# + " > /dev/null 2>&1")
			os.system("make -j8")# > /dev/null 2>&1")
			#os.system("cp -rf cifar-10-batches-bin cifar10")
			#start the learning
			os.system(r"./Release/mnist >> ../res/result_cassis_tmp.md")
			#Process exit file
			#skip 6 first  lines
			f = open("../res/result_cassis_tmp.md", "r")
			for j in range(7):
				f.readline()
			current_max = -1000
			buff = ""
			res = ""
			for line in f :
				to_parse = line
				f.readline()
				f.readline()
				#f.readline()
				tempres = re.findall(r"[-+]?\d*\.\d+|\d+|\d+", to_parse)
				calc = 0;
				tp = float(tempres[0])
				tn = float(tempres[1])
				fp = float(tempres[2])
				fn = float(tempres[3])
				
				calc = kappa(tp,tn,fp,fn)
				
				print(calc)
				if float(calc) >= float(current_max) :
					res = tempres
					current_max = calc
					print(str(res))
			f.close()
			fres = open("../res/result_cassis.md", "a")
			fres.write("\t test " + str(i) + " with Minority_class : " + str(i) + " : " + str(res)+"\n")
			fres.close()
	
	
	
