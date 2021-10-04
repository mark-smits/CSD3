import numpy as np
import random

# read the data from onsegMarkers fmat save
with open('onsegMarkers') as f:
    onsetLines = f.readlines()
f.close()

# read the data from onsegMarkersAdditionalData fmat save
with open('onsegMarkersAdditionalData') as f:
    onsetADLines = f.readlines()
f.close()

# split the lines into words
onsetLinesSplit = []
for line in onsetLines:
    onsetLinesSplit.append(line.split())
onsetADLinesSplit = []
for line in onsetADLines:
    onsetADLinesSplit.append(line.split())

# find matrix dimensions
numRows = int(onsetLinesSplit[1][3])
numColumns = int(onsetLinesSplit[1][4])
linesToProcess = len(onsetLinesSplit) - 2
numRowsAD = int(onsetADLinesSplit[1][3])
numColumnsAD = int(onsetADLinesSplit[1][4])
linesToProcessAD = len(onsetADLinesSplit) - 2

# prepare arrays for data averaging
valuesToAverage = []
for i in range(numColumnsAD - 1):
    valuesToAverage.append([])

# new matrix
newMatrixData = np.zeros([numRows,numColumns + numColumnsAD - 1])
newADMatrix = np.zeros([numRowsAD, numColumnsAD])

iterator = 0
for i in range(linesToProcess):
    onsetLinesSplit[i+2].pop(0)
    onsetLinesSplit[i+2].pop(0)
    onsetLinesSplit[i+2].pop(0)
    onsetLinesSplit[i+2].pop(0)
    onsetLinesSplit[i+2].pop(0)
    while onsetLinesSplit[i+2]:
        item = float(onsetLinesSplit[i+2].pop(0))
        row = int(iterator/numColumns)
        column = iterator % numColumns
        iterator += 1
        newMatrixData[row][column] = item

iterator = 0
for i in range(linesToProcessAD):
    onsetADLinesSplit[i+2].pop(0)
    onsetADLinesSplit[i+2].pop(0)
    onsetADLinesSplit[i+2].pop(0)
    onsetADLinesSplit[i+2].pop(0)
    onsetADLinesSplit[i+2].pop(0)
    while onsetADLinesSplit[i+2]:
        item = float(onsetADLinesSplit[i+2].pop(0))
        row = int(iterator/numColumnsAD)
        column = iterator % numColumnsAD
        iterator += 1
        newADMatrix[row][column] = item

# data averaging
iterator = 0
# assumption that the additiondaldata has a time value starting before the first onset
for i in range(numRowsAD):
    if (newADMatrix[i][0] > newMatrixData[iterator][0] and newADMatrix[i][0] < newMatrixData[iterator][0] + newMatrixData[iterator][1]):
        # if descr falls within duration range
        for j in range(numColumnsAD - 1):
            valuesToAverage[j].append(newADMatrix[i][j + 1])
    elif (newADMatrix[i][0] > newMatrixData[iterator][0] + newMatrixData[iterator][1]):
        # if descr time exceeds duration range
        for j in range(numColumnsAD - 1):
            newMatrixData[iterator][numColumns + j] = np.mean(valuesToAverage[j])
            valuesToAverage[j] = []
        iterator += 1

lines = ['#obj 1 fmat']
lines.append('#mess 1 size ' + str(numRows) + ' ' + str(numColumns + numColumnsAD - 1))
for i, row in enumerate(newMatrixData):
    line = '#mess 1 set ' + str(i) + ' 0 '
    for item in row:
        line += str(item) + ' '
    lines.append(line)
with open('newData', 'w') as f:
    for line in lines:
        f.write(line)
        f.write('\n')
f.close()

# sorting indices for each parameter
rotatedMatrix = np.transpose(newMatrixData)
for parameter in range(numColumns + numColumnsAD - 1):
    listToWrite = []
    paramArray = []
    sortedArray = []
    for value in rotatedMatrix[parameter]:
        paramArray.append(value)
        sortedArray.append(value)
        sortedArray.sort()
    for value in paramArray:
        listToWrite.append( sortedArray.index(value) )
    #print(listToWrite)
    with open('sortedVectorForParam' + str(parameter), 'w') as f:
        f.write('#obj 1 fmat\n')
        f.write('#mess 1 size ' + str(numRows) + ' 1\n')
        values = '#mess 1 set 0 0'
        for value in listToWrite:
            values += ' ' + str(value)
        f.write(values)

'''
myMatrix = np.zeros([4, 5])
for row in range(4):
    for column in range(5):
        myMatrix[row][column] = random.randint(0, 20)
sortingColumn = 2
print(myMatrix)

myMatrix.sort(axis = 0)
print(myMatrix)
print(np.transpose(myMatrix))

valueToFind = myMatrix[1][1]
print(np.where(myMatrix == valueToFind))
'''
