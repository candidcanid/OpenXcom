
#include "../Engine/Action.h"
#include "../Engine/CrossPlatform.h"
#include "../Engine/Game.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Logger.h"

#include "../Engine/Options.h"
#include "../Engine/Palette.h"
#include "../Engine/Screen.h"
#include "../Engine/Script.h"
#include "../Engine/Sound.h"
#include "../Engine/Surface.h"
#include "../Engine/SurfaceSet.h"
#include "../Engine/Timer.h"
#include "../Geoscape/SelectMusicTrackState.h"
#include "../Interface/Bar.h"
#include "../Interface/BattlescapeButton.h"
#include "../Interface/Cursor.h"
#include "../Interface/NumberText.h"
#include "../Interface/Text.h"
#include "../Menu/CutsceneState.h"
#include "../Menu/LoadGameState.h"
#include "../Menu/PauseState.h"
#include "../Menu/SaveGameState.h"
#include "../Mod/AlienDeployment.h"
#include "../Mod/Armor.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleEnviroEffects.h"
#include "../Mod/RuleInterface.h"
#include "../Mod/RuleInventory.h"
#include "../Mod/RuleItem.h"
#include "../Mod/RuleSoldier.h"
#include "../Mod/RuleUfo.h"
#include "../Mod/RuleVideo.h"
#include "../Savegame/Base.h"
#include "../Savegame/BattleItem.h"
#include "../Savegame/BattleUnit.h"
#include "../Savegame/Craft.h"
#include "../Savegame/HitLog.h"
#include "../Savegame/SavedBattleGame.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Soldier.h"
#include "../Savegame/Tile.h"
#include "../Savegame/Ufo.h"
#include "../Ufopaedia/Ufopaedia.h"
#include "../fmath.h"
#include "../lodepng.h"
#include "AbortMissionState.h"
#include "ActionMenuState.h"
#include "AlienInventoryState.h"
#include "BattlescapeGame.h"
#include "BattlescapeGenerator.h"
#include "BriefingState.h"
#include "Camera.h"
#include "DebriefingState.h"
#include "ExtendedBattlescapeLinksState.h"
#include "InfoboxState.h"
#include "InventoryState.h"
#include "Map.h"
#include "MiniMapState.h"
#include "Pathfinding.h"
#include "Position.h"
#include "SDL_video.h"
#include "SkillMenuState.h"
#include "TileEngine.h"
#include "TurnDiaryState.h"
#include "UnitInfoState.h"
#include "WarningMessage.h"
#include <SDL_gfxPrimitives.h>
#include <algorithm>
#include <iomanip>
#include <iterator>
#include <sstream>

#include "Camera.h"
#include "TacOverlay.h"
#include "BattlescapeState.h"

namespace OpenXcom {

using std::vector;


class PawnPanel : public InteractiveSurface {
//    friend class UnitViewEntry;
//    friend class ShowHideButton;
    friend class PanelElement;

    class PanelElement : public InteractiveSurface {
        private:
            PawnPanel *_panel;

        public:
            PanelElement(PawnPanel *panel, int width, int height, int x = 0, int y = 0)
                : InteractiveSurface(width, height, x, y), _panel(panel) {};

            PawnPanel *getPanel() { return _panel; }
    };

    class ShowHideButton : public PanelElement {
        public:
            ShowHideButton(PawnPanel *panel, int width, int height, int x = 0, int y = 0)
                : PanelElement(panel, width, height, x, y) {}

            void mouseOver(OpenXcom::Action *action, OpenXcom::State *state) override {
                getPanel()->_overlay->setOverlayElementActive(true);
            }

            void mouseOut(OpenXcom::Action *action, OpenXcom::State *state) override {
                getPanel()->_overlay->setOverlayElementActive(false);
            }

            void mouseClick(OpenXcom::Action *action, OpenXcom::State *state) override {
                getPanel()->_active = !getPanel()->_active;
            }
    };

    class ScrollUp : public PanelElement {
        public:
            ScrollUp(PawnPanel *panel, int width, int height, int x = 0, int y = 0)
                : PanelElement(panel, width, height, x, y) {}

            void mouseClick(OpenXcom::Action *action, OpenXcom::State *state) override {
                if(getPanel()->_item_window_pos > 0)
                    getPanel()->_item_window_pos -= 1;
            }
    };

    class ScrollDown : public PanelElement {
        public:
            ScrollDown(PawnPanel *panel, int width, int height, int x = 0, int y = 0)
                : PanelElement(panel, width, height, x, y) {}

            void mouseClick(OpenXcom::Action *action, OpenXcom::State *state) override {
                if(getPanel()->_item_window_pos + getPanel()->_item_window_ndisplayed
                    < std::size(getPanel()->_active_units))
                    getPanel()->_item_window_pos += 1;
            }
    };

    class UnitViewEntry : public PanelElement {
        public:
            BattleUnit *_unit = nullptr;
            Text *_indicator;
            NumberText *_time_units;
            NumberText *_energy_units;
            Surface *_rank;
            Text *_unit_name;

            UnitViewEntry(PawnPanel *panel, int width, int height, int x = 0, int y = 0)
                : PanelElement(panel, width, height, x, y) {

                auto *state = getPanel()->_overlay->getBattleState();
                auto *game = state->getGame();

                _indicator = new Text(4, height, x, y);
                state->add(_indicator);

                _time_units = new NumberText(15, height, x, y);
                state->add(_time_units);
                // TODO: use .rul file
                _time_units->setColor(128);

                _energy_units = new NumberText(15, height, x + 15, y);
                state->add(_energy_units);
                // TODO: use .rul file
                _energy_units->setColor(64);

                _rank = new Surface(7, height, x + 24, y);
                state->add(_rank);

                _unit_name = new Text(width - 37, height, x + 34, y);
                state->add(_unit_name);

                const int default_color = game->getMod()
                    ->getInterface("battlescape")
                    ->getElement("textTooltip")->color;

                    _unit_name->setColor(default_color);
                    _unit_name->setHighContrast(true);
            }

            void setVisible(bool value) override {
                _indicator->setVisible(value);
                _time_units->setVisible(value);
                _energy_units->setVisible(value);
                _rank->setVisible(value);
                _unit_name->setVisible(value);
            }

            void mouseOver(OpenXcom::Action *action, OpenXcom::State *state) override {
                if(!getPanel()->_active || _unit == nullptr)
                    return;
                getPanel()->_overlay->setOverlayElementActive(true);
            }

            void mouseOut(OpenXcom::Action *action, OpenXcom::State *state) override {
                if(!getPanel()->_active || _unit == nullptr)
                    return;
                getPanel()->_overlay->setOverlayElementActive(false);
            }

            void mouseClick(Action *action, State *state) {
                if(!getPanel()->_active || _unit == nullptr)
                    return;


                     auto *game = getPanel()->_overlay
                         ->getBattleState()
                         ->getBattleGame();

                     game->getSave()
                         ->setSelectedUnit(_unit);
                     getPanel()->_overlay
                         ->getBattleState()
                         ->updateSoldierInfo(false);
                     game->cancelAllActions();
                     game->getCurrentAction()->actor = _unit;

                     getPanel()->_overlay->setOverlayElementActive(true);

                     auto *map = getPanel()->_overlay
                         ->getBattleState()
                         ->getMap();

                     map->getCamera()->centerOnPosition(_unit->getPosition());
                     game->setupCursor();
            }

            void animate() {
                if(_unit == nullptr) {
                    return;
                }

                auto *game = getPanel()->_overlay
                    ->getBattleState()
                    ->getGame();

                _time_units->setValue(_unit->getTimeUnits());
                _energy_units->setValue(_unit->getEnergy());
                {
                    std::ostringstream ss;
                    ss << _unit->getName(game->getLanguage(), false);
                    _unit_name->setText(ss.str());
                }
            }
    };

    // class ScrollBar : public InteractiveSurface {};

public:
    constexpr static unsigned int UNIT_ITEM_MAX = 32;

    TacOverlay *_overlay;
    vector<UnitViewEntry*> _view_items;
    ShowHideButton *_show_hide_button;
    ScrollUp *_up_button;
    ScrollDown *_down_button;

    bool _active = false;
    vector<BattleUnit*> _active_units;
    unsigned int _item_window_pos = 0;
    unsigned int _item_window_ndisplayed = 0;

    PawnPanel(TacOverlay *overlay, int width, int height, int x = 0, int y = 0)
        : InteractiveSurface(width, height, x, y), _overlay(overlay) {

        _view_items.reserve(128);
        _active_units.reserve(128);

        auto *state = overlay->getBattleState();
        state->add(this);

        const auto *game = state->getBattleGame();
        _show_hide_button = new ShowHideButton(this,
            18, 12, x + 3, height - 8
        );
        state->add(_show_hide_button);
        _show_hide_button->setVisible(true);

        _up_button = new ScrollUp(this,
            18, 6, x + 3 + 18 + 4, height - 8
        );
        state->add(_up_button);
        _up_button->setVisible(true);

        _down_button = new ScrollDown(this,
            18, 6, x + 3 + 18 + 4, height - 8 + 6
        );
        state->add(_down_button);
        _down_button->setVisible(true);

        constexpr int off = 10;
        int base = 0;
        // TODO: nicer math, cap at given height of screen
        while(std::size(_view_items) < UNIT_ITEM_MAX) {
            UnitViewEntry *entry = new UnitViewEntry(
                this,
                width - 8, 10,
                    x + 3, _show_hide_button->getY() - 14 - base
            );
            overlay->getBattleState()->add(entry);
            entry->setVisible(false);
            _view_items.push_back(entry);
            base += off;
        }

        _item_window_ndisplayed = std::size(_view_items);
    };

    void animate() {
        {
            SDL_Rect rect = {
                .x = 0,
                .y = 0,
                .w = static_cast<Uint16>(_show_hide_button->getWidth()),
                .h = static_cast<Uint16>(_show_hide_button->getHeight()),
            };

            _show_hide_button->drawRect(&rect, 112);
        }

        {
            SDL_Rect rect = {
                .x = 0,
                .y = 0,
                .w = static_cast<Uint16>(_up_button->getWidth()),
                .h = static_cast<Uint16>(_up_button->getHeight()),
            };
            _up_button->drawRect(&rect, 84);
        }

        {
            SDL_Rect rect = {
                .x = 0,
                .y = 0,
                .w = static_cast<Uint16>(_down_button->getWidth()),
                .h = static_cast<Uint16>(_down_button->getHeight()),
            };
            _down_button->drawRect(&rect, 45);
        }

        if (!_active) {
            for(auto entry = _view_items.begin(); entry != _view_items.end(); entry++) {
                (*entry)->setVisible(false);
            }
        } else {
            for (auto entry = _view_items.begin(); entry != _view_items.end(); entry++) {
                if((*entry)->_unit != nullptr) {
                    (*entry)->setVisible(true);
                    (*entry)->animate();
                } else {
                    (*entry)->setVisible(false);
                }
            }
        }
    }

    void think() override {
        auto game = _overlay->getBattleState()->getGame();
        auto *save = _overlay->getBattleState()->getBattleGame()->getSave();
        auto *units = save->getUnits();

        unsigned int prior_size = std::size(_active_units);
        _active_units.clear();

        for(auto entry = units->begin(); entry != units->end(); entry++) {
            auto *unit = *entry;
            bool player_owned = unit->getOriginalFaction() == FACTION_PLAYER || unit->isSummonedPlayerUnit();
            if (!player_owned || unit->isOut())
                continue;
            _active_units.push_back(unit);
        }

        // TODO: would be nicer if this didn't bump pos all the way back
        // to the top
        if(prior_size < std::size(_active_units))
            _item_window_pos = 0;

        unsigned int panel_idx = 0;
        auto it = std::vector<BattleUnit*>(
            _active_units.begin() + _item_window_pos,
            _active_units.end()
            );

        for(auto entry : it) {
            auto *pf = _view_items[panel_idx];
            pf->_unit = entry;
            panel_idx += 1;
            if(panel_idx >= UNIT_ITEM_MAX)
                break;
        }
    }

    void setActive(bool val) {
        _active = val;
    }
};

TacOverlay::TacOverlay(BattlescapeState *battle_state) {
    _battle_state = battle_state;
    _save = battle_state->getGame()->getSavedGame()->getSavedBattle();

    _battle_state->addUsingMouseCallback([this]() { return isUsingMouse(); });

    auto *icons = battle_state->getIcons();
    const int xpos_icons = icons->getX();
    const int h_screen = Options::baseYResolution;

    // we have enough space to create PawnPanel
    if(xpos_icons >= 180 && h_screen >= 240) {
        _pawn_panel = new PawnPanel(
            this,
            // TODO: determine if there's enough empty space next to icons
            //   and only enable if resolution > some threshold
            // start at lhs, use icon empty-space
            xpos_icons,
            h_screen - 10, // - 20 here is to ensure we don't clobber fps-counter
            0, 10
        );
        _pawn_panel->setActive(true);
    }
}

void TacOverlay::setOverlayElementActive(bool value) {
    assert(_battle_state != nullptr);
    _using_mouse_over = value;
    _using_scroll_wheel = value;

    // FIXME: use same handlers for disabling/enabling mouse wheel
    if(value)
        _battle_state->getMap()->getCamera()->disableMouseWheel();
    else
        _battle_state->getMap()->getCamera()->enableMouseWheel();
}

bool TacOverlay::isUsingMouse() {
    return _using_mouse_over;
}

void TacOverlay::animate() {
    if(_pawn_panel != nullptr)
        _pawn_panel->animate();
}

TacOverlay::~TacOverlay() {}}


