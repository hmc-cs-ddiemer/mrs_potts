#Amelia Sheppard 12/7/2017

import csv

values = []

for line in open("rawVolumeData.csv"):
    row = line.split() #returns a list ["1","50","60"]
    num1 = 0
    num2 = 0
    for i in range(len(row[0])):
      c = row[0][i]
      if c == ",":
          num1 = row[0][0:i]
          num2 = row[0][i+1:len(row[0])]
          if num1 == "[-inf]":
              num1 = 0
          if num2 == "[-inf]":
              num2 = 0
    if num1 == 0 and num2 == 0:
        ave = 0.0
    else:
        ave = (float(num1)+float(num2)+200.0)
    values.append(float(ave))


resampled = []
maxVal = max(values)
minVal = min(values)
rangeV = maxVal-minVal
step = 735
for i in range(0, len(values)-10, step):
    new = values[i:i+step]
    average = sum(new)/step
    if average == 0:
        normalized = 0.0
    else:
        normalized = (average-minVal)/rangeV

    if i == 0:
        resampled.append(str(0.0) + " " + str(normalized))
    else:
        resampled.append(str(i/44100.0) + " " + str(normalized))

with open("outputVolume.csv", 'wb') as myfile:
    wr = csv.writer(myfile, quoting=csv.QUOTE_NONE, delimiter='\n', quotechar='',escapechar=' ')
    wr.writerow(resampled)
        

