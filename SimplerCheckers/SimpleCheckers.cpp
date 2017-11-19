/* Simple Checkers by Matthew Freeman */
/* Rules: https://en.wikipedia.org/wiki/English_draughts */

/* Outline: 
Components	- 8x8 Board, 12 pieces per player.

Motion		- Moving: Pieces move diagonally forward either left or right.
			- Jumping: Pieces can capture enemy pieces by jumping over them.
	
Rules		- Players alternate turns and can only move one piece per turn.
			- Players cannot receive kinged pieces, when a piece reaches the end of the board it can no longer move. 
			- Pieces start on the first three rows closest to the owning player and alternate spaces. 
			Additionally each row is set diagonally forward from the previous row. 
			- Multiple jumps are possible. If another jump is available after a jump, it can be taken without
			ending the turn. Players can choose which sequence of jumps they make, but all available jumps
			must be made.
			- If a player has the option to jump, they must take it.
			- A player wins by capturing all enemy pieces or by leaving the opponent with no legal moves.
			- The game ends in a draw if neither side can force a win, or by agreement.
*/

/* Pseudocode */
/* Architectural Requirements
 - 8x8 Board: 2D Array of type Piece 
	- Board must keep track of all the pieces on the board.
	- Board must display the board state after every turn. 
		- Player 1's Pieces labeled with W and Player 2's Pieces labeled with R

 - Two sets of 12 pieces: Struct Piece
	- int owner: integer to show which player owns the piece. 
	Can also be used to determine the direction of the piece.
		- 0 = Empty Space, -1 = Player 1, 1 = Player 2
	- Piece *LeftMove, *RightMove: Stores a reference to the board place where
	the player could make a legal move. Prevents us from having to recalculate if the player is indecisive.
*/

/* Controls */
/* To start with, we will have the player enter an (x, y) coordinate to select the piece. Then l or r
to move the piece, if legal.
*/

/* AI */
/* Start with a simple AI that iterates through all pieces and moves one at random. */

/* Post-Mortem */
/* Things completed: 
	- Players can see a board that accurately shows the state of the game. 
	- Players can select pieces on the board and the movements of the pieces follow the rules
	of the game.
		- Players can move diagonally and jump pieces if jumping is legal.
	- Players can quit the game or change their move.
*/
/* Things not completed: 
	- There is no AI for the player to play against.
	- There are no win or lose conditions.
*/
/* Lessons learned: 
	- Focus harder on implementing the primary mechanics before worrying about input validation.
	- If a game will have an AI, functions must be able to work with input that won't be from the keyboard.
	- Starting by creating a game from an AI perspective might have been more time efficient since 
	player controls, in this situation, were easy to implement. */

#pragma once
#include <iostream>
#include <string>
#include <vector>

using namespace std;

//Helper function declarations.
pair<int, int> CheckInput(string input);

class Piece
{
private:
	int owner;
	pair <int, int> position;
public:
	Piece *left, *right;
	//Constructor for Piece, just sets the piece to unowned and initializes the left and right moves.
	Piece(int NewOwner, int x, int y)
	{
		owner = NewOwner;
		left = nullptr;
		right = nullptr;
		position = pair<int, int>(x, y);
	}
	int GetOwner() { return owner; }
	void SetOwner(int NewOwner) { owner = NewOwner; }
	pair<int, int> GetPosition() { return position; }
};

class Board
{
private:
	int player1, player2;
public:
	Piece *board[8][8];
	vector<Piece*> computerPieces;
	//Board constructor that builds a 8x8 2D array of Pieces.
	Board()
	{
		//Setting up the 2D array for the board.
		for (int i = 0; i < 8; ++i)
		{
			for (int j = 0; j < 8; ++j)
			{
				board[i][j] = new Piece(0, i, j);
			}
		}
		//Set up the pieces for player 2.
		for (int i = 0; i < 3; ++i)
		{
			//Since the row index starts at zero we have to add 1 to i to 
				//get the correct result with this method.
			for (int j = (i + 1) % 2; j < 8; j += 2)
			{
				board[i][j]->SetOwner(1);
				if (CheckBounds(pair<int, int>(i+1, j-1)))
				{
					board[i][j]->left = board[i + 1][j - 1];
				}
				if (CheckBounds(pair<int, int>(i + 1, j + 1)))
				{
					board[i][j]->right = board[i + 1][j + 1];
				}
				computerPieces.push_back(board[i][j]);
			}
		}

		//Set up the pieces for player 1.
		for (int i = 5; i < 8; ++i)
		{
			for (int j = (i + 1) % 2; j < 8; j += 2)
			{
				board[i][j]->SetOwner(-1);
				if (CheckBounds(pair<int, int>(i - 1, j - 1)))
				{
					board[i][j]->left = board[i - 1][j - 1];
				}
				if (CheckBounds(pair<int, int>(i - 1, j + 1)))
				{
					board[i][j]->right = board[i - 1][j + 1];
				}

			}
		}

		//FOR JUMP TESTING
		board[4][3]->SetOwner(1);
	}
	//Function to display the board in it's current state.
	void DisplayBoard()
	{
		int _owner;
		char space;
		// Manually formatted strings to make the board a little more visually pleasing.
		cout << endl;
		cout << "    1   2   3   4   5   6   7   8" << endl;
		cout << "  ---------------------------------" << endl;
		for (int i = 0; i < 8; ++i)
		{
			// Prints the Y axis numbering for the player to see.
			cout << i + 1;
			for (int j = 0; j < 8; ++j)
			{
				// Establish who is the owner of the current piece.
				_owner = board[i][j]->GetOwner();
				// Set the character to display to the corresponding result.
				// ' ' for no player, W for Player 1, R for player 2
				if (_owner == 0)
				{
					space = ' ';
				}
				else if (_owner == -1)
				{
					space = 'W';
				}
				else
				{
					space = 'R';
				}
				cout << " | " << space;
			}
			// More manual formatting.
			cout << " |" << endl;
			cout << "  ---------------------------------" << endl;
		}

	}

	//Function that moves a piece between two locations. 
	Piece* MovePiece(pair<int, int> previous, pair<int, int> next, string direction)
	{
		bool jump = false;
		Piece* result; 
		//References for the pieces at the two locations.
		Piece *p1 = board[previous.first][previous.second];
		Piece *p2 = board[next.first][next.second];
		Piece *p3 = nullptr;

		if (p1->GetOwner() == p2->GetOwner() * -1)
		{
			jump = true;
			if (direction == "left")
			{
				p3 = board[next.first + p1->GetOwner()][next.second - 1];
			}
			else if (direction == "right")
			{
				p3 = board[next.first + p1->GetOwner()][next.second + 1];
			}
		}

		//If we aren't jumping, then we're just moving.
		if (!jump)
		{
			//Change ownership of the space.
			p2->SetOwner(p1->GetOwner());

			//Reset left and right for the new space.
			if (CheckBounds(pair<int, int>(next.first + p1->GetOwner(), next.second - 1)))
			{
				p2->left = board[next.first + p1->GetOwner()][next.second - 1];
			}
			if (CheckBounds(pair<int, int>(next.first + p1->GetOwner(), next.second + 1)))
			{
				p2->right = board[next.first + p1->GetOwner()][next.second + 1];
			}
			result = p2;
		}
		//If we are jumping we need to go an extra space.
		else
		{

			//Change ownership of the space.
			p3->SetOwner(p1->GetOwner());

			//Reset left and right for the new space.
			if (CheckBounds(pair<int, int>(next.first + (2*p1->GetOwner()), next.second - 2)))
			{
				p3->left = board[next.first + (2*p1->GetOwner())][next.second - 2];
			}
			if (CheckBounds(pair<int, int>(next.first + (2*p1->GetOwner()), next.second + 2)))
			{
				p3->right = board[next.first + (2*p1->GetOwner())][next.second + 2];
			}

			p2->SetOwner(0);
			p2->left = nullptr;
			p2->right = nullptr;
			result = p3;
		}
		//Reset the original location to an empty space.
		p1->SetOwner(0);
		p1->left = nullptr;
		p1->right = nullptr;

		//All inputs should already be validated by this point, so we don't need to do any extra.
		return result;
	}

	Piece *GetPiece(pair<int, int> coordinates) 
	{ 
		if (CheckBounds(coordinates))
		{
			return board[coordinates.first][coordinates.second];
		}
		return nullptr;
		

	}

	bool isValid(pair<int, int> move, int owner, string direction)
	{
		//If we own the piece, we cannot jump it.
		Piece *newPosition = GetPiece(move);
		if (newPosition->GetOwner() == owner)
		{
			return false;
		}
		//If there is no piece, we can move there.
		else if (newPosition->GetOwner() == 0)
		{
			return true;
		}
		else
		{
			Piece* jump;
			if (direction == "left")
			{
				jump = GetPiece(pair<int,int>(move.first + owner, move.second - 1));
				//If there is no piece at the jump location we can jump.
				if (jump != nullptr && jump->GetOwner() == 0)
				{
					return true;
				}
				//If there is a piece
				else
				{
					return false;
				}
			}
			else
			{
				jump = GetPiece(pair<int, int>(move.first + owner, move.second + 1));
				//If there is no piece at the jump location we can jump.
				if (jump != nullptr && jump->GetOwner() == 0)
				{
					return true;
				}
				//If there is a piece
				else
				{
					return false;
				}
			}
		}
	}
	//Checks to ensure that the player owns the piece they are trying to select.
	bool CheckOwner(pair<int, int> piece, int owner)
	{
		//Check to see if piece selection is out of bounds.
		if (!CheckBounds(piece))
		{
			cout << "Out of bounds!" << endl;
			return false;
		}
		//Check if the player owns that piece.
		if (board[piece.first][piece.second]->GetOwner() == owner)
		{
			return true;
		}
		cout << "Illegal selection!" << endl;
		return false;
	}

	//Checks if the given location is out of bounds
	bool CheckBounds(pair<int, int> piece)
	{
		if (piece.first < 0 || piece.first > 7)
		{
			return false;
		}
		else if (piece.second < 0 || piece.second > 7)
		{
			return false;
		}
		return true;
	}
};


int main()
{
	//Variable declarations
	string input = "", direction = "";
	Board *NewBoard = new Board();
	pair<int, int> PieceLocation; 

	//Infinite loop that continues the game until the player types "q".
	do
	{
		NewBoard->DisplayBoard();
		cout << "Select a piece with two numbers (XY) or type 'q' to quit: " << endl;
		cin >> input;
		//Player inputs a piece to target.
		PieceLocation = CheckInput(input);
		//Check if player owns that piece.
			//Checks boundaries and ownership.
		if (NewBoard->CheckOwner(PieceLocation, -1))
		{
			Piece *selected = NewBoard->GetPiece(PieceLocation);
			cout << "What direction are you moving?" << endl;

			//Check if diagonal left and diagonal right are viable
			//If owner == 0, return true, space is empty
			//Else if Owner == this.owner, return false, you can't jump your own pieces.
			//Else if X < 0 or X > 8, return false, out of bounds.
			//Else if y < 0 or y > 8, return false, out of bounds.
			//Else owner is enemy, check for jump. 
				//If jump space is unowned and not out of bounds, return true.
				//Else return false, space is occupied or out of bounds.

			//Output the available moves to the player.
			bool moves[] = { true, true };
			if (selected->left != nullptr && NewBoard->isValid(
				pair<int, int>(PieceLocation.first - 1, PieceLocation.second - 1), selected->GetOwner(), "left"))
			{
				cout << "L: Diagonal Left" << endl;
			}
			else { moves[0] = false; }
			if (selected->right != nullptr && NewBoard->isValid(
				pair<int, int>(PieceLocation.first - 1, PieceLocation.second + 1), selected->GetOwner(), "right"))
			{
				cout << "R: Diagonal Right" << endl;
			}
			else { moves[1] = false; }
			//Get the direction for the player's move.
			cout << "B: Back" << endl;
			cin >> direction;
			direction = tolower(direction[0]);

			//Move the piece in the direction the player indicates.
			if (direction == "l" && moves[0] == true)
			{
				NewBoard->MovePiece(PieceLocation,
					pair<int, int>(PieceLocation.first - 1, PieceLocation.second - 1), "left");
			}
			else if (direction == "r" && moves[1] == true)
			{
				NewBoard->MovePiece(PieceLocation,
					pair<int, int>(PieceLocation.first - 1, PieceLocation.second + 1), "right");
			}
			else if (direction == "b")
			{
				continue;
			}
			else
			{
				cout << "Sorry, that was an illegal move." << endl;
				continue;
			}
			//Computer's Turn
			//AI, iterate backwards through the list of computer pieces.
				//If that piece has a valid move, make it.
				//Else move onto the next one.

			//TODO: Refactor the Piece class to have it know it's own position in the array.
				//If the pieces themselves knew their own position then it would prevent the need for
				//requiring the player to give coordinates. 
			Piece *comp = nullptr;
			
			//Computer moves are similar to player moves, but require separate variables.
				//comp - A reference to a particular piece.
				//position - the position of comp on the board.
			cout << "Turn Start" << endl;
			for (int i = NewBoard->computerPieces.size()-1; i > 0; --i)
			{
				comp = NewBoard->computerPieces[i];
				pair<int, int> position = comp->GetPosition();

				//Confirm that the computer still owns this piece, if not try the next one.
				if (comp->GetOwner() != 1)
				{
					cout << comp->GetOwner() << " " << comp->GetPosition().first 
						 << ", " << comp->GetPosition().second << endl;
					continue;
				}
				
				//Boolean array to validate what moves are legal.
				bool moves[] = { true, true };
				cout << "checking left" << endl;

				if (comp->left != nullptr && NewBoard->isValid(
					pair<int, int>(position.first + 1, position.second - 1), comp->GetOwner(), "left"))
				{
					cout << "L: Diagonal Left" << endl;
				}
				else { moves[0] = false; }

				cout << "checking right" << endl;
				if (selected->right != nullptr && NewBoard->isValid(
					pair<int, int>(position.first + 1, position.second + 1), comp->GetOwner(), "right"))
				{
					cout << "R: Diagonal Right" << endl;
				}
				else { moves[1] = false; }

				cout << i << endl;
				if (moves[0])
				{
					
					cout << "Moving Left" << endl;
					NewBoard->computerPieces[i] = NewBoard->MovePiece(position, 
						pair<int, int>(position.first + 1, position.second - 1), "left");
					break;
				}
				cout << "Didn't move left" << endl;
			}
			
		}
	} while (input != "q");
	return 0;
}

pair<int, int> CheckInput(string input)
{
	//TODO Input validation.
	char x = input[0], y = input[1];
	int a = (int)x - '0', b = int(y) - '0';
	--a;
	--b;
	return pair<int, int>(b, a);
}
