#ifndef LOM_MENU_INTERNAL_H
#define LOM_MENU_INTERNAL_H

#include "common.h"

#include "common/saved_game.h"
#include "common/vector.h"
#include "main/audio/akao.h"
#include "main/audio/game_audio.h"
#include "main/controller.h"
#include "main/display.h"
#include "overlays/field/field_numeric_sprites.h"
#include "overlays/field/field_object_state.h"
#include "overlays/field/field_overlay_handoff.h"
#include "overlays/field/field_player_records.h"
#include "overlays/field/field_portrait.h"
#include "overlays/field/field_sound.h"
#include "overlays/field/field_text.h"
#include "overlays/field/field_ui_text.h"
#include "overlays/menu/menu.h"

/* Encoded-name functions of FIELD, called while FIELD is resident. */
void field_copy_name(u8* destination, u8* source);
s32 field_name_byte_length(u8* name);

/* FIELD functions run from the menu frame loop and the character panel. */
s32 field_get_next_level_experience(s32 index);

/* Menu chrome CLUT ids produced by getClut for the rows uploaded in menu_upload_tim. */
#define MENU_CLUT_GRID_BASE 0x7C80
#define MENU_CLUT_GRID_ALT 0x7C81
#define MENU_CLUT_CORNER 0x7CCA
#define MENU_GRID_OT_INDEX 0x0F
#define MENU_GRID_ALT_CLUT_START 0x11
#define MENU_GRID_SPRITE_COUNT 0x1D
#define MENU_GRID_TEXTURE_WINDOW_SIZE 0xFF
#define MENU_GRID_TPAGE 5
#define MENU_SCROLL_ARROW_SIZE 16
#define MENU_SCROLL_ARROW_UV_UP 0x1080
#define MENU_SCROLL_ARROW_UV_DOWN 0x2080
#define MENU_SCROLL_ARROW_CLUT 0x7C86

#define MENU_SLOT_COUNT 4
#define MENU_SLOT_STATE_FREE 0
#define MENU_SLOT_STATE_OPENING 1
#define MENU_SLOT_STATE_OPEN 2
#define MENU_SLOT_STATE_CLOSING 3
#define MENU_SLOT_OT_INDEX_SHIFT 25
#define MENU_SLOT_OT_INDEX_CLEAR_MASK 0x01FFFFFF
#define MENU_WINDOW_TRANSITION_STEPS 12
/** @brief Last frame of the opening animation; a window created open starts here. */
#define MENU_WINDOW_OPEN_FRAME (MENU_WINDOW_TRANSITION_STEPS / 2 - 1)
#define MENU_WINDOW_MIN_WIDTH 0x20
#define MENU_WINDOW_MIN_HEIGHT 0x10

/** Size of the embedded TIM image block, including its block header. */
#define MENU_TIM_IMAGE_BLOCK_SIZE 0x800C

/* Packed menu-chrome UV origins: high byte V, low byte U. */
#define MENU_TW_CORNER_TL 0x70D0
#define MENU_TW_CORNER_TR 0x70D8
#define MENU_TW_CORNER_BL 0x78D0
#define MENU_TW_CORNER_BR 0x78D8
#define MENU_TW_EDGE_TOP 0x80D0
#define MENU_TW_EDGE_BOT 0x88D0
#define MENU_TW_EDGE_LEFT 0x90D0
#define MENU_TW_EDGE_RIGHT 0x90D8
#define MENU_TW_FILL 0xA0A0
#define MENU_WINDOW_CORNER_SIZE 8
#define MENU_WINDOW_FILL_TILE_SIZE 0x60
#define MENU_WINDOW_EDGE_TEXTURE_LONG_SIDE 16
#define MENU_WINDOW_EDGE_TEXTURE_SHORT_SIDE 8
#define MENU_LABEL_BUFFER_SIZE 16
/** Prefix range for encoded text units with one following byte. */
#define MENU_TEXT_PREFIX_FIRST 0x19U
#define MENU_TEXT_PREFIX_COUNT 7U

/*
 * Node / scroll / layout constants
 */
/** @brief Total number of nodes in g_menu_nodes[]. */
#define MENU_NODE_COUNT 0x2C
/** @brief MenuNode::u2 flag indicating that the node participates in layout. */
#define MENU_NODE_FLAG_ACTIVE 0x01
/** @brief MenuNode::u2 flag indicating that the node's children are visible. */
#define MENU_NODE_FLAG_EXPANDED 0x02
/** @brief Bits [14:8] of idx_nav.nav_x_packed: the 7-bit column (nav_x) field. */
#define MENU_NAV_X_MASK 0x7F00
/** @brief Clears bits [14:8] of idx_nav.nav_x_packed (inverse of MENU_NAV_X_MASK). */
#define MENU_NAV_X_CLEAR 0x80FF
/** @brief Bit 15 of idx_nav.nav_x_packed: bit 0 of the 9-bit nav cursor Y. */
#define MENU_NAV_Y0_BIT 0x8000
/** @brief Bit 15 of u8_u.nav_y_packed: bit 0 of the 9-bit layout Y position. */
#define MENU_LAYOUT_Y0_BIT 0x8000
/** @brief Maximum number of children per menu node. */
#define MENU_MAX_CHILDREN 4
/** @brief No layout interpolation remains; copy the target position directly. */
#define MENU_NODE_LAYOUT_IDLE 0
/** @brief Number of render updates used to interpolate a newly assigned row. */
#define MENU_NODE_LAYOUT_STEPS 4
/**
 * @brief Node ids: indexes into g_menu_nodes and the value of g_menu_scene_type.
 * @note Nodes 0-8 are the three party characters' nodes (see MENU_NODES_PER_CHARACTER); the comments give each node's title.
 */
typedef enum
{
    MENU_NODE_HERO_STATUS = 0x00,
    MENU_NODE_HERO_EQUIPMENT = 0x01,
    MENU_NODE_HERO_ABILITIES = 0x02, /**< "Ability/ST/Magic". */
    /** @brief Guest slot nodes when the guest is a hero-kind player (FIELD_PLAYER_KIND_HERO). */
    MENU_NODE_GUEST_HERO_STATUS = 0x03,
    MENU_NODE_GUEST_HERO_EQUIPMENT = 0x04,
    MENU_NODE_GUEST_HERO_ABILITIES = 0x05,
    /** @brief Guest slot nodes for any other guest: two status pages and abilities, no equipment. */
    MENU_NODE_GUEST_PARTNER_STATUS = 0x06,
    MENU_NODE_GUEST_PARTNER_ATTRIBUTES = 0x07,
    MENU_NODE_GUEST_PARTNER_ABILITIES = 0x08,
    /** @brief First node after the party characters' nodes. */
    MENU_NODE_CHARACTER_END = 0x09,
    MENU_NODE_PET_STATUS = 0x09,     /**< "Pet: Status" of the companion slot. */
    MENU_NODE_PET_ATTRIBUTES = 0x0A, /**< "Status": the pet's element, defense and armor values. */
    /** @brief Copies of MENU_NODE_PET_STATUS and MENU_NODE_PET_ATTRIBUTES that are never made active. */
    MENU_NODE_PET_STATUS_UNUSED = 0x0C,
    MENU_NODE_PET_ATTRIBUTES_UNUSED = 0x0D,
    MENU_NODE_GOLEM_STATUS = 0x0F,     /**< "Golem: Status" of the companion slot. */
    MENU_NODE_GOLEM_ATTRIBUTES = 0x10, /**< "Status": the golem's element, defense and armor values. */
    MENU_NODE_GOLEM_LOGIC_BLOCKS = 0x11,
    MENU_NODE_ITEMS = 0x12,
    MENU_NODE_WEAPONS = 0x13,
    MENU_NODE_WEAPON_STATUS = 0x14,      /**< "Weapons: Status". */
    MENU_NODE_WEAPON_ITEM_STATUS = 0x15, /**< "Items: Status" under Weapons. */
    MENU_NODE_ARMOR = 0x16,
    MENU_NODE_ARMOR_STATUS = 0x17,      /**< "Armor: Status". */
    MENU_NODE_ARMOR_ITEM_STATUS = 0x18, /**< "Items: Status" under Armor. */
    MENU_NODE_INSTRUMENTS = 0x19,
    MENU_NODE_INSTRUMENT_STATUS = 0x1A,      /**< "Instruments: Status". */
    MENU_NODE_INSTRUMENT_ITEM_STATUS = 0x1B, /**< "Items: Status" under Instruments. */
    MENU_NODE_ARTIFACTS = 0x1C,
    MENU_NODE_BACKFIELD = 0x1D, /**< "Backfield": the event list. */
    MENU_NODE_GAME_ENVIRONMENT = 0x1E,
    /** @brief "Key Configuration" of party slot 0. */
    MENU_NODE_BUTTONS_CHARACTER_0 = 0x1F,
    /** @brief Index of the "browse all items" root node; Circle navigates here. */
    MENU_NODE_BROWSE_ALL = 0x20,
    /** @brief "Key Configuration" of party slot 1. */
    MENU_NODE_BUTTONS_CHARACTER_1 = 0x2B
} MenuNodeId;
/** @brief Sentinel value meaning "none" for parent_idx, content_id, and child indices. */
#define MENU_NONE 0xFF
/** @brief Vertical spacing per node in scroll-position units (19 px). */
#define MENU_ROW_HEIGHT 0x13
/** @brief Full visible scroll-viewport height: 9 rows * MENU_ROW_HEIGHT (171 px). */
#define MENU_VIEW_HEIGHT 0xAB
/** @brief Height of one scroll-list row; each packed navigation entry stores its row times this. */
#define MENU_LIST_ROW_HEIGHT 0x10
#define MENU_LIST_ROW_SHIFT 4
/** @brief Bits [13:0] containing an item's vertical navigation position. */
#define MENU_ITEM_NAV_POSITION_MASK 0x3FFF
/** @brief Nine-bit mask for packed previous and next item indices. */
#define MENU_ITEM_NAV_INDEX_MASK 0x1FF
#define MENU_ITEM_NAV_PREVIOUS_SHIFT 14
#define MENU_ITEM_NAV_NEXT_SHIFT 23
#define MENU_LIST_COUNT_SHIFT 16
#define MENU_LIST_COUNT_MASK (MENU_ITEM_NAV_INDEX_MASK << MENU_LIST_COUNT_SHIFT)
/** @brief A packed MenuListNavigation word with its item count replaced by @p count. */
#define MENU_LIST_WITH_COUNT(packed, count) (((packed) & ~MENU_LIST_COUNT_MASK) | (((count) & MENU_ITEM_NAV_INDEX_MASK) << MENU_LIST_COUNT_SHIFT))
/** @brief Content items of the eight bindings on a button-assignment page; item MENU_BUTTON_ITEM_FIRST + n is action n. */
#define MENU_BUTTON_ITEM_FIRST 0x11
#define MENU_BUTTON_COUNT 8
/** @brief First menu subtype corresponding to an equipment slot. */
#define MENU_EQUIPMENT_SUBTYPE_BASE 7
/** @brief Menu subtype of equipment slot @p slot (FIELD_WEAPON_SLOT, then the armor slots). */
#define MENU_EQUIPMENT_SUBTYPE(slot) (MENU_EQUIPMENT_SUBTYPE_BASE + (slot))
/** @brief Menu subtype of the first Ability button; subtypes 1-2 select FieldCharacterRecord::info.actions.commands[0-1]. */
#define MENU_COMMAND_SUBTYPE_BASE 1
/** @brief Menu subtype of the first skill slot; subtypes 3-6 select FieldCharacterRecord::info.actions.skills[0-3]. */
#define MENU_SKILL_SUBTYPE_BASE 3
/** @brief Menu subtypes 11-13: help for the equipped weapon's derived.weapon.stats[2..4] attacks. */
#define MENU_WEAPON_ATTACK_SUBTYPE_BASE 11
/** @brief Menu subtype showing the pet's monster family help. */
#define MENU_MONSTER_FAMILY_SUBTYPE 14
/** @brief Menu subtype of the viewed item's name; confirming it opens the naming screen. */
#define MENU_RENAME_ITEM_SUBTYPE 15
/** @brief Menu subtypes 16-18: help for the viewed item's special_ids. */
#define MENU_ITEM_SPECIAL_SUBTYPE_BASE 16
/** @brief Menu subtypes 19-21: help for the viewed weapon's derived.weapon.stats[2..4] attacks. */
#define MENU_ITEM_ATTACK_SUBTYPE_BASE 19
/** @brief Menu subtypes showing the viewed instrument's magic, spirit and magic-shape help. */
#define MENU_INSTRUMENT_MAGIC_SUBTYPE 22
#define MENU_INSTRUMENT_SPIRIT_SUBTYPE 23
#define MENU_INSTRUMENT_SHAPE_SUBTYPE 24
/** @brief Menu subtype showing the companion ability help. */
#define MENU_COMPANION_ABILITY_SUBTYPE 25
/** @brief g_menu_active_item_category of the pages that do not list inventory records. */
#define MENU_ITEM_CATEGORY_NONE 3
/** @brief Node of party character @c slot: g_menu_nodes[slot * MENU_NODES_PER_CHARACTER + MENU_CHARACTER_NODE_*]. */
#define MENU_NODES_PER_CHARACTER 3
#define MENU_CHARACTER_NODE_STATUS 0
#define MENU_CHARACTER_NODE_EQUIPMENT 1
#define MENU_CHARACTER_NODE_ABILITIES 2
/** @brief Ordering-table entry of the content pages opened by menu_open_content_page. */
#define MENU_CONTENT_OT_INDEX 3
/** @brief Number of 0x40-byte records in the inventory item table. */
#define MENU_ITEM_TABLE_COUNT 100
/** @brief Byte offset of the inventory item table within g_saved_game_ctx. */
#define MENU_ITEM_TABLE_OFFSET OFFSETOF(SavedGameLayout, items)
/** @brief Size in bytes of one inventory/equipment item record. */
#define MENU_ITEM_RECORD_SIZE sizeof(FieldItemRecord)
/** @brief Size in bytes of one character's save/menu state block. */
#define MENU_CHARACTER_BLOCK_SIZE sizeof(FieldCharacterRecord)
/** @brief Offset of the character equipment-area header within its state block. */
#define MENU_CHARACTER_RECORD_OFFSET OFFSETOF(SavedGameLayout, characters)
/** Byte offset of the eight packed values displayed in the character panel. */
#define MENU_CHARACTER_PACKED_VALUES_OFFSET OFFSETOF(SavedGameLayout, characters[0].stats)
/** @brief Offset of the first equipped-item record within the equipment area. */
#define MENU_CHARACTER_ITEMS_OFFSET OFFSETOF(FieldCharacterRecord, equipment)
/** @brief Byte offset in g_saved_game_ctx of @p field of the first character; add slot * MENU_CHARACTER_BLOCK_SIZE for the others. */
#define MENU_CHARACTER_FIELD_OFFSET(field) (MENU_CHARACTER_RECORD_OFFSET + OFFSETOF(FieldCharacterRecord, field))
/** @brief Byte offset of the first character's info word (FieldCharacterRecord::info) within g_saved_game_ctx. */
#define MENU_CHARACTER_INFO_OFFSET MENU_CHARACTER_FIELD_OFFSET(info)
/** @brief Byte offset of the first character's first equipped-item record within g_saved_game_ctx. */
#define MENU_FIRST_EQUIPMENT_OFFSET (MENU_CHARACTER_RECORD_OFFSET + MENU_CHARACTER_ITEMS_OFFSET)
/** @brief Clears the packed previous-index field while preserving all other bits. */
#define MENU_ITEM_NAV_PREVIOUS_CLEAR_MASK 0xFF803FFF
/** @brief Clears the packed next-index field while preserving all other bits. */
#define MENU_ITEM_NAV_NEXT_CLEAR_MASK 0x007FFFFF
/** @brief The ability list reads SavedGameLayout::ability_bits one byte (eight abilities) at a time. */
#define MENU_ABILITY_BYTE_COUNT (FIELD_ABILITY_WORD_COUNT * 4)
#define MENU_ABILITIES_PER_BYTE 8
/** @brief Byte offset of the first character's skill slots (FieldCharacterRecord::info.actions.skills) in g_saved_game_ctx. */
#define MENU_SKILL_OFFSET (MENU_CHARACTER_RECORD_OFFSET + OFFSETOF(FieldCharacterRecord, info.actions.skills))
/**
 * @brief Byte offset in g_saved_game_ctx of the first character's battle-action byte for menu subtype 0.
 * @note The byte of subtype s is at this offset + s: commands for subtypes 1-2, skills for 3-6.
 */
#define MENU_ACTION_SLOT_OFFSET (MENU_CHARACTER_INFO_OFFSET + 1)
/** @brief Event List: words of event progress stages, events per word, and the stage of each event. */
#define MENU_EVENT_WORD_COUNT 16
#define MENU_EVENTS_PER_WORD 8
#define MENU_EVENT_STAGE_BITS 4
#define MENU_EVENT_STAGE_MASK 0xF
/** @brief Stage from which an event appears in the Event List. */
#define MENU_EVENT_STAGE_LISTED 2
/** @brief Stage from which a listed event shows the progress mark. */
#define MENU_EVENT_STAGE_MARKED 8
/** @brief Final stage of an event; it shows the completion mark. */
#define MENU_EVENT_STAGE_COMPLETE 0xF
/** @brief Minimum Y for g_content_cursor_y within the content sub-window (12 px). */
#define MENU_CURSOR_Y_MIN 0x0C
/** @brief Maximum Y for g_content_cursor_y within the content sub-window (163 px). */
#define MENU_CURSOR_Y_MAX 0xA3
/** @brief Horizontal inset from a node's navigation column to the content cursor. */
#define MENU_CONTENT_CURSOR_X_OFFSET 8
/** @brief Frames to suppress cursor highlight after opening a content view. */
#define MENU_CURSOR_REVEAL_DELAY 5
/** @brief Extracts the 9-bit screen X coordinate from MenuContentItem::packed_x. */
#define MENU_CONTENT_X_MASK 0x1FF
/** @brief Screen X of a content item. */
#define MENU_CONTENT_X(item) ((item)->packed_x & MENU_CONTENT_X_MASK)
/** @brief Viewport Y of a content item. */
#define MENU_CONTENT_Y(item) ((item)->y - MENU_CONTENT_VIEW_Y_OFFSET)
/** @brief Shift and mask for the three-bit text alignment field in MenuContentItem::packed_x. */
#define MENU_CONTENT_STYLE_SHIFT 9
#define MENU_CONTENT_STYLE_MASK 0x7
/** @brief FIELD_TEXT_ALIGN_* value a content item draws its text and numbers with. */
#define MENU_CONTENT_ALIGN(item) (((item)->packed_x >> MENU_CONTENT_STYLE_SHIFT) & MENU_CONTENT_STYLE_MASK)
/** @brief Shift of the high-nibble content type in MenuContentItem::packed_x. */
#define MENU_CONTENT_TYPE_SHIFT 12
/** @brief Converts a content item's Y coordinate to the viewport origin. */
#define MENU_CONTENT_VIEW_Y_OFFSET 8
/** @brief High-nibble mask selecting a content item's action class. */
#define MENU_CONTENT_ITEM_TYPE_MASK 0xF000
/** @brief Content item class that opens a nested submenu. */
#define MENU_CONTENT_ITEM_TYPE_SUBMENU 0x5000
/** @brief Content item class that dispatches a menu action code. */
#define MENU_CONTENT_ITEM_TYPE_ACTION 0xF000
/** @brief Packed bits marking a content item as the active selectable entry. */
#define MENU_CONTENT_ITEM_ACTIVE_MASK 0x0E00
/** @brief g_menu_redraw_state: navigation key pressed, scroll position adjusted. */
#define MENU_REDRAW_NAVIGATE 6
/** @brief g_menu_redraw_state: layout pass completed (position change or first run). */
#define MENU_REDRAW_LAYOUT 8
/** @brief g_saved_game_ctx->characters[FIELD_PARTY_GUEST].info.word bit enabling injected menu input. */
#define MENU_PAD_INJECT_ENABLED 0x80
/** @brief Enables vibration feedback in g_saved_game_ctx->options.word. */
#define MENU_OPTION_VIBRATION_ENABLED 0x01
/** @brief Stereo sound in g_saved_game_ctx->options.word; clear for mono. */
#define MENU_OPTION_STEREO 0x02
/** @brief Number of directional links stored by each content item. */
#define MENU_CONTENT_DIRECTION_COUNT 4
/** @brief Ordering-table entry used as the menu frame's list head. */
#define MENU_FRAME_OT_INDEX 13
/** @brief Vertical offset of the node-tree clipping region within a draw page. */
#define MENU_TREE_DRAW_Y_OFFSET 12
#define MENU_TREE_DRAW_X 15
#define MENU_TREE_DRAW_WIDTH 36
#define MENU_TREE_DRAW_HEIGHT 170

/* Type-view and fixed-address helpers. */
#define MENU_CONTROLLER_ACTUATORS ((MenuControllerActuatorState*)CONTROLLER_STATE_ADDRESS)

/* Rendering and window types. */

/**
 * @brief Container holding the menu TIM and its additional CLUT.
 *
 * The two offsets are stored in the asset header; the embedded TIM begins
 * immediately after the header, and its variable-sized image block follows
 * the first CLUT.
 */
typedef struct
{
    u32 entry_count;
    u32 tim_offset;
    u32 second_clut_offset;
    Tim tim;
    u8 image_block[MENU_TIM_IMAGE_BLOCK_SIZE];
    u16 second_clut[CLUT_ENTRY_COUNT];
} MenuTimAsset;

typedef struct
{
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} MenuRect;

typedef struct
{
    u16 x;
    u16 y;
    s16 w;
    s16 h;
} MenuRectU16;

/** @brief Known table indices in the menu text-resource directory. */
typedef enum
{
    MENU_TEXT_GENERAL = 0,
    MENU_TEXT_MESSAGES = 1,
    MENU_TEXT_MATERIAL_NAMES = 2, /**< Untranslated JP text in the US release. */
    MENU_TEXT_ABILITY_NAMES = 3,
    MENU_TEXT_ARTIFACT_HELP = 4,
    MENU_TEXT_ARTIFACT_NAMES = 5,
    MENU_TEXT_TECHNIQUE_HELP = 6,
    MENU_TEXT_TECHNIQUE_NAMES = 7,
    MENU_TEXT_SPIRIT_HELP = 8,
    MENU_TEXT_SPIRIT_NAMES = 9,
    MENU_TEXT_ITEM_HELP = 10,
    MENU_TEXT_ITEM_NAMES = 11,
    MENU_TEXT_MAGIC_SHAPE_HELP = 12,
    MENU_TEXT_MAGIC_SHAPE_NAMES = 13,
    MENU_TEXT_ABILITY_HELP = 14,
    MENU_TEXT_MAGIC_HELP = 15,
    MENU_TEXT_MAGIC_NAMES = 16,
    MENU_TEXT_MONSTER_FAMILIES = 17,
    MENU_TEXT_MONSTER_NAMES = 18,
    MENU_TEXT_MONSTER_TECHNIQUE_HELP = 19,
    MENU_TEXT_MONSTER_TECHNIQUE_NAMES = 20,
    MENU_TEXT_ITEM_SPECIAL_HELP = 21, /**< Blank in both releases. */
    MENU_TEXT_ITEM_SPECIAL_NAMES = 22,
    MENU_TEXT_WEAPON_ATTACK_HELP = 23,
    MENU_TEXT_WEAPON_ATTACK_NAMES = 24,
    MENU_TEXT_EQUIPMENT_TYPE_NAMES = 25,
    MENU_TEXT_EVENT_NAMES = 26,
    MENU_TEXT_GOLEM_PALETTE_NAMES = 27,
    MENU_TEXT_COMPANION_ABILITY_NAMES = 28,
    MENU_TEXT_COMPANION_ABILITY_HELP = 29,
    MENU_TEXT_GOLEM_GRID_SIZES = 30,
    MENU_TEXT_GOLEM_CLASS_NAMES = 31,
    MENU_TEXT_EVENT_HELP = 32,
    MENU_TEXT_ITEM_NAME_PARTS = 33,
    MENU_TEXT_TABLE_COUNT = 34
} MenuTextTable;

/** @brief Entries of the MENU_TEXT_MESSAGES table used by the item, equipment and skill windows. */
typedef enum
{
    MENU_MESSAGE_TECHNIQUES_REMOVED = 29,
    MENU_MESSAGE_QUICK_ATTACKS = 58,
    MENU_MESSAGE_POWER_ATTACKS = 59,
    MENU_MESSAGE_EVENT_LIST = 60,
    MENU_MESSAGE_TECHNIQUE = 61,
    MENU_MESSAGE_MAGIC = 62,
    MENU_MESSAGE_EQUIP = 63,
    MENU_MESSAGE_STATUS = 64,
    MENU_MESSAGE_AUTO_SELECT = 65,
    MENU_MESSAGE_TOTAL_AUTO = 66,
    MENU_MESSAGE_REMOVE = 67,
    MENU_MESSAGE_ITEM_STATUS = 68,
    MENU_MESSAGE_TRASH = 69,
    MENU_MESSAGE_CHANGE = 72,
    MENU_MESSAGE_CANCEL = 73,
    MENU_MESSAGE_CANNOT_REMOVE_EQUIPMENT = 86,
    MENU_MESSAGE_EQUIPPED_ITEM_SUFFIX = 90, /**< Appended to the equipped item's name; empty in the US release. */
    MENU_MESSAGE_NO_CONTROLLER_LINE1 = 101,
    MENU_MESSAGE_NO_CONTROLLER_LINE2 = 102
} MenuMessage;

/** @brief Content pages opened by menu_open_content_page. */
typedef enum
{
    MENU_PAGE_WEAPONS = 0,
    MENU_PAGE_ARMOR = 1,
    MENU_PAGE_INSTRUMENTS = 2,
    MENU_PAGE_EVENTS = 3,
    MENU_PAGE_ITEMS = 4,
    MENU_PAGE_ARTIFACTS = 5,
    MENU_PAGE_MESSAGE = 6,
    MENU_PAGE_TWO_LINE_MESSAGE = 7
} MenuContentPage;

/** @brief Codes in g_menu_content_action_codes, run when a MENU_CONTENT_KIND_ACTION item is confirmed. */
typedef enum
{
    MENU_ACTION_NONE = 0,
    /** @brief Rename the selected pet or golem in the naming screen. */
    MENU_ACTION_RENAME = 1,
    /** @brief First of the Game Environment option codes. */
    MENU_ACTION_OPTION_FIRST = 6,
    MENU_ACTION_SOUND_STEREO = 6,
    MENU_ACTION_SOUND_MONO = 7,
    MENU_ACTION_VIBRATION_ON = 8,
    MENU_ACTION_VIBRATION_OFF = 9,
    /** @brief Hand the guest to controller port 2. */
    MENU_ACTION_GUEST_PORT_2_ON = 10,
    MENU_ACTION_GUEST_PORT_2_OFF = 11,
    MENU_ACTION_OPTION_END = 12
} MenuActionCode;

/** @brief MENU_CONTENT_KIND_ACTION action_type values from 0xF0: the text table the item's help line comes from. */
typedef enum
{
    MENU_HELP_FIRST = 0xF0,
    MENU_HELP_MONSTER_FAMILY = 0xF0,
    MENU_HELP_ARTIFACT = 0xF1,
    MENU_HELP_MAGIC_SHAPE = 0xF2,
    MENU_HELP_SPIRIT = 0xF3,
    MENU_HELP_MAGIC = 0xF4,
    MENU_HELP_ITEM_SPECIAL = 0xF5,
    MENU_HELP_MONSTER_TECHNIQUE = 0xF6,
    MENU_HELP_WEAPON_ATTACK = 0xF7,
    /** @brief Magic or technique help, chosen by g_menu_help_skill_selection. */
    MENU_HELP_SKILL = 0xF8,
    MENU_HELP_TECHNIQUE = 0xF9,
    MENU_HELP_ABILITY = 0xFA,
    /** @brief Four values, all item help. */
    MENU_HELP_ITEM_FIRST = 0xFB
} MenuHelpSource;

/** @brief Icon ids of g_menu_icon_sprite_defs. */
#define MENU_ICON_NONE 0xFF
/** @brief "R1" page-switch hint. */
#define MENU_ICON_R1_HINT 0x1E
/** @brief Empty frame drawn for a clear flag or an empty equipment slot. */
#define MENU_ICON_EMPTY 0x21
/** @brief Eight spirit icons in FieldSpirit order. */
#define MENU_ICON_SPIRIT_FIRST 0x35
/** @brief Eight status-effect icons, one per status_flags bit. */
#define MENU_ICON_STATUS_FIRST 0x3D
/** @brief Weapon, armor and instrument icons, indexed by FIELD_ITEM_TYPE. */
#define MENU_ICON_WEAPON_FIRST 0x45
#define MENU_ICON_ARMOR_FIRST 0x50
#define MENU_ICON_INSTRUMENT_FIRST 0x5C
/** @brief "SET" marker on the chosen Game Environment option. */
#define MENU_ICON_SET 0x6A

/** @brief MENU_CONTENT_KIND_ICON action_type values: which icon the item shows. A _FIRST value starts a run of eight, one per flag bit. */
typedef enum
{
    /** @brief Values 0 and 1 show MENU_ICON_R1_HINT on some pages. */
    MENU_ICON_SELECT_R1_HINT_END = 0x02,
    /** @brief The armor's status_flags bits, as status icons. */
    MENU_ICON_SELECT_ARMOR_STATUS_FIRST = 0x02,
    /** @brief The armor's element_flags bits, as spirit icons. */
    MENU_ICON_SELECT_ARMOR_ELEMENT_FIRST = 0x0A,
    /** @brief status_flags of armor slots 1 to 3 combined, as status icons. */
    MENU_ICON_SELECT_ARMOR_SET_STATUS_FIRST = 0x12,
    /** @brief element_flags of armor slots 1 to 3 combined, through g_menu_element_icon_ids. */
    MENU_ICON_SELECT_ARMOR_SET_ELEMENT_FIRST = 0x1A,
    /** @brief The weapon's status_flags bits, as spirit icons. */
    MENU_ICON_SELECT_WEAPON_FLAG_FIRST = 0x22,
    /** @brief Eight values: the button bound to action n of the button-assignment page. */
    MENU_ICON_SELECT_BUTTON_FIRST = 0x2A,
    /** @brief Four values: the item in equipment slot n. */
    MENU_ICON_SELECT_SLOT_FIRST = 0x32,
    /** @brief g_menu_item_ptr's item icon. */
    MENU_ICON_SELECT_VIEWED_ITEM = 0x36,
    /** @brief Values up to MENU_ICON_SELECT_ACTION_LAST read g_menu_action_icon_ids. */
    MENU_ICON_SELECT_ACTION_FIRST = 0x37,
    MENU_ICON_SELECT_ACTION_LAST = 0x54,
    /** @brief MENU_ICON_SET markers of the Game Environment options. */
    MENU_ICON_SELECT_STEREO = 0x55,
    MENU_ICON_SELECT_MONO = 0x56,
    MENU_ICON_SELECT_VIBRATION_ON = 0x57,
    MENU_ICON_SELECT_VIBRATION_OFF = 0x58,
    MENU_ICON_SELECT_GUEST_PORT_2_ON = 0x59,
    MENU_ICON_SELECT_GUEST_PORT_2_OFF = 0x5A
} MenuIconSelector;

/** @brief Rows of the skill-slot window (menu_skill_action_callback). */
typedef enum
{
    MENU_SKILL_COMMAND_TECHNIQUE = 0,
    MENU_SKILL_COMMAND_MAGIC = 1,
    MENU_SKILL_COMMAND_REMOVE = 2,
    MENU_SKILL_COMMAND_STATUS = 3
} MenuSkillCommand;

/** @brief Rows of the equipment-slot window (menu_equipment_action_callback). */
typedef enum
{
    MENU_EQUIPMENT_COMMAND_EQUIP = 0,
    MENU_EQUIPMENT_COMMAND_AUTO_SELECT = 1,
    MENU_EQUIPMENT_COMMAND_TOTAL_AUTO = 2,
    MENU_EQUIPMENT_COMMAND_STATUS = 3,
    MENU_EQUIPMENT_COMMAND_REMOVE = 4
} MenuEquipmentCommand;

/**
 * @brief Content item kinds: the high nibble of MenuContentItem::packed_x.
 * @note Kinds 13 and 14 appear in the layout data, but no MENU code draws or dispatches them.
 */
typedef enum
{
    MENU_CONTENT_KIND_NONE = 0,
    /** @brief MENU_TEXT_MESSAGES entry action_type. */
    MENU_CONTENT_KIND_MESSAGE = 1,
    /** @brief Icon picked by action_type (a status, element, button or equipment icon). */
    MENU_CONTENT_KIND_ICON = 2,
    /** @brief MenuItemField of the item being viewed. */
    MENU_CONTENT_KIND_ITEM_FIELD = 3,
    /** @brief MenuPartyField of the selected party member. */
    MENU_CONTENT_KIND_PARTY_FIELD = 4,
    /** @brief Opens a submenu; see MENU_CONTENT_ITEM_TYPE_SUBMENU. */
    MENU_CONTENT_KIND_SUBMENU = 5,
    /** @brief MenuGameField: a value of the whole saved game. */
    MENU_CONTENT_KIND_GAME_FIELD = 6,
    /** @brief MenuAttackLabel heading of the attack lists. */
    MENU_CONTENT_KIND_ATTACK_LABEL = 7,
    /** @brief Dispatches a menu action code; see MENU_CONTENT_ITEM_TYPE_ACTION. */
    MENU_CONTENT_KIND_ACTION = 15
} MenuContentKind;

/**
 * @brief MENU_CONTENT_KIND_ITEM_FIELD values: fields of g_menu_item_ptr or of the weapon, armor and instrument being compared.
 * @note A _FIRST value starts a run of consecutive fields, one per slot, stat or value index.
 */
typedef enum
{
    MENU_ITEM_FIELD_NAME = 0x01,
    /** @brief Material name followed by the equipment type name. */
    MENU_ITEM_FIELD_TYPE_NAME = 0x02,
    /** @brief Eight fields, one per bonus_nibbles nibble. */
    MENU_ITEM_FIELD_BONUS_FIRST = 0x03,
    /** @brief Three fields, one per special_ids entry. */
    MENU_ITEM_FIELD_SPECIAL_FIRST = 0x0B,
    /** @brief Three fields: names of the weapon's derived.weapon.stats[2..4] attacks. */
    MENU_ITEM_FIELD_WEAPON_ATTACK_FIRST = 0x0E,
    MENU_ITEM_FIELD_WEAPON_POWER = 0x11,
    /** @brief The armor's derived.values[0]. */
    MENU_ITEM_FIELD_ARMOR_VALUE = 0x12,
    MENU_ITEM_FIELD_INSTRUMENT_MAGIC_NAME = 0x13,
    MENU_ITEM_FIELD_INSTRUMENT_SPIRIT_NAME = 0x14,
    MENU_ITEM_FIELD_INSTRUMENT_SHAPE_NAME = 0x15,
    MENU_ITEM_FIELD_INSTRUMENT_POWER = 0x16,
    /** @brief Four fields, one per equipment slot: size of the derived.values[0] change. */
    MENU_ITEM_FIELD_SLOT_CHANGE_FIRST = 0x17,
    /** @brief Four fields, one per equipment slot: sign of the derived.values[0] change. */
    MENU_ITEM_FIELD_SLOT_CHANGE_SIGN_FIRST = 0x1B,
    /** @brief Eight fields, one per stat_nibbles modifier: its sign. */
    MENU_ITEM_FIELD_MODIFIER_SIGN_FIRST = 0x1F,
    /** @brief Eight fields, one per stat_nibbles modifier: its size. */
    MENU_ITEM_FIELD_MODIFIER_FIRST = 0x27,
    /** @brief Eight fields, one per stat_nibbles modifier: sign of the change from the equipped item. */
    MENU_ITEM_FIELD_MODIFIER_CHANGE_SIGN_FIRST = 0x2F,
    /** @brief Eight fields, one per stat_nibbles modifier: size of the change from the equipped item. */
    MENU_ITEM_FIELD_MODIFIER_CHANGE_FIRST = 0x37,
    /** @brief Four fields, one per armor derived.values entry. */
    MENU_ITEM_FIELD_ARMOR_VALUE_FIRST = 0x3F,
    /** @brief Four fields, one per derived.values entry: sign of the armor total's change. */
    MENU_ITEM_FIELD_ARMOR_TOTAL_CHANGE_SIGN_FIRST = 0x43,
    /** @brief Four fields, one per derived.values entry: size of the armor total's change. */
    MENU_ITEM_FIELD_ARMOR_TOTAL_CHANGE_FIRST = 0x47
} MenuItemField;

/** @brief MENU_CONTENT_KIND_PARTY_FIELD values: fields of the party member in g_menu_char_slot. */
typedef enum
{
    MENU_PARTY_FIELD_PORTRAIT = 0x01,
    MENU_PARTY_FIELD_NAME = 0x02,
    MENU_PARTY_FIELD_LEVEL = 0x03,
    MENU_PARTY_FIELD_CURRENT_HP = 0x04,
    MENU_PARTY_FIELD_MAXIMUM_HP = 0x05,
    MENU_PARTY_FIELD_EXPERIENCE = 0x06,
    /** @brief Eight fields, one per effective stat. */
    MENU_PARTY_FIELD_STAT_FIRST = 0x07,
    MENU_PARTY_FIELD_WEAPON_NAME = 0x0F,
    /** @brief Three fields: names of the weapon's derived.weapon.stats[2..4] attacks. */
    MENU_PARTY_FIELD_WEAPON_ATTACK_FIRST = 0x10,
    MENU_PARTY_FIELD_WEAPON_POWER = 0x13,
    /** @brief Three fields: names of the armor in equipment slots 1 to 3. */
    MENU_PARTY_FIELD_ARMOR_NAME_FIRST = 0x14,
    /** @brief Two fields, one per command slot. */
    MENU_PARTY_FIELD_COMMAND_NAME_FIRST = 0x17,
    /** @brief Experience still needed for the next level. */
    MENU_PARTY_FIELD_NEXT_LEVEL_EXPERIENCE = 0x19,
    /** @brief Four fields, one per skill slot. */
    MENU_PARTY_FIELD_SKILL_NAME_FIRST = 0x1B,
    /** @brief Monster name of a pet (info.actions.variant). */
    MENU_PARTY_FIELD_MONSTER_NAME = 0x1F,
    /** @brief Four fields, one per derived.values entry summed over the armor slots. */
    MENU_PARTY_FIELD_ARMOR_TOTAL_FIRST = 0x20,
    MENU_PARTY_FIELD_WEAPON_POWER_CHANGE_SIGN = 0x3F,
    /** @brief Four fields, one per derived.values entry: sign of the armor total's change. */
    MENU_PARTY_FIELD_ARMOR_TOTAL_CHANGE_SIGN_FIRST = 0x40,
    MENU_PARTY_FIELD_WEAPON_POWER_CHANGE = 0x44,
    /** @brief Four fields, one per derived.values entry: size of the armor total's change. */
    MENU_PARTY_FIELD_ARMOR_TOTAL_CHANGE_FIRST = 0x45,
    MENU_PARTY_FIELD_COMPANION_ABILITY_NAME = 0x49,
    MENU_PARTY_FIELD_GOLEM_CLASS_NAME = 0x4A,
    MENU_PARTY_FIELD_GOLEM_GRID_SIZE = 0x4B,
    MENU_PARTY_FIELD_GOLEM_FAILURE_RATE = 0x4C,
    MENU_PARTY_FIELD_GOLEM_PALETTE_NAME = 0x4D,
    /** @brief Eight fields, one per bonus_nibbles nibble of the equipped weapon. */
    MENU_PARTY_FIELD_WEAPON_BONUS_FIRST = 0x4E
} MenuPartyField;

/** @brief MENU_CONTENT_KIND_GAME_FIELD values. */
typedef enum
{
    MENU_GAME_FIELD_MONEY = 1,
    MENU_GAME_FIELD_PLAY_TIME = 2
} MenuGameField;

/** @brief MENU_CONTENT_KIND_ATTACK_LABEL values; a guest of variant 5 or 8 shows neither. */
typedef enum
{
    MENU_ATTACK_LABEL_QUICK = 1,
    MENU_ATTACK_LABEL_POWER = 2
} MenuAttackLabel;

/** @brief Text-resource directory with byte offsets relative to the resource base. */
typedef struct
{
    u32 table_count;
    s32 table_offsets[MENU_TEXT_TABLE_COUNT];
} MenuTextResources;

/* Navigation and content types. */

/**
 * @brief One node in the hierarchical menu navigation tree.
 */
typedef struct
{
    u8 label_id;                /**< Index into the menu label string table. */
    u8 layout_frames_remaining; /**< Updates left to reach the target Y; zero snaps to the target. */
    union
    {
        u16 word; /**< Full 16-bit word: low byte = flags, high byte = parent_idx. */
        struct
        {
            u8 flags;      /**< Bit 0: node active/enabled in layout. Bit 1: node expanded (children shown). */
            u8 parent_idx; /**< Index of parent node in g_menu_nodes, or MENU_NONE (0xFF) for root nodes. */
        } s;
        struct
        {
            u16 active : 1;      /**< Node participates in layout (MENU_NODE_FLAG_ACTIVE). */
            u16 expanded : 1;    /**< Node's children are shown (MENU_NODE_FLAG_EXPANDED). */
            u16 anim_frames : 2; /**< Highlight animation frame counter. */
            u16 char_slot : 2;   /**< Party slot shown for this node; 3 keeps the current slot. */
            u16 depth : 2;       /**< Tree depth (0 = root row), used for indentation/style. */
            u16 parent_idx : 8;  /**< Same byte as s.parent_idx. */
        } bits;
    } u2;
    u8 icon_id;    /**< Sprite/icon definition passed to menu_emit_icon_sprite. */
    u8 content_id; /**< Passed to the content-open function; 0xFF = no content. */
    union
    {
        u16 nav_x_packed; /**< Raw word; high byte = nav_x, low byte = self_idx. */
        struct
        {
            u8 self_idx; /**< This node's own index in g_menu_nodes (used as content-table key). */
            u8 nav_x;    /**< Bits 0-6: nav cursor X = (nav_x & 0x7F) + 8. Bit 7: bit 0 of nav cursor Y. */
        } s;
    } idx_nav;
    union
    {
        u16 nav_y_packed; /**< Raw word; high byte = layout_x_y0, low byte = nav_y_hi. */
        struct
        {
            u8 nav_y_hi;    /**< Bits 1-8 of the 9-bit nav cursor Y: reconstruct as (nav_y_hi<<1)|(nav_x>>7). */
            u8 layout_x_y0; /**< Bits 6:0 = layout X; bit 7 = bit 0 of layout Y. */
        } s;
    } u8_u;
    union
    {
        u16 layout_child_packed; /**< Low byte: layout Y bits 8:1; high byte: first child index. */
        struct
        {
            u8 layout_y_hi;                 /**< Layout Y bits 8:1. */
            u8 children[MENU_MAX_CHILDREN]; /**< Child indices, terminated by MENU_NONE. */
            u8 unknown_0xf;
        } s;
    } layout;
} MenuNode;

/**
 * @brief Packed field view of a menu node's current and target positions.
 * The two coordinate pairs follow the content-table index without byte padding.
 */
typedef struct
{
    u8 label_id;
    u8 layout_frames_remaining;
    u16 flags_and_parent;
    u8 icon_id;
    u8 content_id;
    u32 self_idx : 8;
    u32 nav_x : 7;
    u32 nav_y : 9;
    u32 layout_x : 7;
    u32 layout_y : 9;
    u8 children[MENU_MAX_CHILDREN];
    u8 unknown_0xf;
} __attribute__((packed, aligned(2))) MenuNodeCoordinateView;

typedef struct
{
    u16 packed_x;   /**< Bottom 9 bits = X screen position; upper bits unknown. */
    u8 y;           /**< Y position; caller subtracts 8 when using as display offset. */
    u8 action_type; /**< Content/action selector interpreted by the active scene. */
    u8 params[4];   /**< Remaining content-specific parameter bytes. */
} MenuContentItem;

typedef struct
{
    u16 x : 9;
    u16 upper : 7;
    u8 y;
    u8 pad[5];
} MenuContentItemBits;

/**
 * @brief Packed source fields for one sprite emitted by @ref menu_build_grid.
 */
typedef struct
{
    u16 uv; /**< Packed texture coordinates: low byte U, high byte V. */
    u16 _pad2;
    u32 packed_xy; /**< Packed signed screen coordinates: low half X, high half Y. */
    u32 packed_wh; /**< Packed signed dimensions: low half width, high half height. */
} MenuGridSpriteDef;

/** Partial view of one controller port through its large-motor command. */
typedef struct
{
    u8 padding0[0x90];
    u8 small_motor_command;
    u8 padding91;
    u8 large_motor_command;
    u8 padding93[0x1B];
} MenuControllerActuatorPort;

/** Fixed two-port controller state at 0x801ED600. */
typedef struct
{
    MenuControllerActuatorPort ports[2];
} MenuControllerActuatorState;

typedef enum
{
    MENU_CURSOR_MODE_NODE_TREE = 0,
    MENU_CURSOR_MODE_CONTENT = 1,
    MENU_CURSOR_MODE_CONTENT_EXIT = 2,
} MenuCursorMode;

/* Text, list, and equipment types. */

/**
 * @brief Marker on the button binding held for a swap on a button-assignment page.
 */
typedef struct
{
    u8 x; /**< Screen X byte of the marker. */
    u8 y; /**< Screen Y byte of the marker. */
    u8 pad2;
    u8 selected_idx; /**< Content item index being held, or MENU_NONE. */
} MenuButtonSwapMarker;

typedef struct
{
    u8 u_coord; /**< Texture U coordinate. */
    u8 v_coord; /**< Texture V coordinate. */
    u8 w;       /**< Sprite width in pixels. */
    u8 h;       /**< Sprite height in pixels. */
} MenuIconSpriteInfo;

/** @brief Partial menu-slot view used by scrollable list callbacks. */
typedef struct
{
    u8 active; /**< MENU_SLOT_STATE_* lifecycle state. */
    u8 index;
    u8 anim_frame;
    u8 has_title;
    MenuListNavigation navigation;
    u16 base_x;     /**< Widget screen base X. */
    u16 base_y;     /**< Widget screen base Y. */
    s16 viewport_w; /**< Visible list width. */
    s16 viewport_h; /**< Visible list height; also determines the fast-scroll step. */
    u16 scroll_x;   /**< Current X scroll offset. */
    u16 scroll_y;   /**< Current Y scroll offset. */
    s16 target_x;   /**< X scroll interpolation target. */
    s16 target_y;   /**< Y scroll interpolation target. */
    u8 lerp_steps;  /**< Remaining interpolation steps. */
} ScrollListState;

/**
 * @brief One-byte selector for a stored character name or a resource-table name.
 * @note A packed value of MENU_NONE denotes an empty selection.
 */
typedef union
{
    u8 packed;
    struct
    {
        u32 index : 7;
        u32 stored_name : 1;
    } __attribute__((packed)) fields;
} MenuNameSelection;

void menu_init(void);
void menu_tick(RenderContext* render_ctx);
void menu_build_grid(RenderContext* render_ctx);
void menu_set_active_node(void);
void menu_snap_view_to_cursor(void);
void menu_reset_content_view(void);
void menu_init_item_nav_entries(s32 count);
void menu_update_active_slot(void);
void menu_update_slots(RenderContext* render_ctx);
void menu_draw_window_transition(RenderContext* render_ctx, MenuSlot* slot, s32 cursor_enable);
void menu_draw_window(MenuSlot* slot, RenderContext* render_ctx, MenuRect* rect, Vec2s* view_origin, s32 cursor_enable);
SPRT* menu_emit_corner(SPRT* sprite, u_long* ot_entry, s32 x, s32 y, u32 uv);
SPRT* menu_fill_window_interior(SPRT* sprite, u_long* ot_entry, const MenuRectU16* rect, u32 uv);
u_long* menu_build_h_edge(u_long* packet_cursor, u_long* ot_entry, const MenuRectU16* rect, s32 texture_origin);
u_long* menu_build_v_edge(u_long* packet_cursor, u_long* ot_entry, const MenuRectU16* rect, s32 texture_origin);
u8* menu_draw_frame(u8* packet_cursor, u_long* ot_entry, s32 frame_parity, s32 allow_input);
void* menu_draw_scene_content(void* packet_cursor, u_long* ot_entry);
void* menu_draw_active_node_cursor(void* packet_cursor, u_long* ot_entry, s32 draw_label);
void* menu_draw_content_cursor(void* prim_buf, u_long* ot, s32 draw_label);
s32 menu_handle_node_input(void);
u32 menu_step_item_selection(s32 step);

void menu_upload_tim(const TimUploadDestination* layout);
void* menu_draw_clamped_number(u_long* ot_entry, void* packet_cursor, s32 value, s32 text_color, Vec2s* origin, s32 flags);

void* menu_ability_list_callback(u_long* ot, ScrollListState* state, void* prim_buf, Vec2s* view_origin, int active);
void* menu_event_list_callback(u_long* ot, ScrollListState* state, void* prim_buf, Vec2s* view_origin, int active);
void* menu_item_list_callback(u_long* ot, ScrollListState* state, void* prim_buf, Vec2s* view_origin, int active);
void* menu_artifact_list_callback(u_long* ot, ScrollListState* state, void* prim_buf, Vec2s* view_origin, int active);
void* menu_message_callback(u_long* ot, ScrollListState* state, void* prim_buf, Vec2s* view_origin, int active);
void* menu_two_line_message_callback(u_long* ot, ScrollListState* state, void* prim_buf, Vec2s* view_origin, int active);
void* menu_inventory_list_callback(u_long* ot, ScrollListState* state, void* prim_buf, Vec2s* view_origin, s32 active);
void* menu_equipment_action_callback(u_long* ot, ScrollListState* state, void* prim_buf, Vec2s* view_origin, int active);
void* menu_skill_action_callback(u_long* ot, ScrollListState* state, void* prim_buf, Vec2s* view_origin, int active);

void* menu_emit_cursor(void* prim_buf, u_long* ot, s32 x, s32 y, s32 active);
void* menu_emit_draw_mode_primitive(DR_TPAGE* draw_mode, u_long* ot);
void* menu_emit_slot_scroll_arrows(SPRT* sprite, u_long* ot_entry, MenuSlot* slot);
void* menu_emit_tree_scroll_arrows(SPRT* sprite, u_long* ot_entry);

void* menu_emit_swap_marker(void*, u_long*, s16, s16);
s32 menu_item_differs_from_bare_hands(const FieldItemRecord*);

void* menu_draw_node_tree(void* prim_buf, u_long* ot);
void* menu_draw_node_recursive(s32 node_index, void* prim_buf, u_long* ot);

void* menu_emit_icon_sprite(void*, u_long*, s32, s32, s32, s32, s32, s32, s32);

void scroll_list_update_target(ScrollListState*, u32*);

void* menu_item_action_callback(u_long* ot, ScrollListState* state, void* prim_buf, Vec2s* view_origin, int active);
void* menu_equipment_compare_callback(u_long* ot, ScrollListState* state, void* prim_buf, Vec2s* view_origin, int active);
/*
 * MENU calls these FIELD functions (overlays/field/field_inventory.h) through
 * unprototyped declarations: one call passes field_compact_inventory an
 * argument it ignores.
 */
void field_copy_inventory_record();
void field_compact_inventory();
FieldItemRecord* field_find_free_inventory_record(void);
void* menu_special_technique_list_callback(u_long* ot, ScrollListState* state, void* prim_buf, Vec2s* view_origin, s32 active);
void menu_swap_item_records(FieldItemRecord*, FieldItemRecord*);

void menu_play_se(s32 sound_id, s32 volume);

s32 menu_stage_best_weapon(void);
s32 menu_stage_best_armor(void);

void* scroll_list_draw(void* prim_buf, u_long* ot, ScrollListState* state, u32* entries, Vec2s* view_origin, int active);

FieldItemRecord* menu_find_best_weapon(void);
FieldItemRecord* menu_find_best_armor(void);
void menu_open_content_page(u32 content_id);

s32 menu_build_inventory_nav_entries(s32 item_kind);
s32 menu_build_event_nav_entries(void);
s32 menu_build_item_nav_entries(void);
s32 menu_build_artifact_nav_entries(void);

/* Core menu state and shared assets. */

extern s32 g_playtime_vsync_origin;
/** @brief Default scene content count encoded as count - 1 (value 3). */
extern u8 g_menu_default_content_count_minus_one;
extern u8 g_menu_content_item_counts[];
/** @brief Base of the four default MenuContentItem descriptors used by scene -1. */
extern MenuContentItem g_menu_default_content_items;
/** @brief Icon of content items 0x37-0x54, indexed by action type (the stored entries start at index 0x37). */
extern u8 g_menu_action_icon_ids[];
/** @brief Armor element icons for content items 0x1A-0x21, indexed by action type. */
extern u8 g_menu_element_icon_ids[];
/** @brief Icon of each pad button on the button-assignment pages. */
extern u8 g_menu_button_icon_ids[];
/** @brief Active character's equipment[], as the armor columns and stat totals of the content pages read it (slots 1-3). */
extern FieldItemRecord* g_menu_shown_armor;
/** @brief Set with g_menu_shown_equipment; MENU never reads it. TODO: purpose unknown. */
extern FieldItemRecord* g_menu_shown_equipment_copy;
/** @brief Active character's equipment[], as the weapon and power comparison columns of the content pages read it. */
extern FieldItemRecord* g_menu_shown_equipment;

/** @brief Optional help/description string drawn below the active menu content. */
extern u8* g_menu_help_text;
/** @brief Current MenuCursorMode: node-tree, content, or content-exit cursor handling. */
extern s32 g_menu_cursor_mode;
/** @brief Set non-zero by a content callback to abort @ref menu_draw_window early. */
extern s32 g_menu_draw_early_out;
/** @brief Base address of the menu double-buffered DRAWENV array. */
extern RenderContext* g_menu_draw_buf_base;
/** @brief Display-enable countdown: set to 2 when the menu opens during a script, counted down once the script ends. */
extern s32 g_menu_display_delay;
/** @brief When non-zero, suppresses cursor highlight even on the active slot. */
extern s32 g_menu_suppress_cursor;
/** @brief Scene/language selector used in window title decoration layout switches. */
extern s32 g_menu_scene_type;

extern MenuNode g_menu_nodes[0x2C];
extern u8 g_menu_prev_node;
/** @brief Gate flag for menu_draw_content_cursor: 0 = draw empty slot, nonzero = full item render. */
extern s32 g_menu_content_ready;

/** @brief Comparison item addresses, or zero for an empty replacement. */
extern FieldItemRecord* g_item_slot_data[FIELD_EQUIPMENT_SLOT_COUNT];
/** @brief Nonzero for slots with a pending comparison change. */
extern u8 g_item_slot_flags[FIELD_EQUIPMENT_SLOT_COUNT];

/** @brief Pointer into g_saved_game_ctx item data for the current category; null = no items. */
extern FieldItemRecord* g_menu_item_ptr;
extern FieldItemRecord* g_menu_weapon_item;
extern FieldItemRecord* g_menu_armor_item;
extern FieldItemRecord* g_menu_instrument_item;
extern FieldItemRecord* g_menu_active_equipped_item;
extern FieldItemRecord* g_menu_saved_category0_item;
extern FieldItemRecord* g_menu_saved_category1_item;
/** @brief Packed circular navigation entries for item sub-pages. */
extern s32 g_menu_item_nav_entries[];
extern FieldItemRecord* g_menu_equipment_base;
/** @brief Current interpolated vertical scroll position of the node tree. */
extern s32 g_menu_content_height;
extern s32 g_menu_scroll_pos;
extern s32 g_menu_redraw_state;
extern s32 g_menu_active_node;
/** @brief Equipped-item address saved when opening an item-action submenu. */
extern FieldItemRecord* g_menu_saved_equipment_item;
extern u8 g_menu_init_content_id;

/** @brief Type-conflict bits of the other equipped items; the weapon and armor pages grey out items that conflict. */
extern s8 g_menu_equipment_conflicts;
/** @brief Node index for the companion character's stat page (0x2B = companion present, 0xFF = none). */
extern u8 g_menu_companion_node;

/** @brief Number of nodes in the linear navigation list. */
extern s32 g_menu_nav_count;
/** @brief Visible node IDs in drawing and navigation order. */
extern s32 g_menu_nav_nodes[];
/** @brief Y display coordinate for the content viewport origin. */
extern s32 g_content_view_y;
/** @brief Set to 1 to request an overlay/scene load at end of this input frame. */
extern s32 g_menu_load_request;
/** @brief Transition/result code paired with g_menu_load_request. */
extern s32 g_menu_transition_code;
/** @brief X pixel position of the content cursor within the content window. */
extern s32 g_content_cursor_x;
/** @brief Index of the item found by hit-test, or -1 if none. */
extern s32 g_menu_hit_item_idx;
/** @brief X display coordinate for the content viewport origin. */
extern s32 g_content_view_x;
/** @brief Y pixel position of the content cursor within the content window; clamped to [0xC, 0xA3]. */
extern s32 g_content_cursor_y;
/** @brief Default X/Y origin for the content viewport when no item hit-test position is available. */
extern struct
{
    s16 x;
    s16 y;
} g_menu_default_view_pos;
/** @brief Per-node table of MenuContentItem arrays, indexed by node.idx_nav.s.self_idx; NULL = no cursor data. */
extern MenuContentItem* g_menu_content_table[];
extern s32 g_menu_layout_end;

/* Content and action state. */

/** @brief Active character slot index: 0 = char slot 0 (node 0x1F), 1 = char slot 1 (node 0x2B). */
extern s32 g_menu_char_slot;

/** @brief Total page count for the current sub-menu view; g_script_repeat_last cycles in [0, g_menu_page_count-1]. */
extern s32 g_menu_page_count;
/** @brief Action sub-type of the most recently confirmed 0x5000 menu item; routes downstream handlers. */
extern s32 g_menu_active_subtype;
/** @brief Storage for packed circular navigation entries used by scroll-list pages. */
extern u32 g_menu_scroll_nav_entries[];

/** @brief Party-sort selection marker; selected_idx is MENU_NONE when inactive. */
extern MenuButtonSwapMarker g_menu_button_swap_marker;
extern u8 g_menu_content_group_ids[];
/** @brief Action code for each content group and one of its eight encoded item slots. */
extern u8 g_menu_content_action_codes[][8];
/** @brief First glyph/string pointer used by confirmation and status messages. */
extern void* g_menu_message_line1;
/** @brief Optional second glyph/string pointer used by two-line messages. */
extern void* g_menu_message_line2;

/** @brief Conflict bits of each weapon type and each armor type (FIELD data, indexed by FIELD_ITEM_TYPE). */
extern u8 g_field_weapon_type_conflicts[];
extern u8 g_field_armor_type_conflicts[];

/** @brief Signed stat modifier for each four-bit modifier index (index 4 is 0; range -10 to +20). */
extern s8 g_field_stat_modifier_values[];

/**
 * @brief Skill-slot selection choosing the help text of help source MENU_HELP_SKILL: MENU_NONE for none, bit 0x80 for magic.
 * @note TODO: MENU never writes it.
 */
extern s32 g_menu_help_skill_selection;

/* Rendering assets. */
extern MenuIconSpriteInfo g_menu_icon_sprite_defs[];
extern u8 g_menu_icon_clut_codes[];

/** @brief Three-frame cursor icon-id sequence (0x6B, 0x6C, 0x6D). */
extern u8 g_menu_cursor_icon_ids[];

/* Inventory and equipment state. */
/** @brief Window title (MENU_TEXT_MESSAGES entry) of item-list and weapon pages, indexed by page id; the page-label tables overlap. */
extern u8 g_menu_list_page_label_ids[];
/** @brief Window title of the technique pages, indexed by page id. */
extern u8 g_menu_technique_page_label_ids[];
extern u8 g_menu_item_description_buffer[];
extern s32 g_menu_inventory_index;
extern s32 g_menu_active_item_category;
/** @brief Selected equipment row awaiting a swap, or MENU_NONE. */
extern s32 g_menu_pending_item_row;

/* Equipment-action assets. */
/** @brief Window title of the armor page, indexed by page id. */
extern u8 g_menu_armor_page_label_ids[];
/** @brief FIELD's BareHands item record; an empty weapon slot holds a copy of it. */
extern FieldItemRecord g_field_bare_hands_item;
/** @brief Item info word of g_field_bare_hands_item. */
extern u32 g_field_bare_hands_info;

/** @brief Mark every menu window slot as free. */
static inline void menu_clear_slots(void)
{
    s32 slot_index;

    for (slot_index = MENU_SLOT_COUNT - 1; slot_index >= 0; slot_index--)
    {
        g_menu_slots[slot_index].active = 0;
    }
}

/**
 * @brief Link the first @p count entries of g_menu_item_nav_entries into a circular list.
 * @param count Number of navigation entries to initialize.
 */
static inline void menu_relink_item_nav(s32 count)
{
    s32 entry_index;
    s32 previous_index;
    s32 link;
    s32 entry_with_position;
    s32 entry_with_previous;
    s32 next_index;
    s32 has_next;

    for (entry_index = 0; entry_index < count; entry_index++)
    {
        s32 packed_entry = g_menu_item_nav_entries[entry_index];

        previous_index = count - 1;
        link = packed_entry & ~MENU_ITEM_NAV_POSITION_MASK;
        link = link | ((entry_index * MENU_LIST_ROW_HEIGHT) & MENU_ITEM_NAV_POSITION_MASK);
        entry_with_position = link;
        g_menu_item_nav_entries[entry_index] = entry_with_position;

        if ((entry_index - 1) >= 0)
        {
            previous_index = entry_index - 1;
        }

        entry_with_previous = entry_with_position & MENU_ITEM_NAV_PREVIOUS_CLEAR_MASK;
        entry_with_previous = entry_with_previous | ((previous_index & MENU_ITEM_NAV_INDEX_MASK) << MENU_ITEM_NAV_PREVIOUS_SHIFT);
        g_menu_item_nav_entries[entry_index] = entry_with_previous;
        next_index = entry_index + 1;
        has_next = next_index < count;
        link = 0;

        if (has_next != 0)
        {
            link = next_index;
        }

        g_menu_item_nav_entries[entry_index] = (entry_with_previous & MENU_ITEM_NAV_NEXT_CLEAR_MASK) | (link << MENU_ITEM_NAV_NEXT_SHIFT);
    }
}

/** @brief Build circular navigation links for a two-entry item list. */
static inline void menu_relink_pair(void)
{
    s32 index;
    s32 previous_index;
    s32 next_index;
    s32 has_next;
    s32 link;
    s32 entry_with_previous;
    index = 0;

    while (index < 2)
    {
        s32 entry = g_menu_item_nav_entries[index];
        s32 entry_with_position;
        previous_index = 1;
        link = entry & ~MENU_ITEM_NAV_POSITION_MASK;
        link = link | ((index * MENU_LIST_ROW_HEIGHT) & MENU_ITEM_NAV_POSITION_MASK);
        entry_with_position = link;
        g_menu_item_nav_entries[index] = entry_with_position;

        if ((index - 1) >= 0)
        {
            previous_index = index - 1;
        }

        entry_with_previous = entry_with_position & MENU_ITEM_NAV_PREVIOUS_CLEAR_MASK;
        entry_with_previous = entry_with_previous | ((previous_index & MENU_ITEM_NAV_INDEX_MASK) << MENU_ITEM_NAV_PREVIOUS_SHIFT);
        g_menu_item_nav_entries[index] = entry_with_previous;
        next_index = index + 1;
        has_next = next_index < 2;
        link = 0;

        if (has_next != 0)
        {
            link = next_index;
        }

        g_menu_item_nav_entries[index] = (entry_with_previous & MENU_ITEM_NAV_NEXT_CLEAR_MASK) | (link << MENU_ITEM_NAV_NEXT_SHIFT);
        index = next_index;
    }
}

/**
 * @brief Locate an equipped item belonging to the active character.
 * @param context Shared save-state context.
 * @param equipment_slot Index within the character's equipment array.
 * @return Address of the equipped item record.
 */
static inline FieldItemRecord* menu_equipped_item(SavedGameLayout* context, s32 equipment_slot)
{
    s32 offset = equipment_slot * MENU_ITEM_RECORD_SIZE;
    offset += g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE;
    return (FieldItemRecord*)((u8*)context + offset + MENU_CHARACTER_RECORD_OFFSET + MENU_CHARACTER_ITEMS_OFFSET);
}

/**
 * @brief Locate one character record in the shared context.
 * @param context Shared save-state context.
 * @param character_slot Character record index.
 * @return Character record, including its equipped items.
 */
static inline FieldCharacterRecord* menu_character_record(SavedGameLayout* context, s32 character_slot)
{
    return (FieldCharacterRecord*)((u8*)context + (character_slot * MENU_CHARACTER_BLOCK_SIZE + MENU_CHARACTER_RECORD_OFFSET));
}

/**
 * @brief Draw a content label at the menu's fixed help-text position.
 * @param packet_cursor First free primitive byte.
 * @param ot Ordering-table entry receiving the text primitives.
 * @param text Encoded text to draw.
 * @return First free primitive byte after the label.
 */
static inline void* menu_emit_content_label(void* packet_cursor, u_long* ot, u8* text)
{
    return field_draw_text(packet_cursor, ot, text, 1, 0xA0, 0xCA, FIELD_TEXT_ALIGN_CENTER);
}

/**
 * @brief Concatenate two encoded menu strings and write their terminator.
 * @param destination Buffer large enough for both strings and the terminator.
 * @param first First null-terminated encoded string.
 * @param second Second null-terminated encoded string.
 * @note Bytes 0x19-0x1F introduce a second byte, which may be zero.
 */
static inline void menu_copy_encoded_pair(u8* destination, const u8* first, const u8* second)
{
    while (1)
    {
        u8 character = *first;

        if (character == 0)
        {
            break;
        }

        if ((u8)(character - MENU_TEXT_PREFIX_FIRST) < MENU_TEXT_PREFIX_COUNT)
        {
            *destination++ = character;
            first++;
            *destination++ = *first++;
        }
        else
        {
            *destination++ = character;
            first++;
        }
    }

    while (1)
    {
        u8 character = *second;

        if (character == 0)
        {
            break;
        }

        if ((u8)(character - MENU_TEXT_PREFIX_FIRST) < MENU_TEXT_PREFIX_COUNT)
        {
            *destination++ = character;
            second++;
            *destination++ = *second++;
        }
        else
        {
            *destination++ = character;
            second++;
        }
    }

    *destination = 0;
}

/**
 * @brief Return one text table from the active resource directory.
 * @param table Resource-directory index.
 * @return Table base used by its relative string offsets.
 */
static inline u8* menu_text_table_base(s32 table)
{
    u8* resources = (u8*)g_menu_state_ptr;
    return resources + ((MenuTextResources*)resources)->table_offsets[table];
}

/**
 * @brief Resolve one table-relative string offset.
 * @param base Text table beginning with halfword offsets.
 * @param index String-offset index.
 * @return Encoded string at that offset.
 */
static inline u8* menu_text_entry(void* base, s32 index)
{
    return (u8*)base + ((u16*)base)[index];
}

/**
 * @brief Show a one-line message window in place of the open windows.
 * @param message MENU_TEXT_MESSAGES entry to show.
 */
static inline void menu_show_message(s32 message)
{
    g_menu_message_line1 = menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), message);
    menu_clear_slots();
    menu_open_content_page(MENU_PAGE_MESSAGE);
}

/**
 * @brief Locate image data at a word-aligned offset in the upload buffer.
 * @param buffer Word-aligned base of the image upload buffer.
 * @param byte_offset Byte offset, rounded down to a four-byte boundary.
 * @return Source words for LoadImage.
 */
static inline u_long* menu_image_upload_source(void* buffer, s32 byte_offset)
{
    return (u_long*)(((byte_offset >> 2) << 2) + (uintptr_t)buffer);
}

/**
 * @brief Convert a packed menu icon palette code to a Psy-Q CLUT id.
 * @param code Palette column in the low nibble, row above VRAM_CLUT_Y in the high nibble.
 * @return The CLUT id.
 */
static inline u16 menu_icon_clut(u8 code)
{
    return getClut((code & 0xF) << 4, (code >> 4) + VRAM_CLUT_Y);
}

/**
 * @brief Read the seven-bit navigation column from a packed position word.
 * @param packed Position word containing the column in bits 14:8.
 * @return Navigation column.
 */
static inline s32 menu_nav_x(u16 packed)
{
    return (packed >> 8) & (MENU_NAV_X_MASK >> 8);
}

/**
 * @brief Address of the FIELD UI string an offset entry points at.
 * @param entry Offset entry, one of the g_field_ui_text_* symbols.
 * @param index FIELD_UI_TEXT_* index of @p entry.
 * @return The entry's string.
 */
static inline u8* menu_shared_text_entry(const FieldUiTextOffset* entry, s32 index)
{
    uintptr_t string_page_base = (entry->high << 8) + (uintptr_t)(entry - index);
    return (u8*)(entry->low + string_page_base);
}

/**
 * @brief Copy the label for a value's sign and append its terminator.
 * @param buffer Destination text buffer.
 * @param value Value whose sign picks the label (zero counts as positive).
 */
static inline void menu_copy_sign_label(u8* buffer, s32 value)
{
    u8* write_cursor = buffer;
    u8* source;

    if (value >= 0)
    {
        source = menu_shared_text_entry(&g_field_ui_text_plus, FIELD_UI_TEXT_PLUS);
        field_copy_name(write_cursor, source);
    }
    else
    {
        source = menu_shared_text_entry(&g_field_ui_text_minus, FIELD_UI_TEXT_MINUS);
        field_copy_name(write_cursor, source);
    }

    write_cursor += field_name_byte_length(source);
    *write_cursor = 0;
}

#endif
