/*

Data Generation File that "matches"
the current version of Tilted against itself. 

4-28-24
Nathaniel Potter

*/

#include "STiltedSearchEvalV2.cpp"

std::string makeFen(){
	uint64_t occ = pieces[6] | pieces[13];
	std::string result = "";

	uint64_t squarebb;

	char names[14] = {'k', 'r', 'n', 'q', 'b', 'p', 'x',
						'K', 'R', 'N', 'Q', 'B', 'P', 'X'};

	int emptyCount;

	for (int i = 0; i < 8; i++){ //for each of 8 rows
		emptyCount = 0;
		for (int j = 0; j < 8; j++){ //for each square in each row
			squarebb = 1ULL << (8 * i + j);

			//std::cout << "Considering Square: " << 8 * i + j << '\n';

			if (squarebb & occ){ //if landed on occupied space
				//std::cout << "Intersection Found\n";
				if (emptyCount != 0){ //unload stored empty squares
					result += emptyCount + 48;
					emptyCount = 0;
					//std::cout << "Unloaded Empty Squares\n";
				}

				for (int k = 0; k < 14; k++){ //find it by looping through bitboards
					if (squarebb & pieces[k]){ //if found which bb has it
						result += names[k]; //append label to FEN
						//std::cout << "Found " << names[k] << '\n';
						break;
					}
				}
				emptyCount = 0; //no longer empty
			} else { //otherwise, count as empty square
				emptyCount++;
				//std::cout << "Is Empty Square\n";
			}
		}
		if (emptyCount != 0){
			result += emptyCount + 48;
		}
		if (i != 7){
			result += '/';
		}
	}

	result += ' ';
	result += (toMove ? 'w' : 'b');
	result += " - - ";
	result += std::to_string(chm[thm]);
	result += " 1";

	return result;
}

void resetEngine(){ //equivalent to ucinewgame
	setStartPos();
	//evaluateScratch();
	thm = 0;
	beginZobristHash();
	for (int i = 0; i < 0xFFFFF; i++){
		if (ttable[i].enType != -1){
			ttable[i] = TTentry();
		}
	}
}

int main(int argc, char** argv){
	if (argc < 5){
		std::cout << "Missing Command Line Arguments\n";
		return 0;
	}
	srand(time(0));

	std::cout << "NumGames: " << argv[1] << '\n';
	std::cout << "WriteFile: " << argv[2] << '\n';
	std::cout << "NodesPerMove: " << argv[3] << '\n';
	std::cout << "NumRandMoves: " << argv[4] << '\n';
	

	int numGames = stoi((std::string) argv[1]);
	mnodes = stoi((std::string) argv[3]);

	std::ofstream outputFile(argv[2]);
	int numRandMoves = stoi((std::string) argv[4]);
	
	initializeTables();

	std::string fens[1000];
	int givenScore, tempScore;
	std::string res;

	uint64_t oPos[14]; //original pieces
    bool oMove;  //original move
	int othm;

	int aGames = 0;

	int randMoveNum;

	while (aGames < numGames){ //for each game
		resetEngine(); //reset analysis functions
		fens[0] = "GAME " + std::to_string(aGames);
		std::cout << "\nStarted Game " << aGames << '\n';

		for (int mm = 0; mm < numRandMoves; mm++){
			randMoveNum = fullMoveGen(0, 0);
			makeMove(moves[0][rand() % randMoveNum], 1, 0);
			if (isChecked()){
				makeMove(moves[0][rand() % randMoveNum], 0, 0);
				mm--;
			}
		}

		std::cout << "Moves: ";

		//printAllBitboards();

		while (true){
			for (int i = 0; i < 14; i++){
				oPos[i] = pieces[i];
			}
			oMove = toMove;
			othm = thm;

			givenScore = iterativeDeepening(0xFFFFFFFF, 64);

			for (int i = 0; i < 14; i++){
				pieces[i] = oPos[i];
			}
			thm = othm;
			toMove = oMove;
			nodes = 0;
			
			makeMove(bestMove, 1, 0);
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			fens[thm] = makeFen();
			//std::cout << "FEN " << thm << " made: " << fens[thm] << '\n';
			if (thm % 50 == 0){
				std::cout << thm << ' ';
			}

			if (chm[thm] == 140){
				res = "0.5";
				break;
			}
			tempScore = givenScore * (1 - 2 * toMove);
			if (tempScore < -28000){
				res = "0";
				break;
			}
			if (tempScore > 28000){
				res = "1";
				break;
			}

			//3fold detection
			int rind = thm;
			int reps = 0;
			while (chm[rind]){ //search until the latest reset
				if (zhist[thm] == zhist[rind]){
					reps++;
				}
				rind -= 2;
			}
			if (reps > 2){
				res = "0.5";
				break;
			}

			if ((pieces[0] == pieces[6]) and (pieces[7] == pieces[13])){
				res = "0.5";
				break;
			}

		}
		
		std::cout << thm;

		if ((thm > 100)){
			std::cout << "\nSaving Result " << aGames << '\n';
			outputFile << "GAME " << aGames << '\n';
			outputFile << "Result: " << res << '\n';
			for (int j = 20; j < thm - 1; j++){
				if (chm[j] > 15){
					continue;
				}
				outputFile << fens[j] << '\n';
			}
			outputFile << "Final Pos: " << fens[thm] << '\n';
			aGames++;
			std::this_thread::sleep_for(std::chrono::seconds(1));
		} else {
			std::cout << "\nDefective Data";
		}
	}

	return 0;
}