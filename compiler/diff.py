#python3

######################################################################################
#   The program only prints the diffs [ we should remove the memory from printing]   #
######################################################################################

import sys

# arguments variable holds whatever the script gives
# for example if in the run.sh you give as:
# Directory Path: examples/firstOccurance
# the arguments variable will hold ['examples', 'firstOccurance', '*']
# where * is the name of the file examined.
arguments = sys.argv[1].split("/")

if (arguments[2] == "types" or arguments[2] == "other"):
    arguments[2] = arguments[2] + "/" + arguments[3]

f1 = open(sys.argv[2] + "/" + sys.argv[3] + "/compilers-NTUA/compiler/examples/" + arguments[1] + "/" + arguments[2] + ".txt", "r")  
f2 = open(sys.argv[2] + "/" + sys.argv[3] + "/compilers-NTUA/compiler/examples/prevOutputs/" + arguments[1] + "/" + arguments[2] + ".txt", "r")  
  
i = 0
  
for line1 in f1:
    i += 1
      
    for line2 in f2:
          
        # matching line1 from both files
        if not (line1 == line2):  
            print("Line ", i, ":")
            # else print that line from both files
            print("\tFile 1:", line1, end='')
            print("\tFile 2:", line2, end='')
        break
  
# closing files
f1.close()                                       
f2.close()