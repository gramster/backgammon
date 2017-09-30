#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#define GLOBAL
#include "back.h"

int pointval(char c)
{
	if (c=='-') return 0;
	else return toupper(c)-64;
}

void initboard(short brd[])
{
	short i;
	for (i=0;i<32;i++) brd[i]=0;
	brd[1]=2;
	brd[12]=5;
	brd[17]=3;
	brd[19]=5;
}

int init(void)
{
	wsum=bsum=wdbl=bdbl=0;
	first=1;
	initboard(_white);
	initboard(_black);
	initboard(oldwhite);
	initboard(oldblack);
	showscreen();
	do roll(0); while (die1==die2);
	if (die1>die2) return 1;
	else return 0;
}

main()
{
        short go[6],k,n,t,z,tmp;

        go[5]=NIL;
        level=5;
        wsng=bsng=wgam=bgam=wbak=bbak=0;
	init_metawin();
	randomize();
restart:
        if (init()) goto noblmove;

blacksmv:
        whose=COMPUTER;
        if (closedhome(_white) && _black[0]) goto noblmove;
        if (!first) roll(); 
        else first=0;
        if (die1 == die2)
		{
                bdbl++;
                bsum += 4*die1;
	        }
        else bsum += die1+die2;
        showmove();
        status();
	clearmessage();
        message("I move: ");
        if (nextmove(_black,_white)==NIL) goto noblmove;
	sleep(5); /* Give player a chance to see */
        showmove();
        if (piececount(_black,0,24)==0)
	        {
		gotomessage(1);
                message("I win ");
                if ((piececount(_white,0,6)>0)&&(piececount(_white,0,24)==15))
        	        {
                        message("a backgammon!");
                        bbak++;
                	}
                else if (piececount(_white,0,24)==15)
	                {
                        message("a gammon!");
                        bgam++;
        	        }
                else bsng++;
                level=(level+3)/2;
                status();
		gotomessage(2);
                message("Press any key for another game...");
                getch();
                goto restart;
	        }

noblmove:
        whose=HUMAN;
        if (closedhome(_black)&&_white[0]) goto blacksmv;
        if (!first) roll();
        else first=0;
        if (die1 == die2)
		{
                wdbl++; 
                wsum+=4*die1; 
	        }
        else wsum+=die1+die2;
        status();
        imoves=0;
        movegen(_white,_black);
        if (die1!=die2)
	        { 
                tmp=die1; 
                die1=die2; 
                die2=tmp; 
                movegen(_white,_black);
	        }

retry:
        showmove();
	clearmessage();
        if (imoves==0)
	        {
                error("You can't move");
                goto blacksmv;
	        }
        message("Your move?");
        getstring(s,10);
        spos=s;
        while (*spos==' ') spos++;
        if (*spos=='=')
		{
                reveal();
                goto retry;
		}
	if (*spos=='z' || *spos=='Z')
		{
		if (spos[1]=='z' || spos[1]=='Z')
			{
			exit_metawin();
			exit(0);
			}
		else goto retry;
		}
        if (*spos=='?')
		{
                showhow();
		status();
                goto retry;
	        }
        switch(*spos)
		{
	        case '\0':
	        case '\n':
	                error("You have to move ");
	                if (imoves==1) reveal();
	                goto retry;

		default  :
	                if (!ispoint(*spos)) goto retry;
	                for (n=0;n<4;n++)
				{
	                        if (!ispoint(*spos)) break;
	                        go[n]=pointval(*spos++);
		                }
	                if ((n==1) && isdigit(*spos))
				{
	                        if ((*spos-'0')==die2)
					{
	                                t=die1;
	                                die1=die2;
	                                die2=t;
		                        }
	                        else if ((*spos-'0')!=die1) goto retry;
	                        go[n++]=pointval(*(++spos));
		                }


	                /*
	                go[0]=pointval(*s);
	                if (die1!=die2)
				{
				if (spos[1]
					if (!isdigit(*spos)) goto retry;
				z=*spos-'0';
				if((z!=die1)&&(z!=die2)) goto retry;
				if (z==die2) *s='-';
				if ((spos[1]=='-')||isalpha(spos[1]))
					{
					if (spos[1]=='-') go[0]=0;
					else go[0]=toupper(spos[1])-64;
					n++;
					if (isdigit(spos[2]))
						{
						if (die1+die2-z!=spos[2]-'0')
						goto retry;
						if (isalpha(spos[3])||(spos[3]=='-'))
							{
							if (spos[3]=='-') go[1]=0;
							else go[1]=toupper(spos[3])-64;
							n++;
							}
						else goto retry;
						}
					else if (spos[2]!='\n') goto retry;
					}
				else goto retry;
				}
			else while (isalpha(*spos)||(*spos=='-'))
				{
				if (*spos=='-') go[n++]=0;
				else if (isalpha(*spos))
					go[n++]=toupper(*spos)-64;
				else goto retry;
				spos++;
				}
*/

			if (((die1!=die2) && (n>2)) || n>4)
				{
				error("Too many moves.");
				goto retry;
				}
			if (((die1!=die2) && (n<2) && (moves[1]!=NIL))|| ((die1==die2) && (n<4) && (moves[3]!=NIL)))
				{
				error("Too few  moves"); 
				goto retry; 
				}
			go[n]=NIL;
			for (k=0;k<n;k++)
				{
				if ((go[k]>=0) && (go[k]<25)) continue;
				else	{
					error("Invalid move!");
					goto retry; 
					}
				}
			if (play(_white,_black,go)) goto retry;
			}

	if (piececount(_white,0,24)==0)
		{
		gotomessage(1);
		message("You win ");
		if ((piececount(_black,0,6)>0)&&(piececount(_black,0,24)==15))
			{
			message("a backgammon!");
			wbak++;
			}
		else if (piececount(_black,0,24)==15)
			{
			message("a gammon");
			wgam++;
			}
		else wsng++;
		if ((level>0)&&(level<5))
			if (level<3) level--;
			else level++;
                status();
		gotomessage(2);
                message("Press any key for another game...");
                getch();
                goto restart;
	        }
        goto blacksmv;
}

/***************************************/
reveal(void)
{
        short m,n;
	char buff[80];

        for (m=0;m<imoves;m++)
		{
                for (n=0;n<4;n++)
			{
                        if (moves[m*8+n]==NIL) break;
                        if (moves[m*8+4]!=moves[m*8+5]) /* Not doubles */
                                sprintf(buff,"%c%0d",chcod(moves[m*8+n]),moves[m*8+4+n]);
                        else sprintf(buff,"%c",chcod(moves[m*8+n]));
			message(buff);
	                }
	        }
}

closedhome(short *who)
{
        short k;

        for (k=19;k<25;k++) if (who[k]<2) return FALSE;
        return TRUE;
}

/****************************************************************/
play(short *player,short *playee, short pos[])
{
        short k,n,die,ipos;
	char buff[80];

        for (k=0;k<player[0];k++)
		{
                if (pos[k]==NIL) break;
                if (pos[k]!=0)
	                { 
                        error("You must bring piece(s) on first");
                        return(NIL); 
        	        }
	        }
        for (k=0;(ipos=pos[k])!=NIL;k++)
		{
                if (k) die=die2; 
                else die=die1;
                n=25-ipos-die;
                if (player[ipos]==0) goto badmove;
                if (n>0 && playee[n]>=2) goto badmove;
                if (n<=0)
			{
                        if (piececount(player,0,18)) goto badmove;
                        if ((ipos+die)!=25 && piececount(player,19,24-die))
                                goto badmove;
	                }
                player[ipos]--;
                player[ipos+die]++;
	        }
        for (k=0;pos[k]!=NIL;k++)
		{
                if (k) die=die2;
                else die=die1;
                n=25-pos[k]-die;
                if (n>0 && playee[n]==1)
			{
                        playee[n]=0;
                        playee[0]++;
	                }
	        }
        return(0);

badmove:
        sprintf(buff,"You can't move from %c\n",ipos+'A'-1);
	error(buff);
        while (k--)
		{
                if (k) die=die2; 
                else die=die1;
                player[pos[k]]++;
                player[pos[k]+die]--;
	        }
        return(NIL);
}

/*******************************************************/
nextmove(short *player, short *playee)
{
        short k;

        imoves=0;
        movegen(player,playee);
        if (die1!=die2)
		{
		k=die1;
                die1=die2;
                die2=k; 
                movegen(player,playee);
	        }
        if (imoves==0)
		{
		char buff[80];
                sprintf(buff,"nothing with %d,%d\n",die1,die2);
		message(buff);
                return(NIL);
	        }
        k=strategy(player,playee);
        prtmov(k);
        update(player,playee,k);
        return(0);
}

/*******************************************************/
prtmov(k)
short k;
{
        short n;

        if (k==NIL) printf("No move possible\n");
        else for (n=0;n<4;n++)
		{
                if (moves[k*8+n]==NIL) break;
                if (moves[k*8+n]==0) message("-");
                else	{
			char c[2];
			c[0] = 24-moves[k*8+n]+'A';
			c[1]=0;
			message(c);
			}
                if ((die1!=die2)&&(moves[k*8]!=moves[k*8+1])&&(n==0))
			{
			char buff[80];
                        sprintf(buff,"%d",moves[k*8+n+4]);
			message(buff);
			}
	        }
        if (die1==die2)
		{
		char buff[20];
		sprintf(buff,"(double %d)",die1);
		message(buff);
		}
}

