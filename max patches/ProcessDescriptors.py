import numpy as np
import random

# read the data from onsegMarkers fmat save
with open('testData.txt') as f:
    onsetLines = f.readlines()
f.close()

# read the data from onsegMarkersAdditionalData fmat save
with open('testDataAD.txt') as f:
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
numRows = len(onsetLinesSplit)
numColumns = len(onsetLinesSplit[0])
linesToProcess = numRows
numRowsAD = len(onsetADLinesSplit)
numColumnsAD = len(onsetADLinesSplit[0])
linesToProcessAD = numRowsAD

# prepare arrays for data averaging
valuesToAverage = []
for i in range(numColumnsAD - 1):
    valuesToAverage.append([])

# new matrix
newMatrixData = np.zeros([numRows,numColumns + numColumnsAD - 1])
newADMatrix = np.zeros([numRowsAD, numColumnsAD])

iterator = 0
for i in range(linesToProcess):
    while onsetLinesSplit[i]:
        item = float(onsetLinesSplit[i].pop(0))
        row = int(iterator/numColumns)
        column = iterator % numColumns
        iterator += 1
        newMatrixData[row][column] = item

iterator = 0
for i in range(linesToProcessAD):
    while onsetADLinesSplit[i]:
        item = float(onsetADLinesSplit[i].pop(0))
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
        if iterator > numRows - 1:
            iterator = numRows - 1
            break

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
        values = ''
        for index, value in enumerate(listToWrite):
            if index % 5 == 0:
                values += '\n'
                f.write(values)
                values = '#mess 1 set ' + str(index) + ' 0'
            values += ' ' + str(value)
        f.write(values)
        f.write('\n')
        f.write(values)
