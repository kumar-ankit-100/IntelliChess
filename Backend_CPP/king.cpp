#include "common_header.h"

int is_king_move_valid(Board &board, string &moves, int flag)
{

    // check for is king move valid
    int source_row = moves[1] - 48;
    int source_col = moves[0] - 97;
    int destination_col = moves[2] - 97;
    int destination_row = moves[3] - 48;
    if (!(abs(destination_col - source_col) <= 1 && abs(destination_row - source_row) <= 1))
    {
        cout << "King can only move 1 ahead in any direction" << endl;
        return 0;
    }

    return 1;
}
// Function to check if a square is under attack by any opponent piece

bool is_path_clear(Board &board, char start_col, int start_row, char end_col, int end_row)
{
    int col_step = (end_col > start_col) ? 1 : (end_col < start_col) ? -1
                                                                     : 0;
    int row_step = (end_row > start_row) ? 1 : (end_row < start_row) ? -1
                                                                     : 0;

    char current_col = start_col + col_step;
    int current_row = start_row + row_step;

    while (current_col != end_col || current_row != end_row)
    {
        Bitboard to_mask = 1ULL << ((current_row * 8) - (current_col - 97) - 1);
        if (board.ouccupancy[both] & to_mask)
        {
            return false; // There is a blocking piece
        }
        current_col += col_step;
        current_row += row_step;
    }

    return true; // Path is clear
}


std::string positionToString(char col, int row)
{
    return std::string(1, col) + std::to_string(row);
}

bool is_square_under_attack_by_bishop_or_queen(Board &board, char col, int row, int opponentColor)
{
    Bitboard bit = 1;
    Bitboard from_mask = bit << ((row * 8) - (col - 97) - 1);

    // Define diagonal directions: top-right, top-left, bottom-right, bottom-left
    int directions[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};


    for (auto dir : directions)
    {
        char cur_col = col;
        int cur_row = row;
        board.emptySquares.clear(); // Clear the vector for each direction

        while (true)
        {
            cur_col += dir[1]; // Move left or right
            cur_row += dir[0]; // Move up or down

            // Check if the current position is out of bounds
            if (cur_col < 'a' || cur_col > 'h' || cur_row < 1 || cur_row > 8)
                break;

            // Calculate the bitboard mask for the current position
            Bitboard to_mask = bit << ((cur_row * 8) - (cur_col - 97) - 1);

            // Check if there's a piece in the current square
            if (board.ouccupancy[both] & to_mask)
            {
                // If it's an opponent's bishop or queen, the square is under attack
                if ((board.pices[opponentColor][bishop] & to_mask) || (board.pices[opponentColor][queen] & to_mask))
                {
    // cout<<"andar aagya"<<endl;
                    // cout<<"found queen in attack"<<endl;
                    board.emptySquares.push_back(positionToString(cur_col, cur_row));
                    return true;
                }
                // Stop moving in this direction after finding any piece
                break;
            }
            else
            {
                // Add empty square to the vector
                board.emptySquares.push_back(positionToString(cur_col, cur_row));
            }
        }
    }

    return false; // No attacking bishop or queen found
}
bool is_square_under_attack_by_rook_or_queen(Board &board, char col, int row, int opponentColor)
{
    // Get all possible rook or queen moves from the current position
    Bitboard bit = 1;
    Bitboard from_mask = bit << ((row * 8) - (col - 97) - 1);

    // Check horizontal and vertical lines for attacks
    // Horizontal (left-right)
    board.emptySquares.clear(); // Clear the vector once per direction
    for (int i = col - 1; i >= 'a'; --i)
    { // Move left
        Bitboard to_mask = bit << ((row * 8) - (i - 97) - 1);

        if (board.ouccupancy[both] & to_mask)
        { // If there's a piece
            if ((board.pices[opponentColor][rook] & to_mask) || (board.pices[opponentColor][queen] & to_mask))
            {
                board.emptySquares.push_back(positionToString(i, row));
                return true; // Opponent's rook or queen is attacking
            }
            break; // Blocked by any piece
        }
        else
        {
            // Add empty square to the vector
            board.emptySquares.push_back(positionToString(i, row));
        }
    }

    board.emptySquares.clear(); // Clear the vector once per direction
    for (int i = col + 1; i <= 'h'; ++i)
    { // Move right
        Bitboard to_mask = bit << ((row * 8) - (i - 97) - 1);

        if (board.ouccupancy[both] & to_mask)
        {
            if ((board.pices[opponentColor][rook] & to_mask) || (board.pices[opponentColor][queen] & to_mask))
            {
                board.emptySquares.push_back(positionToString(i, row));
                return true;
            }
            break;
        }
        else
        {
            // Add empty square to the vector
            board.emptySquares.push_back(positionToString(i, row));
        }
    }

    // Vertical (up-down)
    board.emptySquares.clear(); // Clear the vector once per direction
    for (int j = row + 1; j <= 8; ++j)
    { // Move upwards
        Bitboard to_mask = bit << ((j * 8) - (col - 97) - 1);

        if (board.ouccupancy[both] & to_mask)
        {
            if ((board.pices[opponentColor][rook] & to_mask) || (board.pices[opponentColor][queen] & to_mask))
            {
                board.emptySquares.push_back(positionToString(col, j));
                return true;
            }
            break;
        }
        else
        {
            // Add empty square to the vector
            board.emptySquares.push_back(positionToString(col, j));
        }
    }

    board.emptySquares.clear(); // Clear the vector once per direction
    for (int j = row - 1; j >= 1; --j)
    { // Move downwards
        Bitboard to_mask = bit << ((j * 8) - (col - 97) - 1);

        if (board.ouccupancy[both] & to_mask)
        {
            if ((board.pices[opponentColor][rook] & to_mask) || (board.pices[opponentColor][queen] & to_mask))
            {
                board.emptySquares.push_back(positionToString(col, j));
                return true;
            }
            break;
        }
        else
        {
            // Add empty square to the vector
            board.emptySquares.push_back(positionToString(col, j));
        }
    }

    return false;
}

bool is_square_under_attack(Board &board, char col, int row, int opponentColor)
{
    int pawnDirection = (opponentColor == white) ? -1 : 1;

    // Check if the square is attacked by a pawn from the left diagonal
    board.emptySquares.clear(); // Clear the vector for each direction
    if (col > 'a')
    { // Ensure the column is within bounds
        if (board.pices[opponentColor][pawn] & (1ULL << ((row + pawnDirection) * 8 - (col - 1 - 97) - 1)))
        {
            board.emptySquares.push_back(positionToString(col - 1, row + pawnDirection));
            return true;
        }
    }

    // Check if the square is attacked by a pawn from the right diagonal
    board.emptySquares.clear(); // Clear the vector for each direction
    if (col < 'h')
    { // Ensure the column is within bounds
        if (board.pices[opponentColor][pawn] & (1ULL << ((row + pawnDirection) * 8 - (col + 1 - 97) - 1)))
        {
            board.emptySquares.push_back(positionToString(col + 1, row + pawnDirection));
            return true;
        }
    }

    // Define all possible knight moves (L-shaped)
    int knightMoves[8][2] = {
        {2, 1}, {2, -1}, {-2, 1}, {-2, -1}, // Vertical L-shapes
        {1, 2},
        {1, -2},
        {-1, 2},
        {-1, -2} // Horizontal L-shapes
    };

    for (auto &move : knightMoves)
    {
        char new_col = col + move[1];
        int new_row = row + move[0];

        // Check if the new position is within the board boundaries
        if (new_col >= 'a' && new_col <= 'h' && new_row >= 1 && new_row <= 8)
        {
            Bitboard to_mask = 1ULL << ((new_row * 8) - (new_col - 97) - 1);

            // Check if the square is occupied by an opponent's knight
            if (board.pices[opponentColor][knight] & to_mask)
            {
                return true;
            }
        }
    }
    // Check for bishop/queen attacks (diagonals)
    if (is_square_under_attack_by_bishop_or_queen(board, col, row, opponentColor))
    {

        return true;
    }
    // Check for rook/queen attacks (horizontal or vertical)
    if (is_square_under_attack_by_rook_or_queen(board, col, row, opponentColor))
    {
        return true;
    }

    // Define all possible king moves (one square in any direction)
    int kingMoves[8][2] = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}, // Vertical and horizontal
        {1, 1},
        {1, -1},
        {-1, 1},
        {-1, -1} // Diagonal
    };

    for (auto &move : kingMoves)
    {
        char new_col = col + move[1];
        int new_row = row + move[0];

        // Check if the new position is within the board boundaries
        if (new_col >= 'a' && new_col <= 'h' && new_row >= 1 && new_row <= 8)
        {
            Bitboard to_mask = 1ULL << ((new_row * 8) - (new_col - 97) - 1);

            // Check if the square is occupied by an opponent's king
            if (board.pices[opponentColor][king] & to_mask)
            {
                return true;
            }
        }
    }

    return false;
}

vector<vector<string>> get_king_moves(Board &board, char col, int row)
{
    vector<vector<string>> possible_moves;
    vector<string> moveList;
    vector<string> captureList;
    Bitboard bit = 1;
    Bitboard from_mask = bit << ((row * 8) - (col - 97) - 1);
    int flag = board.ouccupancy[white] & from_mask ? white : black;
    int opponentColor = flag == white ? black : white;

    // Define all possible king moves (one square in any direction)
    int kingMoves[8][2] = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}, // Vertical and horizontal
        {1, 1},
        {1, -1},
        {-1, 1},
        {-1, -1} // Diagonal
    };
    board.ouccupancy[flag] &= ~from_mask;
    board.ouccupancy[both] &= ~from_mask;
    for (auto &move : kingMoves)
    {
        char new_col = col + move[1];
        int new_row = row + move[0];

        // Check if the new position is within the board boundaries
        if (new_col >= 'a' && new_col <= 'h' && new_row >= 1 && new_row <= 8)
        {
            Bitboard to_mask = bit << ((new_row * 8) - (new_col - 97) - 1);

            // Check if the square is not under attack
            if (!is_square_under_attack(board, new_col, new_row, opponentColor))
            {
                // Check if the square is occupied by an opponent's piece
                if (board.ouccupancy[opponentColor] & to_mask)
                {
                    // cout << "allowed king move is : " << (string(1, new_col) + to_string(new_row)) << endl;
                    captureList.push_back(string(1, new_col) + to_string(new_row));
                }
                // Check if the square is empty
                else if (!(board.ouccupancy[both] & to_mask))
                {
                    // cout << "allowed king move is : " << (string(1, new_col) + to_string(new_row)) << endl;
                    moveList.push_back(string(1, new_col) + to_string(new_row));
                }
            }
        }
    }
    board.ouccupancy[flag] |= from_mask;
    board.ouccupancy[both] |= from_mask;

    possible_moves.push_back(moveList);    // Normal moves
    possible_moves.push_back(captureList); // Capture moves
    return possible_moves;
}
