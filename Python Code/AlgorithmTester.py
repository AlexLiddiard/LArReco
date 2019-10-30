import os
import RootFileReader
import TrackShowerAlg1
import TrackShowerAlg2

if __name__ == "__main__":
	score = 0
	count = 0
	directory = input("Enter a folder path containing ROOT files: ")
	fileList = os.listdir(directory)

	for fileName in fileList:
		print("Now processing: " + fileName)
		events = RootFileReader.ReadRootFile(os.path.join(directory,fileName))
		
		for eventPfos in events:
			for pfo in eventPfos:
				if pfo.pfoId == 0:
					continue
				count += 1
				print(str(pfo), end = " ")
				result = TrackShowerAlg1.RunAlgorithm(pfo)
				if result == 1:
					print("Result: S", end=" ")
				elif result == 0:
					print("Result: T", end=" ")
				else:
					print("Result: U")
				
				if result != -1:
					if result == pfo.TrulyIsShower():
						print("Correct")
						score += 1
					else:
						print("Incorrect")
	print("Accuracy: " + str(100 * score / count) + "%")
