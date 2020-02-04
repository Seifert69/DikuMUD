/* ************************************************************************
*  file: utils.h, Utility module.                         Part of DIKUMUD *
*  Usage: Utility macros                                                  *
************************************************************************* */

#define TRUE  1

#define FALSE 0

#define LOWER(c) (((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c))

#define UPPER(c) (((c)>='a'  && (c) <= 'z') ? ((c)+('A'-'a')) : (c) )

/* Functions in utility.c                     */
/* #define MAX(a,b) (((a) > (b)) ? (a) : (b)) */
/* #define MIN(a,b) (((a) < (b)) ? (a) : (b)) */

#define ISNEWL(ch) ((ch) == '\n' || (ch) == '\r') 

#define IF_STR(st) ((st) ? (st) : "\0")

#define CAP(st)  (*(st) = UPPER(*(st)), st)

#define CREATE(result, type, number)  do {\
	if (!((result) = (type *) calloc ((number), sizeof(type))))\
		{ perror("malloc failure"); abort(); } } while(0)

#define RECREATE(result,type,number) do {\
  if (!((result) = (type *) realloc ((result), sizeof(type) * (number))))\
		{ perror("realloc failure"); abort(); } } while(0)

#define IS_SET(flag,bit)  ((flag) & (bit))

#define SWITCH(a,b) { (a) ^= (b); \
                      (b) ^= (a); \
                      (a) ^= (b); }

#define IS_AFFECTED(ch,skill) ( IS_SET((ch)->specials.affected_by, (skill)) )

#define IS_DARK(room)  (!world[room].light && IS_SET(world[room].room_flags, DARK))

#define IS_LIGHT(room)  (world[room].light || !IS_SET(world[room].room_flags, DARK))

#define SET_BIT(var,bit)  ((var) = (var) | (bit))

#define REMOVE_BIT(var,bit)  ((var) = (var) & ~(bit) )

/* Can subject see character "obj"? */
#define CAN_SEE(sub, obj)   ( ((!IS_AFFECTED((obj),AFF_INVISIBLE) ||        \
                                 IS_AFFECTED((sub),AFF_DETECT_INVISIBLE)) &&\
                                !IS_AFFECTED((sub),AFF_BLIND) ) &&            \
                                 IS_LIGHT(sub->in_room) )

#define GET_REQ(i) (i<2  ? "Awful" :(i<4  ? "Bad"     :(i<7  ? "Poor"      :\
(i<10 ? "Average" :(i<14 ? "Fair"    :(i<20 ? "Good"    :(i<24 ? "Very good" :\
        "Superb" )))))))

#define HSHR(ch) ((ch)->player.sex ?					\
	(((ch)->player.sex == 1) ? "his" : "her") : "its")

#define HSSH(ch) ((ch)->player.sex ?					\
	(((ch)->player.sex == 1) ? "he" : "she") : "it")

#define HMHR(ch) ((ch)->player.sex ? 					\
	(((ch)->player.sex == 1) ? "him" : "her") : "it")	

#define ANA(obj) (index("aeiouyAEIOUY", *(obj)->name) ? "An" : "A")

#define SANA(obj) (index("aeiouyAEIOUY", *(obj)->name) ? "an" : "a")

#define IS_NPC(ch)  (IS_SET((ch)->specials.act, ACT_ISNPC))

#define IS_MOB(ch)  (IS_SET((ch)->specials.act, ACT_ISNPC) && ((ch)->nr >-1))

#define GET_POS(ch)     ((ch)->specials.position)

#define GET_COND(ch, i) ((ch)->specials.conditions[(i)])

#define GET_NAME(ch)    ((ch)->player.name)

#define GET_TITLE(ch)   ((ch)->player.title)

#define GET_LEVEL(ch)   ((ch)->player.level)

#define GET_CLASS(ch)   ((ch)->player.class)

#define GET_HOME(ch)		((ch)->player.hometown)

#define GET_AGE(ch)     (age(ch).year)

#define GET_STR(ch)     ((ch)->tmpabilities.str)

#define GET_ADD(ch)     ((ch)->tmpabilities.str_add)

#define GET_DEX(ch)     ((ch)->tmpabilities.dex)

#define GET_INT(ch)     ((ch)->tmpabilities.intel)

#define GET_WIS(ch)     ((ch)->tmpabilities.wis)

#define GET_CON(ch)     ((ch)->tmpabilities.con)

#define STRENGTH_APPLY_INDEX(ch) \
        ( ((GET_ADD(ch)==0) || (GET_STR(ch) != 18)) ? GET_STR(ch) :\
          (GET_ADD(ch) <= 50) ? 26 :( \
          (GET_ADD(ch) <= 75) ? 27 :( \
          (GET_ADD(ch) <= 90) ? 28 :( \
          (GET_ADD(ch) <= 99) ? 29 :  30 ) ) )                   \
        )

#define GET_AC(ch)      ((ch)->points.armor)

#define GET_HIT(ch)     ((ch)->points.hit)

#define GET_MAX_HIT(ch) (hit_limit(ch))

#define GET_MOVE(ch)    ((ch)->points.move)

#define GET_MAX_MOVE(ch) (move_limit(ch))

#define GET_MANA(ch)    ((ch)->points.mana)

#define GET_MAX_MANA(ch) (mana_limit(ch))

#define GET_GOLD(ch)    ((ch)->points.gold)

#define GET_EXP(ch)     ((ch)->points.exp)

#define GET_HEIGHT(ch)  ((ch)->player.height)

#define GET_WEIGHT(ch)  ((ch)->player.weight)

#define GET_SEX(ch)     ((ch)->player.sex)

#define GET_HITROLL(ch) ((ch)->points.hitroll)

#define GET_DAMROLL(ch) ((ch)->points.damroll)

#define AWAKE(ch) (GET_POS(ch) > POSITION_SLEEPING)

#define WAIT_STATE(ch, cycle)  (((ch)->desc) ? (ch)->desc->wait = (cycle) : 0)




/* Object And Carry related macros */

#define CAN_SEE_OBJ(sub, obj)                                    \
	( (( !IS_SET((obj)->obj_flags.extra_flags, ITEM_INVISIBLE) ||   \
	     IS_AFFECTED((sub),AFF_DETECT_INVISIBLE) ) &&               \
	     !IS_AFFECTED((sub),AFF_BLIND)) && IS_LIGHT(sub->in_room) )

#define GET_ITEM_TYPE(obj) ((obj)->obj_flags.type_flag)

#define CAN_WEAR(obj, part) (IS_SET((obj)->obj_flags.wear_flags,part))

#define GET_OBJ_WEIGHT(obj) ((obj)->obj_flags.weight)

#define CAN_CARRY_W(ch) (str_app[STRENGTH_APPLY_INDEX(ch)].carry_w)

#define CAN_CARRY_N(ch) (5+GET_DEX(ch)/2+GET_LEVEL(ch)/2)

#define IS_CARRYING_W(ch) ((ch)->specials.carry_weight)

#define IS_CARRYING_N(ch) ((ch)->specials.carry_items)

#define CAN_CARRY_OBJ(ch,obj)  \
   (((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) <= CAN_CARRY_W(ch)) &&   \
    ((IS_CARRYING_N(ch) + 1) <= CAN_CARRY_N(ch)))

#define CAN_GET_OBJ(ch, obj)   \
   (CAN_WEAR((obj), ITEM_TAKE) && CAN_CARRY_OBJ((ch),(obj)) &&          \
    CAN_SEE_OBJ((ch),(obj)))

#define IS_OBJ_STAT(obj,stat) (IS_SET((obj)->obj_flags.extra_flags,stat))



/* char name/short_desc(for mobs) or someone?  */

#define PERS(ch, vict)   (                                          \
	CAN_SEE(vict, ch) ?						                                    \
	  (!IS_NPC(ch) ? (ch)->player.name : (ch)->player.short_descr) :	\
	  "someone")

#define OBJS(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
	(obj)->short_description  : "something")

#define OBJN(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
	fname((obj)->name) : "something")

#define OUTSIDE(ch) (!IS_SET(world[(ch)->in_room].room_flags,INDOORS))

#define EXIT(ch, door)  (world[(ch)->in_room].dir_option[door])

#define CAN_GO(ch, door) (EXIT(ch,door)  &&  (EXIT(ch,door)->to_room != NOWHERE) \
                          && !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))

#define GET_ALIGNMENT(ch) ((ch)->specials.alignment)

#define IS_GOOD(ch)    (GET_ALIGNMENT(ch) >= 350)
#define IS_EVIL(ch)    (GET_ALIGNMENT(ch) <= -350)
#define IS_NEUTRAL(ch) (!IS_GOOD(ch) && !IS_EVIL(ch))
