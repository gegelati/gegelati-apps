import os 
import subprocess

os.chdir("bin")

for i in range(10) :
	cmake_string = "cmake .. -DCMAKE_BUILD_TYPE=Release -DMINORITY_CLASS="+i
	os.system(cmake_string)#+ " > /dev/null 2>&1")
	os.system("make -j8")#/dev/null 2>&1")
	os.system(r"./Release/cifar10 > ../res/tmp_file.md")


