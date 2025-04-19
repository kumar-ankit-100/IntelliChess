#include "common_header.h"

int is_piece_present_in_square(Board &board, char col, int row)
{
    Bitboard bit = 1;
    Bitboard to_mask = bit << ((row * 8) - (col - 97) - 1);
    return board.ouccupancy[both] & to_mask ? 1 : 0;
}

int is_this_pawn_first_move(Board &board, char move, int flag)
{
    // if flag = 1 then white makes pawn move so start row will be 2 for white
    // and 7 for black
    Bitboard bit = 1;
    if (flag)
    {
        Bitboard mask = bit << ((2 * 8) - (move - 97) - 1);
        return board.pices[white][pawn] & mask ? 1 : 0;
    }
    else
    {
        Bitboard mask = bit << ((7 * 8) - (move - 97) - 1);
        return board.pices[black][pawn] & mask ? 1 : 0;
    }
}

int is_pawn_move_valid(Board &board, string &moves, int flag)
{
    // check if pawn try to capture other pices
    if (abs((moves[0] - 97) - (moves[2] - 97)) == 1 && abs((moves[1] - 48) - (moves[3] - 48)) == 1)
    {
        if (is_piece_present_in_square(board, moves[2], moves[3]))
        {
            // cout<<"pice cannot be captured since piece is not available"<<endl;

            return 5;
        }
    }

    // check if pawn not move  in straight
    if (moves[0] != moves[2])
        return 0;

    // check that pawn move up for white and down for black
    if (flag)
    {
        if (((moves[3] - 48) - (moves[1] - 48)) < 0)
            return 0;
    }
    else
    {
        if (((moves[1] - 48) - (moves[3] - 48)) < 0)
            return 0;
    }

    // check if pawn move more than 2  steps in first and more than
    //  1 step in second move onward
    if (is_this_pawn_first_move(board, moves[0], flag))
    {
        if (!(abs(moves[1] - moves[3]) == 2 || abs(moves[1] - moves[3]) == 1))
        {

            return 0;
        }
        else
        {
            if (abs(moves[1] - moves[3]) == 2)
            {
                // flag =1 for white so we need to add 1 or to check wether pices is present or not
                // flag = 0 for black so we need to subtract
                if (flag)
                    return is_piece_present_in_square(board, moves[0], (moves[1] - 48 + 1)) || is_piece_present_in_square(board, moves[0], (moves[1] - 48 + 2)) ? 0 : 1;
                else
                    return is_piece_present_in_square(board, moves[0], (moves[1] - 48 - 1)) || is_piece_present_in_square(board, moves[0], (moves[1] - 48 - 2)) ? 0 : 1;
            }
            else
            {
                if (flag)
                    return is_piece_present_in_square(board, moves[0], (moves[1] - 48 + 1)) ? 0 : 1;
                else
                    return is_piece_present_in_square(board, moves[0], (moves[1] - 48 - 1)) ? 0 : 1;
            }
        }
    }
    else
    {
        if (!(abs(moves[1] - moves[3]) == 1))
        {
            return 0;
        }
        else
        {
            if (flag)
                return is_piece_present_in_square(board, moves[0], (moves[1] - 48 + 1)) ? 0 : 1;
            else
                return is_piece_present_in_square(board, moves[0], (moves[1] - 48 - 1)) ? 0 : 1;
        }
    }
    return 1;
}

// int capture_pices_by_pawn(Board &board, string &moves)
// {
// }

vector<vector<string>> get_pawn_moves(Board &board, char col, int row)
{
    vector<vector<string>> possible_moves;
    vector<string> emptyMove;
    Bitboard bit = 1;
    // cout << "bit is shift by " << ((row * 8) - (col - 97) - 1) << endl;
    Bitboard from_mask = bit << ((row * 8) - (col - 97) - 1);
    int flag = board.ouccupancy[white] & from_mask ? 1 : 0;
    // Calculate forward move (one step forward)
    if (flag) // white pawn
    {
        // cout << "white pawn" << endl;
        if (!is_piece_present_in_square(board, col, row + 1))
        {
            emptyMove.push_back(string(1, col) + to_string(row + 1));
            if (row == 2 && !is_piece_present_in_square(board, col, row + 2))
            {
                emptyMove.push_back(string(1, col) + to_string(row + 2));
            }
        }
    }
    else // black pawn
    {
        // cout << "black pawn" << endl;
        if (!is_piece_present_in_square(board, col, row - 1))
        {
            emptyMove.push_back(string(1, col) + to_string(row - 1));
            if (row == 7 && !is_piece_present_in_square(board, col, row - 2))
            {
                emptyMove.push_back(string(1, col) + to_string(row - 2));
            }
        }
    }
    possible_moves.push_back(emptyMove);
    vector<string> captureMove;

    // Calculate diagonal captures
    int opponentColor = flag ? black : white;
    if (flag) // white pawn captures
    {
        // cout << "white pawn and capture piece available" << endl;

        if (col > 'a' && is_piece_present_in_square(board, col - 1, row + 1))
        {
            Bitboard to_mask = bit << (((row + 1) * 8) - ((col - 1) - 97) - 1);
            if (board.ouccupancy[opponentColor] & to_mask)
                captureMove.push_back(string(1, col - 1) + to_string(row + 1));
        }
        if (col < 'h' && is_piece_present_in_square(board, col + 1, row + 1))
        {
            Bitboard to_mask = bit << (((row + 1) * 8) - ((col + 1) - 97) - 1);
            if (board.ouccupancy[opponentColor] & to_mask)
                captureMove.push_back(string(1, col + 1) + to_string(row + 1));
        }
    }
    else // black pawn captures
    {

        // cout << "black pawn and capture piece available" << endl;
        if (col > 'a' && is_piece_present_in_square(board, col - 1, row - 1))
        {
            Bitboard to_mask = bit << (((row - 1) * 8) - ((col - 1) - 97) - 1);
            if (board.ouccupancy[opponentColor] & to_mask)
                captureMove.push_back(string(1, col - 1) + to_string(row - 1));
        }
        if (col < 'h' && is_piece_present_in_square(board, col + 1, row - 1))
        {
            Bitboard to_mask = bit << (((row - 1) * 8) - ((col + 1) - 97) - 1);
            if (board.ouccupancy[opponentColor] & to_mask)
                captureMove.push_back(string(1, col + 1) + to_string(row - 1));
        }
    }
    possible_moves.push_back(captureMove);
    return possible_moves;
}