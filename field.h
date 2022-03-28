#pragma once
#include "framework.h"
#include <list>
#include <functional>
#include "game.h"

class Game;
class Field {
public:
	bool is_bomb = false;
	bool flagged = false;
	bool open = false;
	int bomb_neighbours_count = 0;

	POINT cords;
	Game* game;

	typedef std::function<void()> update_callback;
	update_callback changed;

	Field() : cords({ -1, -1 }), game(nullptr) {}
	Field(POINT cords, Game* game) : cords(cords), game(game) {}

	Field Add_changed_func(update_callback changed);

	std::list<Field*> Neighbours();
	void Click();
	void ToggleFlag();
	void SetBomb();
};