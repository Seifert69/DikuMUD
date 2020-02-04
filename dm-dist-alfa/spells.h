/* ************************************************************************
*  file: spells.h , Implementation of magic spells.       Part of DIKUMUD *
*  Usage : Spells                                                         *
************************************************************************* */

#define MAX_BUF_LENGTH              240

#define TYPE_UNDEFINED               -1
#define SPELL_RESERVED_DBC            0  /* SKILL NUMBER ZERO */
#define SPELL_ARMOR                   1 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_TELEPORT                2 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BLESS                   3 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BLINDNESS               4 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BURNING_HANDS           5 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CALL_LIGHTNING          6 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CHARM_PERSON            7 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CHILL_TOUCH             8 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CLONE                   9 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_COLOUR_SPRAY           10 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CONTROL_WEATHER        11 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CREATE_FOOD            12 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CREATE_WATER           13 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_BLIND             14 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_CRITIC            15 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_LIGHT             16 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURSE                  17 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_EVIL            18 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_INVISIBLE       19 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_MAGIC           20 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_POISON          21 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DISPEL_EVIL            22 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_EARTHQUAKE             23 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ENCHANT_WEAPON         24 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ENERGY_DRAIN           25 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_FIREBALL               26 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_HARM                   27 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_HEAL                   28 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_INVISIBLE              29 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_LIGHTNING_BOLT         30 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_LOCATE_OBJECT          31 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_MAGIC_MISSILE          32 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_POISON                 33 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_PROTECT_FROM_EVIL      34 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_REMOVE_CURSE           35 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SANCTUARY              36 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SHOCKING_GRASP         37 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SLEEP                  38 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_STRENGTH               39 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SUMMON                 40 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_VENTRILOQUATE          41 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_WORD_OF_RECALL         42 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_REMOVE_POISON          43 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SENSE_LIFE             44 /* Reserved Skill[] DO NOT CHANGE */

/* types of attacks and skills must NOT use same numbers as spells! */

#define SKILL_SNEAK                  45 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_HIDE                   46 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_STEAL                  47 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_BACKSTAB               48 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_PICK_LOCK              49 /* Reserved Skill[] DO NOT CHANGE */

#define SKILL_KICK                   50 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_BASH                   51 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_RESCUE                 52 /* MAXIMUM SKILL NUMBER  */

/* END OF SKILL RESERVED "NO TOUCH" NUMBERS */


/* NEW SPELLS are to be inserted here */
#define SPELL_IDENTIFY               53
#define SPELL_ANIMATE_DEAD           54 /* EXAMPLE */
#define SPELL_FEAR                   55 /* EXAMPLE */
#define SPELL_FIRE_BREATH            56
#define SPELL_GAS_BREATH             57
#define SPELL_FROST_BREATH           58
#define SPELL_ACID_BREATH            59
#define SPELL_LIGHTNING_BREATH       60


#define TYPE_HIT                     100
#define TYPE_BLUDGEON                101
#define TYPE_PIERCE                  102
#define TYPE_SLASH                   103
#define TYPE_WHIP                    104 /* EXAMPLE */
#define TYPE_CLAW                    105  /* NO MESSAGES WRITTEN YET! */
#define TYPE_BITE                    106  /* NO MESSAGES WRITTEN YET! */
#define TYPE_STING                   107  /* NO MESSAGES WRITTEN YET! */
#define TYPE_CRUSH                   108  /* NO MESSAGES WRITTEN YET! */



#define TYPE_SUFFERING               200
/* More anything but spells and weapontypes can be insterted here! */





#define MAX_TYPES 70

#define SAVING_PARA   0
#define SAVING_ROD    1
#define SAVING_PETRI  2
#define SAVING_BREATH 3
#define SAVING_SPELL  4


#define MAX_SPL_LIST	127


#define TAR_IGNORE        1
#define TAR_CHAR_ROOM     2
#define TAR_CHAR_WORLD    4
#define TAR_FIGHT_SELF    8
#define TAR_FIGHT_VICT   16
#define TAR_SELF_ONLY    32 /* Only a check, use with ei. TAR_CHAR_ROOM */
#define TAR_SELF_NONO    64 /* Only a check, use with ei. TAR_CHAR_ROOM */
#define TAR_OBJ_INV     128
#define TAR_OBJ_ROOM    256
#define TAR_OBJ_WORLD   512
#define TAR_OBJ_EQUIP  1024

struct spell_info_type
{
	void (*spell_pointer) (byte level, struct char_data *ch, char *arg, int type,
	                       struct char_data *tar_ch, struct obj_data *tar_obj);
	byte minimum_position;  /* Position for caster 						 */
	ubyte min_usesmana;     /* Amount of mana used by a spell	 */
	byte beats;             /* Heartbeats until ready for next */

	byte min_level_cleric;  /* Level required for cleric       */
	byte min_level_magic;   /* Level required for magic user   */
	sh_int targets;         /* See below for use with TAR_XXX  */
};

/* Possible Targets:

   bit 0 : IGNORE TARGET
   bit 1 : PC/NPC in room
   bit 2 : PC/NPC in world
   bit 3 : Object held
   bit 4 : Object in inventory
   bit 5 : Object in room
   bit 6 : Object in world
   bit 7 : If fighting, and no argument, select tar_char as self
   bit 8 : If fighting, and no argument, select tar_char as victim (fighting)
   bit 9 : If no argument, select self, if argument check that it IS self.

*/

#define SPELL_TYPE_SPELL   0
#define SPELL_TYPE_POTION  1
#define SPELL_TYPE_WAND    2
#define SPELL_TYPE_STAFF   3
#define SPELL_TYPE_SCROLL  4


/* Attacktypes with grammar */

struct attack_hit_type {
  char *singular;
  char *plural;
};

