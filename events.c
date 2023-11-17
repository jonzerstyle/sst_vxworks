#include "sst.h"
#include <math.h>

void events(void) {

	int ictbeam=0, ipage=0, istract=0, line, i, j, k, l, ixhold, iyhold;
	double fintim = dsst.date + Time, datemin, xtime, repair, yank;
	

#ifdef DEBUG
	if (idebug) prout("EVENTS");
#endif

	if (stdamtim == 1e30 && !REPORTS)
	{
		/* chart will no longer be updated because radio is dead */
		stdamtim = dsst.date;
		for (i=1; i <= 8 ; i++)
			for (j=1; j <= 8; j++)
				if (starch[i][j] == 1) starch[i][j] = dsst.galaxy[i][j]+1000;
	}

	for (;;) {
		/* Select earliest extraneous event, line==0 if no events */
		line = FSPY;
		if (alldone) return;
		datemin = fintim;
		for (l=1; l<=NEVENTS; l++)
			if (future[l] <= datemin) {
				line = l;
				datemin = future[l];
			}
		xtime = datemin-dsst.date;
#ifdef CLOAKING
		if (iscloaking) {
			energy -= xtime*500.0;
			if (energy <= 0.) {
				finish(FNRG);
				return;
			}
		}
#endif
		dsst.date = datemin;
		/* Decrement Federation resources and recompute remaining time */
		dsst.remres -= (dsst.remkl+4*dsst.remcom)*xtime;
		dsst.remtime = dsst.remres/(dsst.remkl+4*dsst.remcom);
		if (dsst.remtime <=0) {
			finish(FDEPLETE);
			return;
		}
		/* Is life support adequate? */
		if (damage[DLIFSUP] && condit != IHDOCKED) {
			if (lsupres < xtime && damage[DLIFSUP] > lsupres) {
				finish(FLIFESUP);
				return;
			}
			lsupres -= xtime;
			if (damage[DLIFSUP] <= xtime) lsupres = inlsr;
		}
		/* Fix devices */
		repair = xtime;
		if (condit == IHDOCKED) repair /= docfac;
		/* Don't fix Deathray here */
		for (l=1; l<=ndevice; l++)
			if (damage[l] > 0.0 && l != DDRAY)
                damage[l] -= (damage[l]-repair > 0.0 ? repair : damage[l]);
        /* Fix Deathray if docked */
        if (damage[DDRAY] > 0.0 && condit == IHDOCKED)
            damage[DDRAY] -= (damage[l] - xtime > 0.0 ? xtime : damage[DDRAY]);
		/* If radio repaired, update star chart and attack reports */
		if (stdamtim != 1e30 && REPORTS) {
			stdamtim = 1e30;
			prout("Lt. Uhura- \"Captain, the sub-space radio is working and");
			prout("   surveillance reports are coming in.");
			skip(1);
			for (i=1; i <= 8 ; i++)
				for (j=1; j <= 8; j++)
					if (starch[i][j] > 999) starch[i][j] = 1;
			if (iseenit==0) {
				attakreport();
				iseenit = 1;
			}
			skip(1);
			prout("   The star chart is now up to date.\"");
			skip(1);
		}
		/* Cause extraneous event LINE to occur */
		Time -= xtime;
		switch (line) {
			case FSNOVA: /* Supernova */
				if (ipage==0) pausesst(1);
				ipage=1;
				snova(0,0);
				future[FSNOVA] = dsst.date + expran(0.5*intime);
				if (dsst.galaxy[quadx][quady] == 1000) return;
				break;
			case FSPY: /* Check with spy to see if S.C. should tractor beam */
                if (dsst.nscrem == 0 ||
#ifdef CLOAKING
                      iscloaked ||  /* Cannot tractor beam if we can't be seen! */
#endif
					ictbeam+istract > 0 ||
					condit==IHDOCKED || isatb==1 || iscate==1) return;
				if (ientesc ||
					(energy < 2000 && torps < 4 && shield < 1250) ||
					(damage[DPHASER]>0 && (damage[DPHOTON]>0 || torps < 4)) ||
					(damage[DSHIELD] > 0 &&
					 (energy < 2500 || damage[DPHASER] > 0) &&
					 (torps < 5 || damage[DPHOTON] > 0))) {
					/* Tractor-beam her! */
					istract=1;
					yank = square(dsst.isx-quadx) + square(dsst.isy-quady);
					/*********TBEAM CODE***********/
				}
				else return;
			case FTBEAM: /* Tractor beam */
				if (line==FTBEAM) {
					if (dsst.remcom == 0) {
						future[FTBEAM] = 1e30;
						break;
					}
					i = Rand()*dsst.remcom+1.0;
					yank = square(dsst.cx[i]-quadx) + square(dsst.cy[i]-quady);
                    if (istract || condit == IHDOCKED ||
#ifdef CLOAKING
                          iscloaked || /* cannot tractor beam if we can't be seen */
#endif
                          yank == 0) {
						/* Drats! Have to reschedule */
						future[FTBEAM] = dsst.date + Time +
										 expran(1.5*intime/dsst.remcom);
						break;
					}
				}
				/* tractor beaming cases merge here */
				yank = sqrt(yank);
				if (ipage==0) pausesst(1);
				ipage=1;
				Time = (10.0/(7.5*7.5))*yank; /* 7.5 is yank rate (warp 7.5) */
				ictbeam = 1;
				skip(1);
				proutn("***");
				crmshp();
				prout(" caught in long range tractor beam--");
				/* If Kirk & Co. screwing around on planet, handle */
				atover(1); /* atover(1) is Grab */
				if (alldone) return;
				if (icraft == 1) { /* Caught in Galileo? */
					finish(FSTRACTOR);
					return;
				}
				/* Check to see if shuttle is aboard */
				if (iscraft==0) {
					skip(1);
					if (Rand() >0.5) {
						prout("Galileo, left on the planet surface, is captured");
						prout("by aliens and made into a flying McDonald's.");
						damage[DSHUTTL] = -10;
						iscraft = -1;
					}
					else {
						prout("Galileo, left on the planet surface, is well hidden.");
					}
				}
				if (line==0) {
					quadx = dsst.isx;
					quady = dsst.isy;
				}
				else {
					quadx = dsst.cx[i];
					quady = dsst.cy[i];
				}
				iran10(&sectx, &secty);
				crmshp();
				proutn(" is pulled to");
				cramlc(1, quadx, quady);
				proutn(", ");
				cramlc(2, sectx, secty);
				skip(1);
				if (resting) {
					prout("(Remainder of rest/repair period cancelled.)");
					resting = 0;
				}
				if (shldup==0) {
					if (damage[DSHIELD]==0 && shield > 0) {
						sheild(2); /* Shldsup */
						shldchg=0;
					}
					else prout("(Shields not currently useable.)");
				}
				newqad(0);
				/* Adjust finish time to time of tractor beaming */
				fintim = dsst.date+Time;
				if (dsst.remcom <= 0) future[FTBEAM] = 1e30;
				else future[FTBEAM] = dsst.date+Time+expran(1.5*intime/dsst.remcom);
				break;
			case FSNAP: /* Snapshot of the universe (for time warp) */
				snapsht = dsst;
				dsst.snap = 1;
				future[FSNAP] = dsst.date + expran(0.5 * intime);
				break;
			case FBATTAK: /* Commander attacks starbase */
				if (dsst.remcom==0 || dsst.rembase==0) {
					/* no can do */
					future[FBATTAK] = future[FCDBAS] = 1e30;
					break;
				}
				i = 0;
				for (j=1; j<=dsst.rembase; j++) {
					for (k=1; k<=dsst.remcom; k++)
						if (dsst.baseqx[j]==dsst.cx[k] && dsst.baseqy[j]==dsst.cy[k] &&
							(dsst.baseqx[j]!=quadx || dsst.baseqy[j]!=quady) &&
							(dsst.baseqx[j]!=dsst.isx || dsst.baseqy[j]!=dsst.isy)) {
							i = 1;
							break;
						}
					if (i == 1) break;
				}
				if (j>dsst.rembase) {
					/* no match found -- try later */
					future[FBATTAK] = dsst.date + expran(0.3*intime);
					future[FCDBAS] = 1e30;
					break;
				}
				/* commander + starbase combination found -- launch attack */
				batx = dsst.baseqx[j];
				baty = dsst.baseqy[j];
				future[FCDBAS] = dsst.date+1.0+3.0*Rand();
				if (isatb) /* extra time if SC already attacking */
					future[FCDBAS] += future[FSCDBAS]-dsst.date;
				future[FBATTAK] = future[FCDBAS] +expran(0.3*intime);
				iseenit = 0;
				if (!REPORTS)
				     break; /* No warning :-( */
				iseenit = 1;
				if (ipage==0) pausesst(1);
				ipage = 1;
				skip(1);
				proutn("Lt. Uhura-  \"Captain, the starbase in");
				cramlc(1, batx, baty);
				skip(1);
				prout("   reports that it is under atttack and that it can");
				proutn("   hold out only until stardate ");
				cramf(future[FCDBAS],1,1);
				prout(".\"");
				if (resting) {
					skip(1);
					proutn("Mr. Spock-  \"Captain, shall we cancel the rest period?\"");
					if (ja()) {
						resting = 0;
						Time = 0.0;
						return;
					}
				}
				break;
			case FSCDBAS: /* Supercommander destroys base */
				future[FSCDBAS] = 1e30;
				isatb = 2;
				if (dsst.galaxy[dsst.isx][dsst.isy]%100 < 10) break; /* WAS RETURN! */
				ixhold = batx;
				iyhold = baty;
				batx = dsst.isx;
				baty = dsst.isy;
			case FCDBAS: /* Commander succeeds in destroying base */
				if (line==FCDBAS) {
					future[FCDBAS] = 1e30;
					/* find the lucky pair */
					for (i = 1; i <= dsst.remcom; i++)
						if (dsst.cx[i]==batx && dsst.cy[i]==baty) break;
					if (i > dsst.remcom || dsst.rembase == 0 ||
						dsst.galaxy[batx][baty] % 100 < 10) {
						/* No action to take after all */
						batx = baty = 0;
						break;
					}
				}
				/* Code merges here for any commander destroying base */
				/* Not perfect, but will have to do */
				if (starch[batx][baty] == -1) starch[batx][baty] = 0;
				/* Handle case where base is in same quadrant as starship */
				if (batx==quadx && baty==quady) {
					if (starch[batx][baty] > 999) starch[batx][baty] -= 10;
					quadsst[basex][basey]= IHDOT;
					basex=basey=0;
					newcnd();
					skip(1);
					prout("Spock-  \"Captain, I believe the starbase has been destroyed.\"");
				}
				else if (dsst.rembase != 1 && REPORTS) {
					/* Get word via subspace radio */
					if (ipage==0) pausesst(1);
					ipage = 1;
					skip(1);
					prout("Lt. Uhura-  \"Captain, Starfleet Command reports that");
					proutn("   the starbase in");
					cramlc(1, batx, baty);
					prout(" has been destroyed by");
					if (isatb==2) prout("the Klingon Super-Commander");
					else prout("a Klingon Commander");
				}
				/* Remove Starbase from galaxy */
				dsst.galaxy[batx][baty] -= 10;
				for (i=1; i <= dsst.rembase; i++)
					if (dsst.baseqx[i]==batx && dsst.baseqy[i]==baty) {
						dsst.baseqx[i]=dsst.baseqx[dsst.rembase];
						dsst.baseqy[i]=dsst.baseqy[dsst.rembase];
					}
				dsst.rembase--;
				if (isatb == 2) {
					/* reinstate a commander's base attack */
					batx = ixhold;
					baty = iyhold;
					isatb = 0;
				}
				else {
					batx = baty = 0;
				}
				break;
			case FSCMOVE: /* Supercommander moves */
				future[FSCMOVE] = dsst.date+0.2777;
				if (ientesc+istract==0 &&
					isatb!=1 &&
					(iscate!=1 || justin==1)) scom(&ipage);
				break;
			case FDSPROB: /* Move deep space probe */
				future[FDSPROB] = dsst.date + 0.01;
				probex += probeinx;
				probey += probeiny;
				i = (int)(probex/10 +0.05);
				j = (int)(probey/10 + 0.05);
				if (probecx != i || probecy != j) {
					probecx = i;
					probecy = j;
					if (i < 1 || i > 8 || j < 1 || j > 8 ||
						dsst.galaxy[probecx][probecy] == 1000) {
						// Left galaxy or ran into supernova
						if (REPORTS) {
							if (ipage==0) pausesst(1);
							ipage = 1;
							skip(1);
							proutn("Lt. Uhura-  \"The deep space probe ");
							if (i < 1 ||i > 8 || j < 1 || j > 8)
								proutn("has left the galaxy");
							else
								proutn("is no longer transmitting");
							prout(".\"");
						}
						future[FDSPROB] = 1e30;
						break;
					}
					if (REPORTS) {
						if (ipage==0) pausesst(1);
						ipage = 1;
						skip(1);
						proutn("Lt. Uhura-  \"The deep space probe is now in ");
						cramlc(1, probecx, probecy);
						prout(".\"");
					}
				}
				/* Update star chart if Radio is working or have access to
				   radio. */
				if (REPORTS) 
					starch[probecx][probecy] = damage[DRADIO] > 0.0 ?
						                    dsst.galaxy[probecx][probecy]+1000 : 1;
				proben--; // One less to travel
				if (proben == 0 && isarmed &&
					dsst.galaxy[probecx][probecy] % 10 > 0) {
					/* lets blow the sucker! */
					snova(1,0);
					future[FDSPROB] = 1e30;
					if (dsst.galaxy[quadx][quady] == 1000) return;
				}
				break;
		}
	}
}

				
void waiting(void) {
	int key;
	double temp, delay, origTime;

	ididit = 0;
	for (;;) {
		key = scan();
		if (key  != IHEOL) break;
		proutn("How long? ");
	}
	chew();
	if (key != IHREAL) {
		huh();
		return;
	}
	origTime = delay = aaitem;
	if (delay <= 0.0) return;
	if (delay >= dsst.remtime || nenhere != 0) {
		prout("Are you sure? ");
		if (ja() == 0) return;
	}

	/* Alternate resting periods (events) with attacks */

	resting = 1;
	do {
		if (delay <= 0) resting = 0;
		if (resting == 0) {
			cramf(dsst.remtime, 0, 2);
			prout(" stardates left.");
			return;
		}
		temp = Time = delay;

		if (nenhere) {
			double rtime = 1.0 + Rand();
			if (rtime < temp) temp = rtime;
			Time = temp;
		}
		if (Time < delay) attack(0);
		if (nenhere==0) movetho();
		if (alldone) return;
		events();
		ididit = 1;
		if (alldone) return;
		delay -= temp;
	} while (dsst.galaxy[quadx][quady] != 1000); // leave if quadrant supernovas

	resting = 0;
	Time = 0;
}

void nova(int ix, int iy) {
	static double course[] =
		{0.0, 10.5, 12.0, 1.5, 9.0, 0.0, 3.0, 7.5, 6.0, 4.5};
	int bot, top, top2, burst, hits[11][3], kount, icx, icy, mm, nn, j;
	int iquad, iquad1, i, ll, newcx, newcy, ii, jj;
	if (Rand() < 0.05) {
		/* Wow! We've supernova'ed */
		snova(ix, iy);
		return;
	}

	/* handle initial nova */
	quadsst[ix][iy] = IHDOT;
	crmena(1, IHSTAR, 2, ix, iy);
	prout(" novas.");
	dsst.galaxy[quadx][quady] -= 1;
	dsst.starkl++;
	
	/* Set up stack to recursively trigger adjacent stars */
	bot = top = top2 = 1;
	kount = 0;
	icx = icy = 0;
	hits[1][1] = ix;
	hits[1][2] = iy;
	while (1) {
		for (mm = bot; mm <= top; mm++) 
		for (nn = 1; nn <= 3; nn++)  /* nn,j represents coordinates around current */
			for (j = 1; j <= 3; j++) {
				if (j==2 && nn== 2) continue;
				ii = hits[mm][1]+nn-2;
				jj = hits[mm][2]+j-2;
				if (ii < 1 || ii > 10 || jj < 1 || jj > 10) continue;
				iquad = quadsst[ii][jj];
				switch (iquad) {
//					case IHDOT:	/* Empty space ends reaction
//					case IHQUEST:
//					case IHBLANK:
//					case IHT:
//					case IHWEB:
					default:
						break;
					case IHSTAR: /* Affect another star */
						if (Rand() < 0.05) {
							/* This star supernovas */
							snova(ii,jj);
							return;
						}
						top2++;
						hits[top2][1]=ii;
						hits[top2][2]=jj;
						dsst.galaxy[quadx][quady] -= 1;
						dsst.starkl++;
						crmena(1, IHSTAR, 2, ii, jj);
						prout(" novas.");
						quadsst[ii][jj] = IHDOT;
						break;
					case IHP: /* Destroy planet */
						dsst.newstuf[quadx][quady] -= 1;
						dsst.nplankl++;
						crmena(1, IHP, 2, ii, jj);
						prout(" destroyed.");
						dsst.plnets[iplnet] = nulplanet;
						iplnet = plnetx = plnety = 0;
						if (landed == 1) {
							finish(FPNOVA);
							return;
						}
						quadsst[ii][jj] = IHDOT;
						break;
					case IHB: /* Destroy base */
						dsst.galaxy[quadx][quady] -= 10;
						for (i = 1; i <= dsst.rembase; i++)
							if (dsst.baseqx[i]==quadx && dsst.baseqy[i]==quady) break;
						dsst.baseqx[i] = dsst.baseqx[dsst.rembase];
						dsst.baseqy[i] = dsst.baseqy[dsst.rembase];
						dsst.rembase--;
						basex = basey = 0;
						dsst.basekl++;
						newcnd();
						crmena(1, IHB, 2, ii, jj);
						prout(" destroyed.");
						quadsst[ii][jj] = IHDOT;
						break;
					case IHE: /* Buffet ship */
					case IHF:
						prout("***Starship buffeted by nova.");
						if (shldup) {
							if (shield >= 2000.0) shield -= 2000.0;
							else {
								double diff = 2000.0 - shield;
								energy -= diff;
								shield = 0.0;
								shldup = 0;
								prout("***Shields knocked out.");
								damage[DSHIELD] += 0.005*damfac*Rand()*diff;
							}
						}
						else energy -= 2000.0;
						if (energy <= 0) {
							finish(FNOVA);
							return;
						}
						/* add in course nova contributes to kicking starship*/
						icx += sectx-hits[mm][1];
						icy += secty-hits[mm][2];
						kount++;
						break;
					case IHK: /* kill klingon */
						deadkl(ii,jj,iquad, ii, jj);
						break;
					case IHC: /* Damage/destroy big enemies */
					case IHS:
					case IHR:
						for (ll = 1; ll <= nenhere; ll++)
							if (kx[ll]==ii && ky[ll]==jj) break;
						kpower[ll] -= 800.0; /* If firepower is lost, die */
						if (kpower[ll] <= 0.0) {
							deadkl(ii, jj, iquad, ii, jj);
							break;
						}
						newcx = ii + ii - hits[mm][1];
						newcy = jj + jj - hits[mm][2];
						crmena(1, iquad, 2, ii, jj);
						proutn(" damaged");
						if (newcx<1 || newcx>10 || newcy<1 || newcy>10) {
							/* can't leave quadrant */
							skip(1);
							break;
						}
						iquad1 = quadsst[newcx][newcy];
						if (iquad1 == IHBLANK) {
							proutn(", blasted into ");
							crmena(0, IHBLANK, 2, newcx, newcy);
							skip(1);
							deadkl(ii, jj, iquad, newcx, newcy);
							break;
						}
						if (iquad1 != IHDOT) {
							/* can't move into something else */
							skip(1);
							break;
						}
						proutn(", buffeted to");
						cramlc(2, newcx, newcy);
						quadsst[ii][jj] = IHDOT;
						quadsst[newcx][newcy] = iquad;
						kx[ll] = newcx;
						ky[ll] = newcy;
						kavgd[ll] = sqrt(square(sectx-newcx)+square(secty-newcy));
						kdist[ll] = kavgd[ll];
						skip(1);
						break;
				}
			}
		if (top == top2) break;
		bot = top + 1;
		top = top2;
	}
	if (kount==0) return;

	/* Starship affected by nova -- kick it away. */
	dist = kount*0.1;
	if (icx) icx = (icx < 0 ? -1 : 1);
	if (icy) icy = (icy < 0 ? -1 : 1);
	direc = course[3*(icx+1)+icy+2];
	if (direc == 0.0) dist = 0.0;
	if (dist == 0.0) return;
	Time = 10.0*dist/16.0;
	skip(1);
	prout("Force of nova displaces starship.");
	iattak=2;	/* Eliminates recursion problem */
	lmove();
	Time = 10.0*dist/16.0;
	return;
}
	
	
void snova(int insx, int insy) {
	int comdead, nqx, nqy, nsx, nsy, num, kldead, iscdead;
	int nrmdead, npdead;
	int insipient=0;

	nsx = insx;
	nsy = insy;

	if (insy== 0) {
		if (insx == 1) {
			/* NOVAMAX being used */
			nqx = probecx;
			nqy = probecy;
		}
		else {
			int stars = 0;
			/* Scheduled supernova -- select star */
			/* logic changed here so that we won't favor quadrants in top
			left of universe */
			for (nqx = 1; nqx<=8; nqx++) {
				for (nqy = 1; nqy<=8; nqy++) {
					stars += dsst.galaxy[nqx][nqy] % 10;
				}
			}
			if (stars == 0) return; /* nothing to supernova exists */
			num = Rand()*stars + 1;
			for (nqx = 1; nqx<=8; nqx++) {
				for (nqy = 1; nqy<=8; nqy++) {
					num -= dsst.galaxy[nqx][nqy] % 10;
					if (num <= 0) break;
				}
				if (num <=0) break;
			}
#ifdef DEBUG
			if (idebug) {
				proutn("Super nova here?");
				if (ja()==1) {
					nqx = quadx;
					nqy = quady;
				}
			}
#endif
		}

		if (nqx != quady || nqy != quady || justin != 0) {
			/* it isn't here, or we just entered (treat as inroute) */
			if (REPORTS) {
				skip(1);
				proutn("Message from Starfleet Command       Stardate ");
				cramf(dsst.date, 0, 1);
				skip(1);
				proutn("     Supernova in");
				cramlc(1, nqx, nqy);
				prout("; caution advised.");
			}
		}
		else {
			/* we are in the quadrant! */
			insipient = 1;
			num = Rand()* (dsst.galaxy[nqx][nqy]%10) + 1;
			for (nsx=1; nsx < 10; nsx++) {
				for (nsy=1; nsy < 10; nsy++) {
					if (quadsst[nsx][nsy]==IHSTAR) {
						num--;
						if (num==0) break;
					}
				}
				if (num==0) break;
			}
		}
	}
	else {
		insipient = 1;
	}

	if (insipient) {
		skip(1);
		prouts("***RED ALERT!  RED ALERT!");
		skip(1);
		proutn("***Incipient supernova detected at");
		cramlc(2, nsx, nsy);
		skip(1);
		nqx = quadx;
		nqy = quady;
		if (square(nsx-sectx) + square(nsy-secty) <= 2.1) {
			proutn("Emergency override attempts t");
			prouts("***************");
			skip(1);
			stars();
			alldone=1;
		}
	}
	/* destroy any Klingons in supernovaed quadrant */
	num=dsst.galaxy[nqx][nqy];
    kldead = num/100;
    dsst.remkl -= kldead; // Moved here to correctly set remaining Klingon count
	comdead = iscdead = 0;
	if (nqx==dsst.isx && nqy == dsst.isy) {
		/* did in the Supercommander! */
		dsst.nscrem = dsst.isx = dsst.isy = isatb = iscate = 0;
		iscdead = 1;
		kldead--; /* Get proper kill credit */
		future[FSCMOVE] = future[FSCDBAS] = 1e30;
	}

    if (dsst.remcom) {
		int maxloop = dsst.remcom, l;
		for (l = 1; l <= maxloop; l++) {
			if (dsst.cx[l] == nqx && dsst.cy[l] == nqy) {
				dsst.cx[l] = dsst.cx[dsst.remcom];
				dsst.cy[l] = dsst.cy[dsst.remcom];
				dsst.cx[dsst.remcom] = dsst.cy[dsst.remcom] = 0;
				dsst.remcom--;
				kldead--;
				comdead++;
				if (dsst.remcom==0) future[FTBEAM] = 1e30;
				break;
			}
		}
	}
	/* destroy Romulans and planets in supernovaed quadrant */
	num = dsst.newstuf[nqx][nqy];
	dsst.newstuf[nqx][nqy] = 0;
	nrmdead = num/10;
	dsst.nromrem -= nrmdead;
	npdead = num - nrmdead*10;
	if (npdead) {
		int l;
		for (l = 1; l <= inplan; l++)
			if (dsst.plnets[l].x == nqx && dsst.plnets[l].y == nqy) {
				dsst.plnets[l] = nulplanet;
			}
	}
	/* Destroy any base in supernovaed quadrant */
	if (dsst.rembase) {
		int maxloop = dsst.rembase, l;
		for (l = 1; l <= maxloop; l++)
			if (dsst.baseqx[l]==nqx && dsst.baseqy[l]==nqy) {
				dsst.baseqx[l] = dsst.baseqx[dsst.rembase];
				dsst.baseqy[l] = dsst.baseqy[dsst.rembase];
				dsst.baseqx[dsst.rembase] = dsst.baseqy[dsst.rembase] = 0;
				dsst.rembase--;
				break;
			}
	}
	/* If starship caused supernova, tally up destruction */
	if (insx) {
		num = dsst.galaxy[nqx][nqy] % 100;
		dsst.starkl += num % 10;
		dsst.basekl += num/10;
		dsst.killk += kldead;
		dsst.killc += comdead;
		dsst.nromkl += nrmdead;
		dsst.nplankl += npdead;
		dsst.nsckill += iscdead;
	}
	/* mark supernova in galaxy and in star chart */
	if ((quadx == nqx && quady == nqy) || REPORTS)
		starch[nqx][nqy] = 1;
	dsst.galaxy[nqx][nqy] = 1000;
	/* If supernova destroys last klingons give special message */
	if (dsst.remkl==0 && (nqx != quadx || nqy != quady)) {
		skip(2);
		if (insx == 0) prout("Lucky you!");
		proutn("A supernova in");
		cramlc(1, nqx, nqy);
		prout(" has just destroyed the last Klingons.");
		finish(FWON);
		return;
	}
	/* if some Klingons remain, continue or die in supernova */
	if (alldone) finish(FSNOVAED);
	return;
}
		
				
