typedef enum { _GBORDER =0, _GXOR, _GPSET } _SCREENTYPES;EXTERN void Plotxy(int X0, int Y0);EXTERN void _moveto(int x, int y);EXTERN void _lineto(int x, int y);EXTERN void _setcolor(int col);EXTERN int _getcolor(void);EXTERN int _getplotaction(void);EXTERN void _setplotaction(int flag);EXTERN void _ellipse(int flag, int x, int y, int xr, int yr);EXTERN void delay(int delay);EXTERN void _setpixel(int x,int y);EXTERN void _outgtext(char *str);EXTERN void _outtext(char *str);EXTERN void _settextposition(int x,int y);