import os
import re
import statistics
import math

# metrics : linear - kappa - MCC - Gscore - f1 - Basic
# 		 0       1     2        3    4       5

def basic(tp,tn,fp,fn):
	return tp+tn-fp-fn

def linear(tp,tn,fp,fn,ratio):
	return ratio*tp+tn-ratio*fp-fn

def mcc(tp,tn,fp,fn):
	return (tp*tn-fp*fn)/math.sqrt((tp+fp)*(tp+fn)*(tn+fp)*(tn+fn))

def gscore(tp,tn,fp,fn):
	return math.sqrt((tp/(tp+fn))*(tn/(tn+fp)))

def f1score(tp,tn,fp,fn):
	if(tp+1/2*(fp+fn)!=0):
	    return tp/(tp + 1/2*(fp+fn))
	return -10

def kappa(tp,tn,fp,fn):
	tot = tp + tn + fp + fn
	pagree=(tp+tn)/tot
	prand=(fn+tp)/tot*(tp+fp)/tot - (tn+fp)/tot*(tn+fn)/tot
	return (pagree-prand)/(1-prand)

os.chdir("bin")
os.system("rm -f ../res/result_brute_force.md")
ratio_imb=[1,2,3,4,5,6,7,8,9,10,12,15,18,30,100,300,1000,3000,10000]

for ratio in ratio_imb:
	#M is the fitness used
	for m in range(6):
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
			#os.system("cp -rf cifar-10-batches-bin cifar10")
			#start the learning
			os.system(r"./Release/mnist >> ../res/brute_force_tmp.md")
			#Process exit file
			#skip 6 first  lines
			f = open("../res/brute_force_tmp.md", "r")
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
				if(m==0):
					calc = linear(tp,tn,fp,fn,ratio)
				elif(m==1):
					calc = kappa(tp,tn,fp,fn)
				elif(m==2):
					calc = mcc(tp,tn,fp,fn)
				elif(m==3):
					calc = gscore(tp,tn,fp,fn)
				elif(m==4):
					calc = f1score(tp,tn,fp,fn)
				elif(m==5):
					calc = basic(tp,tn,fp,fn)
				print(calc)
				if float(calc) >= float(current_max) :
					res = tempres
					current_max = calc
					print(str(res))
			f.close()
			fres = open("../res/result_brute_force.md", "a")
			fres.write("\t test " + str(i) + " with Minority_class : " + str(i) + " : " + str(res)+"\n")
			fres.close()
	
	
	
