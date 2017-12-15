# Compare a long-window STFT chromagram to the CQT chromagram
# Feature extraction example
import numpy as np
import librosa
import librosa.display

#filename = librosa.util.example_audio_file()
#filename = 'sCcale.mp3'
filename = 'scaleFast.mp3'
y, sr = librosa.load(filename)

# Create chromagram using librosa
chroma_cq = librosa.feature.chroma_cqt(y=y, sr=sr, n_chroma=12)



def chunks(l, n):
    """Yield successive n-sized chunks from l."""
    for i in range(0, len(l), n):
        yield l[i:i + n]

# Calculate the number of frames in an audio clip
itemLen = 0
for item in chroma_cq:
	for x in range(len(item)):
		itemLen += 1

# chroma_cq has 12 bins so div by 12 to get frames
numFrames = itemLen/12
print(numFrames)

# Go through items in chroma_cq append to our new list so we can split
fullList = []
for i in range(int(numFrames)):
	for item in chroma_cq:
		fullList.append(item[i])

# Split into 12 b/c that's how many pitches there are
splitList = chunks(fullList, 12)

# Calculate notes from the splitList
# Each split is an array of 12 numbers
# If the value in that split == 1 then that is the current pitch
# These indicies are appended to a list where number represents notes
# i.e. [0, 0, 2, 2, 4, 4] 
# this is C for two frames, D for two frames, and E for two frames
indexList = []
for item in splitList:
	index = 0
	for i in item:
		if i == 1.0:
			indexList.append(index)
		index += 1
print(indexList)
print(len(indexList))
print(sr)

np.savetxt('pitches_sCale.csv',indexList,newline='\r\n',fmt='%d')




