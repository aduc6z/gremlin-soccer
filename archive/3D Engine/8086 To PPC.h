#define DATA	r3#define TEMP0	r4#define TEMP1	r5#define TEMP2	r6#define EAX	r7#define SCREEN	r8#define EBX	r9#define	ECX	r10#define	EDX	r11#define ESI	r12#define EJX	r13#define	MOVBHDL		rlwimi	EBX, EDX, 8, 16, 23;#define ADDEDXESI	addc	EDX, EDX, ESI;#define ADCEDXESI	adde	EDX, EDX, ESI;#define ADDEBXECX	addc	EBX, EBX, ECX;#define ADCEBXECX	adde	EBX, EBX, ECX;#define ADDESIEDX	addc	ESI, ESI, EDX;#define ADCESIEDX	adde	ESI, ESI, EDX;// rlwinm	X, EBX, 0, 16, 31;#define MOVXBX(X)	andi.	X, EBX, 0xffff; \			lbzx	X, X, DATA;			#define MOVAH(X)	rlwimi	EAX, X, 8, 16, 23#define MOVEAXL(X)	rlwimi	EAX, X, 16, 8, 15#define MOVEAXH(X)	rlwimi	EAX, X, 24, 0, 7 #define ADCDLCH(X)	rlwinm	X, ECX, 24, 24, 31; \			adde	X, EDX, X; \			rlwimi	EDX, X, 0, 24, 31;#define SUBBXCX(X,Y)	rlwinm	X, EBX, 0, 16, 31; \			rlwinm	Y, ECX, 0, 16, 31; \			subf	X, Y, X; \			rlwimi	EBX, X, 0, 16, 31;						#define ZEROCH(X)	addis	X, r0, -1; \			addi	X, X, 0x00ff; \			and	ECX, ECX, X;#define TEST(X,Y,Z)	andi.	X, Y, Z;#define	ANDBXSI(X)	and	X, EBX, ESI; \			rlwimi	EBX, X, 0, 16,31;			#define	ANDCXSI(X)	and	X, ECX, ESI; \			rlwimi	ECX, X, 0, 16,31;#define MOVCHBL		rlwimi	ECX, EBX, 8, 16, 23;#define ADCBHDH(X,Y)	rlwinm	X, EBX, 24, 24, 31; \			rlwinm  Y, EDX, 24, 24, 31; \			adde	X, X, Y; \			rlwimi	EBX, X, 8, 16, 23;