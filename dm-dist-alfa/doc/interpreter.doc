/* ************************************************************************
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

1	 De to grundliggende rutiner i 'interpreter.c' er :

    	search_block og
   	argument_interpreter

1.1  	 SEARCH_BLOCK

	 Formaalet med denne rutine er, at finde en eventuel overensstemmelse
	mellem en (sandsynligvis af brugeren indtastet) streng og en tabel af
	strenge.

	 Rutinens hoved:

	int search_block(char *argument, int begin, int length, char **list, int mode)

	I denne rutine indeholder argument den streng der skal undersoeges, begin er den
	position, som det foerste tegn, i den del af argumenter der skal undersoeges,
	befinder sig paa.
	BEMAERK: Som normalt i c, vil det foerste tegn have position 0 !
	Length er laengden af den del af argumenter der skal undersoeges. (1 = et tegn)
	List er den tabel, hvori der skal ledes efter en forekomst af den del af argumentet
	der undersoeges.
	Mode er en 'binaer' variabel:
		TRUE ( != 0 ) : Der er kun tale om en forekomst, hvis laengden af
		  den streng man undersoeger, og laengden i 'list' er ens. ( nor != north ) 
		FALSE ( ==0 ) : Hvis alle tegn i den streng man undersoeger er 
		  identiske med tegn i tabellen, er der tale om er forekomst. ( nor == north )

	List skal vaere afsluttet med "/n" som sidste tegn.

	Den vaerdi, som funktionen returnerer er :
		-1 for ukendt,
		0 for tom streng.
		ellers nummeret i listen ( foerste = 1 !! )


1.2	 ARGUMENT_INTERPRETER

	 Formaalet med denne rutine er, at udlede to 'betydende' dele af 
	en streng.

	 Rutinens hoved:

	void argument_interpreter(char *argument, char *first_arg, char *second_arg)

	 Argument er den streng som de betydende dele skal udledes af.
	First_arg og second_arg indeholder de betydende dele.

	Definitionen paa betydende dele er:
	Ord forskellige fra :
	 in
	 from
	 with
	 the
	 on
	 at 
	 to

	Eksempel 1.

		Argument = "The iceBear wiTh the aXe"

		resultat:

		First_arg = "icebear"
		Second_arg = "axe"

	Som det ses vil store bogstaver blive returneret som smaa.

	Eksempel 2.

		Argument = "The ice bear with the axe"

		resultat:

		First_arg = "ice"
		Second_arg = "bear"

	Her er det meningen, at problemet gaar ud over brugeren.

	Eksempel 3.

		Argument = "The The"
		first_arg = ""
		second_arg = ""

	Eksempel 4.

		Argument = "Th icebear with the axe"
		first_arg = "th"
		second_arg = "icebear"

1.3	 Oevrige dele af interpreter

	 One_argument, som argument_interpreter, dog kun en streng der bliver 
	returneret.

	 Command_interpreter, her bliver hovedordet for kommandoen fundet, og
	den tilsvarende rutine kaldes.

	 Fill_word, er TRUE, hvis argumenter er er 'overfloedigt ord', som
         in
         from
         with
         the
         on
         at
         to
