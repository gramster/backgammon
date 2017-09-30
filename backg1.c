#include <stdio.h>
#include <stdlib.h>
#include "back.h"

#define TRUE 	1
#define FALSE 	0

roll(who)
short who;
{
        die1=1+random(6);
	die2=1+random(6);
        drawdice();
}

/*******************************************************/
update(short *player, short *playee, short k)
{
        short n,t;

        for (n=0;n<4;n++)
		{
                if (moves[k*8+n]==NIL) break;
                player[moves[k*8+n]]--;
                player[moves[k*8+n]+moves[k*8+n+4]]++;
                t=25-moves[k*8+n]-moves[k*8+n+4];
                if (t>0 && playee[t]==1)
			{ 
                        playee[0]++; 
                        playee[t]--;
	                }
	        }
}

/******************************************************/
piececount(short *player, short startrow, short endrow)
{
        short sum=0;
        while (startrow<=endrow) sum+=player[startrow++];
        return(sum);
}


/*******************************************************/
movegen(short *mover, short *movee)
{
        short k;

        for (i=0;i<=24;i++)
		{
                count=0;
                if (mover[i]==0) continue;
                if (blocked(movee,die1,i))
                        if (mover[0]) break; 
                        else continue;
                if (cantdo(mover,die1,i)) break;
                mover[i]--; 
                mover[i+die1]++; 
                count=1;

                for (j=0;j<=24;j++)
			{
                        if (mover[j]==0) continue;
                        if (blocked(movee,die2,j))
                                if (mover[0]) break; 
                                else continue;
                        if (cantdo(mover,die2,j)) break;
                        mover[j]--; 
                        mover[j+die2]++; 
                        count=2;
                        if (die1!=die2)
				{
                                moverecord(mover);
                                if (mover[0]>0) break; 
                                else continue;
	                        }
                        for (l=0;l<=24;l++)
				{
                                if (mover[l]==0) continue;
                                if (blocked(movee,die1,l))
                                        if (mover[0]) break; 
                                        else continue;
                                if (cantdo(mover,die1,l)) break;
                                mover[l]--; 
                                mover[l+die1]++; 
                                count=3;
                                for (m=0;m<=24;m++)
					{
                                        if (mover[m]==0) continue;
                                        if (blocked(movee,die1,m))
                                                if (mover[0]) break; 
                                                else continue;
                                        if (cantdo(mover,die1,m)) break;
                                        count=4;
                                        moverecord(mover);
                                        if (mover[0]>0) break;
	                                }
                                if (count==3) moverecord(mover);
                                else 	{ 
                                        mover[l]++; 
                                        mover[l+die1]--; 
	                                }
                                if (mover[0]>0) break;
	                        }
                        if (count==2) moverecord(mover);
                        else 	{ 
                                mover[j]++; 
                                mover[j+die1]--; 
	                        }
                        if (mover[0]>0) break;
        	        }
                if (count==1) moverecord(mover);
                else	{ 
                        mover[i]++; 
                        mover[i+die1]--; 
	                }
                if (mover[0]>0) break;
	        }
}

blocked(short *playee, short die, short pt)
{
        short k;
        k=25-pt-die;
        if ((k>0)&&(playee[k]>1)) return TRUE;
        else return FALSE;
}

cantdo(short *player, short die, short pt)
{
        short k;
        k=25-pt-die;
        if (player[pt] && (k<=0))
                if (piececount(player,0,18)|| ((k<0) && piececount(player,19,pt-1)))
                        return(TRUE);
        return(FALSE);
}


/*****************************************************/
moverecord(short *mover)
{
        short t;

        if (imoves<MAXIMOVES)
		{
                for (t=0;t<=3;t++) moves[imoves*8+t]=NIL;
                switch (count)
			{
	                case 4: 
	                        moves[imoves*8+3]=m; 
	                        moves[imoves*8+7]=die1;
	                case 3: 
	                        moves[imoves*8+2]=l; 
	                        moves[imoves*8+6]=die1;
	                case 2: 
	                        moves[imoves*8+1]=j; 
	                        moves[imoves*8+5]=die2;
	                case 1: 
	                        moves[imoves*8  ]=i; 
	                        moves[imoves*8+4]=die1; 
	                        imoves++;
	                }
	        }
        switch (count)
		{
	        case 4: 
	                break;
	        case 3: 
	                mover[l]++; 
	                mover[l+die1]--; 
	                break;
	        case 2: 
	                mover[j]++; 
	                mover[j+die2]--; 
	                break;
	        case 1: 
	                mover[i]++; 
	                mover[i+die1]--;
	        }
}

/*****************************************************/
strategy(short *player, short *playee)
{
        short k,n,nn,bestval,moveval,prob;

        n=0;
        if (imoves==0) return(NIL);
        goodmoves[0]=NIL;
        bestval=-32000;
        for (k=0; k<imoves; k++)
		{
                if ((moveval=eval(player,playee,k,&prob)) < bestval) continue;
                if (moveval>bestval)
			{ 
                        bestval=moveval; 
                        n=0; 
	                }
                if (n<MAXGMOV)
			{ 
                        goodmoves[n]=k; 
                        probmoves[n++]=prob; 
	                }
	        }
        if (n>1) {
                nn=n;
                n=0;
                prob=32000-level*(1000+random(1000));
                for (k=0;k<nn;k++)
			{
                        if ((moveval=probmoves[k])>prob) continue;
                        if (moveval<prob)
				{ 
                                prob=moveval; 
                                n=0; 
	                        }
                        goodmoves[n]=goodmoves[k];
                        probmoves[n++]=probmoves[k];
	                }
	        }
        return(goodmoves[random(n)]);
}

/******************************************************/
eval(player,playee,k,prob)
short *player,*playee,k,*prob;
{
        short newtry[31],newother[31],*r,*q,*p,n,sum,first,ii,t1,t2,
        lastwhite,lastblack;

        *prob=sum=0;
        r=player+25;
        p=newtry;
        q=newother;
        while (player<r)
		{ 
                *p++= *player++; 
                *q++= *playee++; 
	        }
        q=newtry+31;
        for (p=newtry+25;p<q;p++) *p=0;
        for (n=0;n<4;n++)
		{
                if (moves[k*8+n]==NIL) break;
                newtry[moves[k*8+n]]--;
                newtry[ii=moves[k*8+n]+moves[k*8+n+4]]++;
                if (ii<25 && newother[25-ii]==1)
			{
                        newother[25-ii]=0;
                        newother[0]++;
                        if (ii<=15) sum+=10;
                        if ((ii==7)||(ii==5)) sum+=10;
                        if ((ii==4)||(ii==6)) sum+=5;
	                }
	        }
        for (lastwhite=0;newother[lastwhite]==0;lastwhite++);
        for (lastblack=0;newtry[lastblack]==0;lastblack++);
        lastblack=25-lastblack;
        if (lastblack<=6 && lastblack<lastwhite) sum=1000;
        if (lastwhite>lastblack && lastblack>6)
		{
                for (sum=1000,t1=lastblack; t1>6; t1--)
                        sum-=t1*newtry[25-t1];
                /*#####################################*/
                /* My code for races, to move furthest */
                if (t1=piececount(_black,1,6))
	                {
                        if (t1==piececount(newtry,1,6))
                                sum-=10*(8-lastblack); /* Skei backgammon! */
	                }
                else if (piececount(_black,7,18)==piececount(newtry,7,18))
                        /* Didn't bring any in home , and more on board */
                        if (piececount(newtry,7,24)>piececount(newother,7,24))
                                sum-=6*piececount(_black,16,18); /* Rather bring in -
                                                               Skei gammon! */
                /* End of my code */
                /*################*/
		        }
        for (first=0;first<25; first++)
                if (newother[first]) break;
        q=newtry+25;
        for (ii=1,p=newtry+1;p<q;++ii)
                if ((*p++>1)&&(ii>6)) sum+=1+3*(ii==18||ii==19)+(ii==20);
        if (first>5)
		{
                q=newtry+31;
                p=newtry+25;
                for (n=6;p<q;n--) sum+= *p++ * n;
	        }
        r=newtry+25-first;
        for (ii=7,p=newtry+7; p<r; ++ii)
                if ((*p++ == 1) && (getprob(newtry,newother,ii,ii)>(6+level)))
                        sum--;
        q=newtry+3;
        for (p=newtry;p<q;) sum-= *p++;
        for (n=1;n<=4;n++) *prob+= n*getprob(newtry,newother,6*n-5,6*n);
        /*******************************************/
        /* My code to 'flatten' the home board, and*/
        /* maybe close it up.                      */
        for (n=19;n<25;n++) if (newtry[n]<2) break;
        if (n==25) sum+=10*(2+newother[0]);
        /* End of my code                          */
        /*******************************************/
        return(sum);
}


/***************************************************/
getprob(short *player, short *playee, short start, short finish)
{
        short k,n,sum;

        sum=0;
        for (;start<=finish;start++)
		{
                if (player[start]==1)
			{
                        for (k=1;k<=24;k++)
				{
                                if ((n=25-start-k)<0) break;
                                if (playee[n]!=0) sum+=figprob(player,start,playee,n,k);
	                        }
	                }
	        }
        return(sum);
}



figprob(short *player, short us, short *playee, short them, short k)
{
        short sum,move,i,i1,i2;
        char c;

        sum=0;
        if (probability[k].n==0) return (0);
        for (i=0;i<probability[k].n;++i)
                if (probability[k].cnt[i][0]==probability[k].cnt[i][1])
			{
                        c=probability[k].cnt[i][0];
                        move=c;
                        for (i1=1;i1<=4;++i1)
                                if (25-them-(move*i1)<us && player[25-them-(move*i1)]>=2)
                                        break;
                        if (i1==5) sum++;
	                }
                else 	{
                        c=probability[k].cnt[i][0];
                        i1=c;
                        c=probability[k].cnt[i][1];
                        i2=c;
                        if ((i1+i2)==k)
				{
                                if (player[25-them-i1]<2 || player[25-them-i2]<2) sum++; 
	                        }
                        else 	{ 
                                sum++; 
	                        }
        	        }
        return(sum);
}

