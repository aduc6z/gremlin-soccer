extern const char * GetSTRING(int string_type, int array_number);
extern	int 		GetSTATvalue(int string_type,int array_number,int stat_type);
extern	int		GetSPRITEnumber(int sprite_type,int number,int LastSpriteNO);
extern	int 		GetBUTTONnumber(int button_type, int button_index, int button_flags);
extern	menu 		menu_data[];
extern	label		menu_labels[];
extern	void		Initialise_rolling_demo();
extern	void		FadeSCREEN();
extern const char * GetText(int text_number);
