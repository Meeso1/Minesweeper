#pragma once
#include "framework.h"
#include <list>
#include <functional>
#include "field.h"

class Field;
class Game {
public:
	POINT board_size;
	Field** board = nullptr;

	int num_of_bombs;
	int num_flagged = 0;
	int correct_flagged = 0;

	bool running = false;

	typedef std::function<void(bool)> end_callback;
	end_callback game_end_callback;

	typedef std::function<void()> update_callback;
	update_callback changed;

	Game(POINT board_size, int num_of_bombs, end_callback callback);
	~Game();
	void AddUpdateCallback(update_callback func);
	void GameOver(bool res);
	void Flag(Field* field);

private:
	Field** Make_board(POINT board_size);
	void Plant_bombs(int num);
};