from datetime import datetime
import sys

today = datetime.now()

f = open(sys.argv[1] + '\\build_date.h', 'w')

f.write("#ifndef BUILD_DATE_H\n#define BUILD_DATE_H\n")

f.write("#define __DAY__ " + str(today.day) + "\n")
f.write("#define __MONTH__ " + str(today.month) + "\n")
f.write("#define __YEAR__ " + str(today.year)[-2:] + "\n")

f.write("#endif")
        
f.close()