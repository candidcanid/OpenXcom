#pragma once

#include "../Engine/State.h"
#include "../Engine/InteractiveSurface.h"

namespace OpenXcom {

class SavedBattleGame;
class BattlescapeGame;
class BattlescapeState;

class PawnPanel;

class TacOverlay {
private:
	friend class PawnPanel;

	BattlescapeState *_battle_state;
	SavedBattleGame *_save;
	PawnPanel *_pawn_panel;

    bool _using_mouse_over = false;
    bool _using_scroll_wheel = false;
public:
	inline TacOverlay():
		_battle_state(nullptr),
		_save(nullptr),
		_pawn_panel(nullptr) {};
	TacOverlay(BattlescapeState *battle_state);
	~TacOverlay();
	void animate();
	void setOverlayElementActive(bool value);
	BattlescapeState *getBattleState() { return _battle_state; }
    bool isUsingMouse();
};

}