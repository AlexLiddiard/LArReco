import os
import RootFileReader
import TrackShowerAlg0
import TrackShowerAlg1
import TrackShowerAlg2

if __name__ == "__main__":
	tCorrect = 0
	tIncorrect = 0
	tUnsure = 0
	sCorrect = 0
	sIncorrect = 0
	sUnsure = 0
	directory = "/home/alexliddiard/Desktop/Pandora/LArReco/ROOT Files/"
	#directory = input("Enter a folder path containing ROOT files: ")
	fileList = os.listdir(directory)

	for fileName in fileList:
		print("Now processing: " + fileName)
		events = RootFileReader.ReadRootFile(os.path.join(directory,fileName))
		
		for eventPfos in events:
			for pfo in eventPfos:
				pfoTrueType = pfo.TrueType()

				if pfo.pfoId == 0 or pfoTrueType == -1:
					continue

				print(str(pfo), end = " ")

				# Algorithms
				#result = TrackShowerAlg0.RunAlgorithm(pfo)
				#result = TrackShowerAlg1.RunAlgorithm(pfo)
				result = TrackShowerAlg2.RunAlgorithm(pfo)

				if result == 1:
					print("Result: S", end=" ")
				elif result == 0:
					print("Result: T", end=" ")
				else:
					print("Result: U", end=" ")

				correct = result == pfoTrueType
				if pfoTrueType == 1:
					sCorrect += correct
					sIncorrect += not correct
					sUnsure += result == -1
				else:
					tCorrect += correct
					tIncorrect += not correct
					tUnsure += result == -1
				if correct:
					print("Correct")
				else:
					print("Incorrect")

	print("Overall efficiency: %.2f%%" % (100 * (tCorrect + sCorrect) / (tCorrect + tIncorrect + sCorrect + sIncorrect)))
	print("Track efficiency: %.2f%%" % (100 * tCorrect / (tCorrect + tIncorrect)))
	print("Shower efficiency: %.2f%%" % (100 * sCorrect / (sCorrect + sIncorrect)))
	print("Track purity: %.2f%%" % (100 * tCorrect / (tCorrect + sIncorrect - sUnsure)))
	print("Shower purity: %.2f%%" % (100 * sCorrect / (sCorrect + tIncorrect - tUnsure)))
