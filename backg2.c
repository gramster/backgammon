#include <stdio.h>
#include <stdlib.h>
#include "grconst.h"
#include "grports.h"
#include "grextrn.h"
#include "back.h"

#define BLACK		0
#define BLUE		1
#define GREEN		2
#define CYAN		3
#define RED		4
#define MAGENTA		5
#define BROWN		6
#define GREY		7
#define LIGHT		8
#define DARKGREY	LIGHT|BLACK
#define WHITE		LIGHT|GREY
#define YELLOW		LIGHT|BROWN

#define X_PIECE_RADIUS	30
#define Y_PIECE_RADIUS	25
#define Y_EXTENT	600
#define X_EXTENT	750
#define POINT_WIDTH	((X_EXTENT *2)/15)

int x_pix, y_pix;
int x1_split, x2_split, y1_split, y2_split;
metaPort board_port, die_port, status_port, IO_port, my_port;
metaPort *portnow;

rect R; /* scratch rectangle */
fontRec *font;


static void circle(int x, int y, int xradius, int yradius)
{
	rect r;
	r.Xmin = x-xradius;
	r.Xmax = x+xradius;
	r.Ymin = y+yradius;
	r.Ymax = y-yradius;
	PaintOval(&r);
}

static void pieces(int color, int x, int ysgn, int num)
{
	int y = ysgn * (Y_EXTENT - (num+1)*2*Y_PIECE_RADIUS);
	PenColor(color);
	while (num--)
		{
		circle(x,y,X_PIECE_RADIUS,Y_PIECE_RADIUS);
		y += ysgn*2*Y_PIECE_RADIUS;
		}
}

static void triangle(int color, int basex, int basey, int width, int height)
{
	polyHead tr;
	rect r;
	point pt[3];
	r.Xmin = pt[0].X = basex - width / 2;
	r.Ymin = pt[0].Y = basey;
	r.Xmax = pt[1].X = basex + width / 2;
	pt[1].Y = basey;
	pt[2].X = basex;
	r.Ymax = pt[2].Y = basey + height;
	tr.polyBgn = 0;
	tr.polyEnd = 2;
	tr.polyRect = portnow->portVirt;
	BackColor(BLACK);
	EraseRect(&r);
	PenColor(color);
	PaintPoly(1,&tr,pt);
}

static void rectangle(int color,int x1, int y1, int x2, int y2)
{
	rect r;
	SetRect(&r,x1,y1,x2,y2);
	PenColor(color);
	PaintRect(&r);
}

static void drawpoint(int pt)
{
	int x, xsgn, ysgn;
	ysgn = (pt>12) ? -1: 1;
	if (pt>0 && pt<25)
		{
		char name = pt-1+'A';
		xsgn = ((pt<7) || (pt>18)) ? -1 : 1;
		x = (pt-1) % 6;
		if ((xsgn*ysgn)<0) x = 5-x;
		x = xsgn*(++x * POINT_WIDTH);
		triangle((pt%2)?RED:BLUE,x,ysgn*(Y_EXTENT-2*Y_PIECE_RADIUS),
			POINT_WIDTH,-ysgn*Y_EXTENT*6/7);
		PenColor(WHITE);
		MoveTo(x,ysgn*(Y_EXTENT-Y_PIECE_RADIUS));
		DrawChar(name);
		}
	/* draw bar point */
	else 	{
		x=0; /* correct for bar point */
		rectangle(GREEN,-POINT_WIDTH/2,-Y_EXTENT,+POINT_WIDTH/2,+Y_EXTENT);
		PenColor(WHITE);
		MoveTo(0,0);
		DrawString("BAR");
		}
	/* draw pieces */
	pieces(WHITE, x, ysgn,_white[pt]);
	pieces(GREY, x, ysgn,_black[25-pt]);
}


static void drawdie(int sign, int val)
{
	rectangle(WHITE,-20,sign*50,20,sign*90);
	PenColor(RED);
	if ((val%2)==1) circle(0,70*sign,4,4);
	if (val>1)
		{
		circle(-10,sign*80,4,4);
		circle(+10,sign*60,4,4);
		if (val>3)
			{
			circle(+10,sign*80,4,4);
			circle(-10,sign*60,4,4);
			if (val==6)
				{
				circle(-10,sign*70,4,4);
				circle(+10,sign*70,4,4);
				}
			}
		}
}

void drawdice(void)  /* draws using 'die1' and 'die2' global integers */
{
	SetPort(&die_port);
	drawdie(+1,die1);
	drawdie(-1,die2);
}

void status(void)
{
	char buff[40];
	SetPort(&status_port);
	EraseRect(&status_port.portRect);
	MoveTo(9,1);
	PenColor(GREEN);
	TextFace(cUnderline+cBold);
	DrawString("STATUS");
	MoveTo(6,3);
	TextFace(cUnderline);
	DrawString("Throw Analysis");
	TextFace(cNormal);
	MoveTo(6,4);
	DrawString("Doubles   Total");
	MoveTo(1,5);
	sprintf(buff,"Me :   %-10d%-d",bdbl,bsum);
	DrawString(buff);
	MoveTo(1,6);
	sprintf(buff,"You:   %-10d%-d",wdbl,wsum);
	DrawString(buff);
	MoveTo(2,8);
	TextFace(cUnderline);
	DrawString("Score Sg Gm Bk   Tot");
	TextFace(cNormal);
	MoveTo(2,9);
	sprintf(buff,"Me :  %-3d%-3d%-3d| %-3d",bsng,bgam,bbak,
				bsng+2*bgam+3*bbak);
	DrawString(buff);
	MoveTo(2,10);
	sprintf(buff,"You:  %-3d%-3d%-3d| %-3d",wsng,wgam,wbak,
				wsng+2*wgam+3*wbak);
	DrawString(buff);
	MoveTo(6,12);
	TextFace(cUnderline);
	DrawString("Beared Off");
	TextFace(cNormal);
	MoveTo(1,13);
	sprintf(buff,"Me :%-7dYou: %-d",15-piececount(_black,0,24),
			     15-piececount(_white,0,24));
	DrawString(buff);
	MoveTo(1,15);
	PenColor(RED);
	DrawString("  Enter ? for help.");
}


void showscreen(void) /* clear and redraw the whole screen */
{
	int pt;
	/* clear the board window */

	SetPort(&board_port);
	PenMode(0);
	BackColor(BLACK);
	EraseRect(&board_port.portRect);

	/* Draw left, right, bottom and top borders */

	rectangle(GREEN,-X_EXTENT,-Y_EXTENT,-X_EXTENT+POINT_WIDTH,+Y_EXTENT);
	rectangle(GREEN,+X_EXTENT-POINT_WIDTH,-Y_EXTENT,+X_EXTENT,+Y_EXTENT);
	rectangle(GREEN,-X_EXTENT,-Y_EXTENT,+X_EXTENT,-Y_EXTENT+2*Y_PIECE_RADIUS);
	rectangle(GREEN,-X_EXTENT,+Y_EXTENT,+X_EXTENT,+Y_EXTENT-2*Y_PIECE_RADIUS);

	/* Draw points and pieces */

	for (pt=0;pt<=25;pt++) drawpoint(pt);

	/* draw the dice */

	drawdice();

	/* show the status */

	status();
}

char *helptext1[11] = {
	"You play white. Each point",
	"has a letter which you use",
	"to refer to it. The bar is",
	" represented  by a minus",
	"sign '-'.",
	"",
	"To specify a move, you must",
	"say  which  point you are",
	"moving from. If you threw",
	"doubles, you just enter the",
	"points in order (eg  ssll)"
};

char *helptext2[13] = {
	"Otherwise, if you are moving",
	"moving both dice from the",
	"same point, enter it twice",
	"(eg  ll).",
	"",
	"In the last case, (both dice",
	"and points different, type",
	"the  value  of  the  first",
	"move's  dice  between  the",
	"points (eg l3s).",
	"",
	"To quit, enter ZZ."
};

void showhow(void)
{
	int i;
	SetPort(&status_port);
	EraseRect(&status_port.portRect);
	TextFace(cUnderline);
	MoveTo(6,1);
	DrawString("How to Play");
	TextFace(cNormal);
	for (i=3; i<13;i++) { MoveTo(1,i); DrawString(helptext1[i-3]); }
	MoveTo(1,15); PenColor(WHITE); DrawString("  (Press any key...)");
	getch();
	EraseRect(&status_port.portRect);
	PenColor(GREEN);
	for (i=1;i<13;i++) { MoveTo(1,i); DrawString(helptext2[i-1]); }
	MoveTo(1,13);
	PenColor(RED);
	DrawString("   Good luck!");
	MoveTo(1,15);
	PenColor(WHITE);
	DrawString("        ( Press any key )");
	getch();
}



void showmove(void)
{
	short i;

	SetPort(&board_port);
	for (i=0;i<26;i++)
		if ((oldblack[25-i]!=_black[25-i])||(oldwhite[i]!=_white[i]))
			{
			drawpoint(i);
			oldblack[25-i]=_black[25-i];
			oldwhite[i]=_white[i];
			}
}


void backg_abort(void)
{
	fprintf(stderr,"Error initializing\n");
	exit(0);
}

void init_metawin(void)
{
	int pt, xsgn, ysgn;
	int color;

	if (InitGrafix(EGA640x350)!=0) backg_abort();
	if ( (font=(fontRec *)malloc(6000)) == NULL ) backg_abort();
	if ( FileLoad("backscrn.dat",font,4000) < 0) backg_abort();

	ScreenSize(&x_pix,&y_pix);
	SetDisplay(GrafPg0);
	x1_split = x_pix/2;
	x2_split = x1_split+x_pix/8;
	y1_split = (3*y_pix)/5;
	y2_split = (4*y_pix)/5;

	InitPort(&my_port);
	MovePortTo(0,y2_split);
	PortSize(x_pix,y_pix-y2_split);
	PortOrigin(1);
	EraseRect(&my_port.portRect);
	SetFont(font);
	RasterOp(6);
	TextFace(cBold);
	PenColor(RED);
	MoveTo(x_pix/3,(y_pix-y2_split)/2);
	DrawString("BackGammon");
	MoveTo(x_pix/3+2,(y_pix-y2_split)/2+1);
	PenColor(WHITE);
	DrawString("BackGammon");
	MoveTo(x_pix/3+4,(y_pix-y2_split)/2+2);
	PenColor(GREEN);
	DrawString("BackGammon");

	if ( FileLoad("backgamm.fnt",font,4000) < 0) backg_abort();
	InitPort(&board_port);
	MovePortTo(0,0);
	PortSize(x1_split,y1_split);
	PortOrigin(0); /*lower left */
	SetRect(&R,-X_EXTENT,-Y_EXTENT,+X_EXTENT,+Y_EXTENT);
	VirtualRect(&R);
	SetFont(font);
	TextAlign(alignCenter,alignMiddle);

	InitPort(&die_port);
	MovePortTo(x1_split,0);
	PortSize(x2_split-x1_split,y1_split);
	PortOrigin(0); /*lower left */
	SetRect(&R,-30,-150,+30,+150);
	VirtualRect(&R);

	InitPort(&status_port);
	MovePortTo(x2_split,0);
	PortSize(x_pix-x2_split,y1_split);
	PortOrigin(1); /*top left */
	SetRect(&R,0,0,22,15);
	VirtualRect(&R);
	SetFont(font);

	InitPort(&IO_port);
	MovePortTo(0,y1_split);
	PortSize(x_pix,y2_split-y1_split);
	PortOrigin(1); /*top left */
	SetFont(font);

}

void exit_metawin(void)
{
	int i;
	SetDisplay(TextPg0);
	ClearText();
	free(font);
	i = QueryError();
	if (i) fprintf(stderr,"Exit: %d\n",i);
}


int messagerow = 0;

void gotomessage(int row)
{
	SetPort(&IO_port);
	MoveTo(10,(row+1)*12);
}

void clearmessage(void)
{
	gotomessage(0);
	EraseRect(&IO_port.portRect);
}

void message(char *msg)
{
	SetPort(&IO_port);
	PenColor(WHITE);
	DrawString(msg);
}

void error(char *msg)
{
	SetPort(&IO_port);
	PenColor(RED);
	gotomessage(2);
	DrawString(msg);
	sleep(2);
}

void getstring(char *buf, int len)
{
	int startX,startY, pos=0;
	char c;
	SetPort(&IO_port);
	PenColor(GREEN);
	startX = IO_port.pnLoc.X;
	startY = IO_port.pnLoc.Y;
	while (c=getch(), c != '\n' && c != '\r' && c!='?')
		{
		if (c==8 || c==27) /* backspace or escape */
			{
			if (pos)
				{
				pos=0;
				MoveTo(startX,startY);
				DrawString("            ");
				MoveTo(startX,startY);
				}
			}
		else	{
			buf[pos++] = c;
			DrawChar(c);
			if (pos>=len) break;
			}
		}
	buf[pos]=0;
	if (c=='?') buf[0]='?';
}



