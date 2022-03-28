#include "framework.h"
#include <list>
#include "game.h"
#include "field.h"

Field Field::Add_changed_func(update_callback changed) {
	this->changed = changed;
	return (*this);
}
std::list<Field*> Field::Neighbours() {
	auto res = std::list<Field*>();
	if (cords.x - 1 >= 0 && cords.y - 1 >= 0)
		res.push_back(&(game->board[cords.x - 1][cords.y - 1]));
	if (cords.y - 1 >= 0)
		res.push_back(&(game->board[cords.x][cords.y - 1]));
	if (cords.x + 1 < game->board_size.x && cords.y - 1 >= 0)
		res.push_back(&(game->board[cords.x + 1][cords.y - 1]));

	if (cords.x - 1 >= 0)
		res.push_back(&(game->board[cords.x - 1][cords.y]));
	if (cords.x + 1 < game->board_size.x)
		res.push_back(&(game->board[cords.x + 1][cords.y]));

	if (cords.x - 1 >= 0 && cords.y + 1 < game->board_size.y)
		res.push_back(&(game->board[cords.x - 1][cords.y + 1]));
	if (cords.y + 1 < game->board_size.y)
		res.push_back(&(game->board[cords.x][cords.y + 1]));
	if (cords.x + 1 < game->board_size.x && cords.y + 1 < game->board_size.y)
		res.push_back(&(game->board[cords.x + 1][cords.y + 1]));

	return res;
}
void Field::Click() {
	if (!game->running) game->running = true;
	if (open || flagged) return;
	open = true;
	changed();
	if (bomb_neighbours_count == 0 && !is_bomb) {
		auto neigh = Neighbours();
		for (auto i = neigh.begin(); i != neigh.end(); ++i) if(!(*i)->open) (*i)->Click();
	}
	if (is_bomb) game->GameOver(false);
}
void Field::ToggleFlag() {
	if(!open && game->running){
		flagged = !flagged;
		changed();
		game->Flag(this);
	}
}
void Field::SetBomb() {
	if (is_bomb) return;
	is_bomb = true;
	auto neigh = Neighbours();
	for (auto i = neigh.begin(); i != neigh.end(); ++i) (*i)->bomb_neighbours_count++;
}

Game::Game(POINT board_size, int num_of_bombs, end_callback callback) {
	this->board_size = board_size;
	this->num_of_bombs = num_of_bombs;
	game_end_callback = callback;

	board = Make_board(board_size);
	Plant_bombs(num_of_bombs);
}
void Game::AddUpdateCallback(update_callback func) {
	changed = func;
}
void Game::GameOver(bool res) {
	running = false;
	game_end_callback(res);
}
Game::~Game() {
	delete[] board;
}

Field** Game::Make_board(POINT board_size) {
	Field** res = new Field * [board_size.x];
	for (int i = 0; i < board_size.x; i++) {
		res[i] = new Field[board_size.y];
		for (int j = 0; j < board_size.y; j++) {
			res[i][j] = Field(POINT({ i,j }), this);
		}
	}
	return res;
}
void Game::Plant_bombs(int num) {
	int planted = 0;
	while (planted < num) {
		int x = rand() % board_size.x;
		int y = rand() % board_size.y;

		if (!board[x][y].is_bomb) {
			board[x][y].SetBomb();
			planted++;
		}
	}
}
void Game::Flag(Field* field) {
	if (!running) return;
	if (field->flagged) {
		num_flagged++;
		if (field->is_bomb) correct_flagged++;
	}
	else {
		num_flagged--;
		if (field->is_bomb) correct_flagged--;
	}
	changed();

	if (correct_flagged == num_of_bombs) GameOver(true);
}
