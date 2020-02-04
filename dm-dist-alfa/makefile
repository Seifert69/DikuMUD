CC = /usr/local/games/dikumud/gcc/gcc -B/usr/local/games/dikumud/gcc/
CFLAGS =
HEADERFILES = structs.h utils.h comm.h interpreter.h db.h
dmserver : comm.o act.comm.o act.informative.o act.movement.o act.obj1.o act.obj2.o act.offensive.o act.other.o act.social.o act.wizard.o handler.o db.o interpreter.o utility.o spec_assign.o shop.o limits.o mobact.o fight.o modify.o weather.o spell_parser.o spells1.o spells2.o reception.o constants.o spec_procs.o signals.o board.o mar_fiz_maz.o magic.o
comm.o : comm.c structs.h utils.h comm.h interpreter.h handler.h db.h
	$(CC) -c -g $(CFLAGS) comm.c
act.comm.o : act.comm.c structs.h utils.h comm.h interpreter.h handler.h \
  db.h spells.h
	$(CC) -c $(CFLAGS) act.comm.c
act.informative.o : act.informative.c structs.h utils.h comm.h \
  interpreter.h handler.h db.h spells.h limits.h
	$(CC) -c $(CFLAGS) act.informative.c
act.movement.o : act.movement.c structs.h utils.h comm.h interpreter.h \
  handler.h db.h spells.h
	$(CC) -c $(CFLAGS) act.movement.c
act.obj1.o : act.obj1.c structs.h utils.h comm.h interpreter.h handler.h \
  db.h spells.h
	$(CC) -c $(CFLAGS) act.obj1.c
act.obj2.o : act.obj2.c structs.h utils.h comm.h interpreter.h handler.h \
  db.h spells.h
	$(CC) -c $(CFLAGS) act.obj2.c
act.offensive.o : act.offensive.c structs.h utils.h comm.h interpreter.h \
  handler.h db.h spells.h
	$(CC) -c $(CFLAGS) act.offensive.c
act.other.o : act.other.c structs.h utils.h comm.h interpreter.h handler.h \
  db.h spells.h
	$(CC) -c $(CFLAGS) act.other.c
act.social.o : act.social.c structs.h utils.h comm.h interpreter.h \
  handler.h db.h spells.h
	$(CC) -c $(CFLAGS) act.social.c
act.wizard.o : act.wizard.c structs.h utils.h comm.h interpreter.h \
  handler.h db.h spells.h limits.h
	$(CC) -c $(CFLAGS) act.wizard.c
handler.o : handler.c structs.h utils.h db.h
	$(CC) -c -g $(CFLAGS) handler.c 
db.o : db.c structs.h utils.h db.h comm.h handler.h
	$(CC) -c $(CFLAGS) db.c
interpreter.o : interpreter.c structs.h comm.h interpreter.h db.h utils.h \
  limits.h
	$(CC) -c $(CFLAGS) interpreter.c 
utility.o : utility.c structs.h utils.h
	$(CC) -c $(CFLAGS) utility.c
spec_assign.o : spec_assign.c structs.h db.h
	$(CC) -c $(CFLAGS) spec_assign.c
spec_procs.o : spec_procs.c structs.h utils.h comm.h interpreter.h \
  handler.h db.h spells.h limits.h
	$(CC) -c $(CFLAGS) spec_procs.c
limits.o : limits.c limits.h structs.h utils.h spells.h comm.h
	$(CC) -c $(CFLAGS) limits.c
fight.o : fight.c structs.h utils.h comm.h handler.h interpreter.h db.h \
  spells.h
	$(CC) -c -g $(CFLAGS) fight.c
weather.o : weather.c structs.h utils.h comm.h handler.h interpreter.h db.h
	$(CC) -c $(CFLAGS) weather.c
shop.o : shop.c structs.h comm.h handler.h db.h interpreter.h utils.h
	$(CC) -c $(CFLAGS) shop.c
spells1.o : spells1.c structs.h utils.h comm.h db.h interpreter.h spells.h \
  handler.h
	$(CC) -c $(CFLAGS) spells1.c 
spells2.o : spells2.c structs.h utils.h comm.h db.h interpreter.h spells.h \
  handler.h
	$(CC) -c $(CFLAGS) spells2.c 
magic.o : spells2.c structs.h utils.h comm.h db.h interpreter.h spells.h \
  handler.h
	$(CC) -c $(CFLAGS) magic.c 
spell_parser.o : spell_parser.c structs.h utils.h comm.h db.h interpreter.h \
  spells.h handler.h
	$(CC) -c $(CFLAGS) spell_parser.c 
mobact.o : mobact.c utils.h structs.h db.h
	$(CC) -c $(CFLAGS) mobact.c
modify.o : modify.c structs.h utils.h interpreter.h handler.h db.h comm.h
	$(CC) -c $(CFLAGS) modify.c
reception.o : reception.c structs.h comm.h handler.h db.h interpreter.h \
  utils.h spells.h
	$(CC) -c $(CFLAGS) reception.c
constants.o : constants.c limits.h structs.h
	$(CC) -c $(CFLAGS) constants.c
board.o : board.c structs.h comm.h
	$(CC) -c $(CFLAGS) board.c
list.o : list.c structs.h
	$(CC) $(CFLAGS) -o list list.c
delplay.o : delplay.c structs.h
	$(CC) $(CFLAGS) -O -o delplay delplay.c
signals.o : signals.c utils.h
	$(CC) -c $(CFLAGS) signals.c
mar_fiz_maz.o : mar_fiz_maz.c structs.h utils.h comm.h interpreter.h \
  handler.h db.h spells.h limits.h
	$(CC) -c $(CFLAGS) mar_fiz_maz.c
dmserver : comm.c act.comm.c act.informative.c act.movement.c act.obj1.c act.obj2.c act.offensive.c act.other.c act.social.c act.wizard.c handler.c db.c interpreter.c utility.c spec_assign.c shop.c limits.c mobact.c fight.c modify.c weather.c spells1.c spells2.c spell_parser.c reception.c constants.c spec_procs.c signals.c board.c mar_fiz_maz.c magic.c
	$(CC) -o dmserver $(CFLAGS) comm.o act.comm.o act.informative.o act.movement.o act.obj1.o act.obj2.o act.offensive.o act.other.o act.social.o act.wizard.o handler.o db.o interpreter.o utility.o spec_assign.o shop.o limits.o mobact.o fight.o modify.o weather.o spells1.o spells2.o spell_parser.o reception.o constants.o spec_procs.o signals.o board.o mar_fiz_maz.o magic.o
