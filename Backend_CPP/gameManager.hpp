#include <unordered_map>
#include <queue>
#include <memory>
#include <random>

#include "common_header.h"

using namespace std;

class Game
{
public:
    Game(int white_socket, int black_socket)
        : white_socket(white_socket), black_socket(black_socket), current_turn(white) ,previousMove("")
    {
        initializeBoard(nboard);
    }

    int getOpponentSocket(int socket_id)
    {
        return socket_id == white_socket ? black_socket : white_socket;
    }

    bool isPlayerTurn(int socket_id)
    {
        return (current_turn == white && socket_id == white_socket) ||
               (current_turn == black && socket_id == black_socket);
    }

    void switchTurn()
    {
        current_turn = (current_turn == white) ? black : white;
    }
    void updatePreviousMove(const string &move)
    {
        previousMove=move;
    }

    Board nboard;
    int white_socket;
    int black_socket;
    int current_turn;
    string whitePlayer="";
    string blackPlayer = "";
    string previousMove;
    string whiteMessage="";
    string blackMessage="";
    bool isAIGame = false;
};

class GameManager
{
public:
    GameManager() : random_engine(std::random_device{}()) {}


    pair<bool, string> handleMove(int socket_id, const std::string &move)
    {
        auto game_it = findGameBySocket(socket_id);
        if (game_it == games.end())
        {

            return make_pair(false, ""); // Player not in a game
        }

        Game &game = *game_it->second;
        if (!game.isPlayerTurn(socket_id))
        {
            return make_pair(false, ""); // Not this player's turn
        }

        // Validate and apply the move
        // if (moveGeneration(move, game.nboard))
        // {
        json request = json::parse(move);
        string purpose = request["purpose"];
        if (purpose == "generateMove")
        {
            return make_pair(true, handle_request(move, game.nboard,socket_id));
        }
        else if (purpose == "updateBoard")
        {
            cout<<"switching turn"<<endl;
            game.switchTurn();
            game.updatePreviousMove(request["position"]);
            return make_pair(true, handle_request(move, game.nboard,socket_id));
        }

        // Send updated nboard state to both players
        // sendBoardUpdate(game.white_socket, game.nboard);
        // sendBoardUpdate(game.black_socket, game.nboard);
        // }

        return make_pair(false, ""); // Invalid move
    }

    std::unordered_map<int, std::shared_ptr<Game>>::iterator findGameBySocket(int socket_id)
    {
        return std::find_if(games.begin(), games.end(),
                            [socket_id](const auto &pair)
                            {
                                return pair.second->white_socket == socket_id || pair.second->black_socket == socket_id;
                            });
    }

    void makeGame(int socket1, int socket2)
    {
        int player1 = socket1;
        int player2 = socket2;
        int white_socket;
        int black_socket;
        if(socket2==-1){
             white_socket=socket1;
             black_socket=-1;

        }
        else{

        // Randomly assign colors
        std::uniform_int_distribution<> dis(0, 1);
         white_socket = dis(random_engine) == 0 ? player1 : player2;
         black_socket = white_socket == player1 ? player2 : player1;
        cout << "white socket : " << white_socket << " " << "black socket" << black_socket << endl;
        }

        auto game = std::make_shared<Game>(white_socket, black_socket);
        if(socket2==-1){
            game->blackPlayer="Ai Bot";
            game->isAIGame=true;
        }
        int game_id = next_game_id++;
        games[game_id] = game;
    }

    std::unordered_map<int, std::shared_ptr<Game>> games;
private:
    int next_game_id = 1;
    std::mt19937 random_engine;
};

// Global GameManager instance
GameManager game_manager;