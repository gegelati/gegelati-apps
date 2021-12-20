import os
import re
import statistics

os.chdir("bin")
os.system("rm -f ../res/result.md")
ratio_imb=[1,2,3,4,5,6,7,8,9,10,12,15,18,30,100,300,1000,3000,10000]

for ratio in ratio_imb:
	fres = open("../res/result.md", "a")
	fres.write("results for ratio 1:"+str(ratio)+"\n")
	fres.close()
	buff=[0]*10
	for i in range(10) :
		#clean output temp file
		os.system('echo  > ../res/tmp_file.md')
		#compile the program
		cmake_string = "cmake .. -DCMAKE_BUILD_TYPE=Release -DTARGET_CLASS="+str(i)+" -DRATIO_IMB="+str(ratio)  +  " -DFITNESS="+str(1)
		os.system("rm -rf *")
		os.system(cmake_string)# + " > /dev/null 2>&1")
		os.system("make -j8")# > /dev/null 2>&1")
		os.system("cp -rf cifar-10-batches-bin cifar10")
		#start the learning
		os.system(r"./Release/cifar10 >> ../res/tmp_file.md")
		#Process exit file
		#skip 6 first  lines
		f = open("../res/tmp_file.md", "r")
		for j in range(7):
			f.readline()
		current_max = 0
		res = ""
		for line in f :
			to_parse = line
			f.readline()
			f.readline()
			f.readline()
			tempres = re.findall(r"[-+]?\d*\.\d+|\d+", to_parse)
			if float(tempres[0]) >= float(current_max) :
				res = tempres
				current_max = float(tempres[0])
				buff[i] = float(tempres[0])
		f.close()
		fres = open("../res/result.md", "a")
		fres.write("\t test " + str(i) + " with Minority_class : " + str(i) + " : " + str(res)+"\n")
		fres.close()
	# compute means
	MK = statistics.mean(buff)
	Mmin = min(buff)
	Mmax = max(buff)
	fres = open("../res/result.md", "a")
	fres.write("\t Min : " + str(Mmin) + " Mean : " + str(MK) + " Max : " + str(Mmax) +"\n")
	fres.close()
	
	
	
