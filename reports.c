#include "sst.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

void attakreport(void) {
    if (future[FCDBAS] < 1e30) {
        proutn(YEL "Starbase in ");
        cramlc(1, batx, baty);
        prout(" is currently under attack.");
        proutn("It can hold out until Stardate ");
        cramf(future[FCDBAS], 0,1);
        prout("." RESET);
    }
    if (isatb == 1) {
        proutn(YEL "Starbase in ");
        cramlc(1, dsst.isx, dsst.isy);
        prout(" is under Super-commander attack.");
        proutn("It can hold out until Stardate ");
        cramf(future[FSCDBAS], 0, 1);
        prout("." RESET);
    }
}


void report(int f) {
    char *s1,*s2,*s3;

    chew();
    s1 = (thawed?"thawed ":"");
    switch (length) {
        case 1: s2="short"; break;
        case 2: s2="medium"; break;
        case 4: s2="long"; break;
        default: s2="unknown length"; break;
    }
    switch (skill) {
        case SNOVICE: s3="novice"; break;
        case SFAIR: s3="fair"; break;
        case SGOOD: s3="good"; break;
        case SEXPERT: s3="expert"; break;
        case SEMERITUS: s3="emeritus"; break;
        default: s3="skilled"; break;
    }
    printf("\nYou %s playing a %s%s %s game.\n",
            alldone? "were": "are now", s1, s2, s3);
    if (skill>SGOOD && thawed && !alldone) prout("No plaque is allowed.");
    if (tourn) printf("This is tournament game %d.\n", tourn);
    if (f) printf("Your secret password is \"%s\"\n",passwd);
    printf("%d of %d Klingon ships have been destroyed",
            dsst.killk+dsst.killc+dsst.nsckill, inkling);
    if (dsst.killc) printf(", including %d Commander%s.\n", dsst.killc, dsst.killc==1?"":"s");
    else if (dsst.killk+dsst.nsckill > 0) prout(", but no Commanders.");
    else prout(".");
    if (skill > SFAIR) printf("The Super Commander has %sbeen destroyed.\n",
            dsst.nscrem?"not ":"");
    if (dsst.rembase != inbase) {
        proutn("There ");
        if (inbase-dsst.rembase==1) proutn("has been 1 base");
        else {
            proutn("have been ");
            crami(inbase-dsst.rembase, 1);
            proutn(" bases");
        }
        proutn(" destroyed, ");
        crami(dsst.rembase, 1);
        prout(" remaining.");
    }
    else printf("There are %d bases.\n", inbase);
    if (REPORTS || iseenit) {
        /* Don't report this if not seen and
           either the radio is dead or not at base! */
        attakreport();
        iseenit = 1;
    }
    if (casual) printf("%d casualt%s suffered so far.\n",
            casual, casual==1? "y" : "ies");
#ifdef CAPTURE
    if (brigcapacity != brigfree) printf("%d Klingon%s in brig.\n",
            brigcapacity-brigfree, brigcapacity-brigfree>1 ? "s" : "");
    if (kcaptured > 0) printf("%d captured Klingon%s turned in to Star Fleet.\n", 
            kcaptured, kcaptured>1 ? "s" : "");
#endif
    if (nhelp) printf("There were %d call%s for help.\n",
            nhelp, nhelp==1 ? "" : "s");
    if (ship == IHE) {
        proutn("You have ");
        if (nprobes) crami(nprobes,1);
        else proutn("no");
        proutn(" deep space probe");
        if (nprobes!=1) proutn("s");
        prout(".");
    }
    if (REPORTS && future[FDSPROB] != 1e30) {
        if (isarmed) 
            proutn("An armed deep space probe is in");
        else
            proutn("A deep space probe is in");
        cramlc(1, probecx, probecy);
        prout(".");
    }
    if (icrystl) {
        if (cryprob <= .05)
            prout("Dilithium crystals aboard ship...not yet used.");
        else {
            int i=0;
            double ai = 0.05;
            while (cryprob > ai) {
                ai *= 2.0;
                i++;
            }
            printf("Dilithium crystals have been used %d time%s.\n",
                    i, i==1? "" : "s");
        }
    }
    skip(1);
}

void lrscan(void) {
    int x, y;
    chew();
    if (damage[DLRSENS] != 0.0) {
        /* Now allow base's sensors if docked */
        if (condit != IHDOCKED) {
            prout(RED "LONG-RANGE SENSORS DAMAGED." RESET);
            return;
        }
        skip(1);
        proutn("Starbase's long-range scan for");
    }
    else {
        skip(1);
        proutn("Long-range scan for");
    }
    cramlc(1, quadx, quady);
    skip(1);
    for (x = quadx-1; x <= quadx+1; x++) {
        for (y = quady-1; y <= quady+1; y++) {
            if (x == 0 || x > 8 || y == 0 || y > 8)
                printf("   -1");
            else {
                if (dsst.galaxy[x][y] > 999)
                    printf(RED "%5d" RESET, dsst.galaxy[x][y]);
                else if (dsst.galaxy[x][y] >= 100)
                    printf(YEL "%5d" RESET, dsst.galaxy[x][y]);
                else
                    printf("%5d", dsst.galaxy[x][y]);
                // If radio works, mark star chart so
                // it will show current information.
                // Otherwise mark with current
                // value which is fixedsst. 
                starch[x][y] = damage[DRADIO] > 0 ? dsst.galaxy[x][y]+1000 :1;
            }
        }
        putchar('\n');
    }
}

void dreprt(void) {
    int jdam = FALSE, i;
    chew();

    for (i = 1; i <= ndevice; i++) {
        if (damage[i] > 0.0) {
            if (!jdam) {
                skip(1);
                prout("DEVICE            -REPAIR TIMES-");
                prout("                IN FLIGHT   DOCKED");
                jdam = TRUE;
            }
            printf("  %16s ", device[i]);
            cramf(damage[i]+0.05, 8, 2);
            proutn("  ");
            cramf(docfac*damage[i]+0.005, 8, 2);
            skip(1);
        }
    }
    if (!jdam) prout(GRN "All devices functional." RESET);
}

void chart(int nn) {
    int i,j;

    chew();
    skip(1);
    if (stdamtim != 1e30 && stdamtim != dsst.date && condit == IHDOCKED) {
        prout("Spock-  \"I revised the Star Chart from the");
        prout("  starbase's records.\"");
        skip(1);
    }
    if (nn == 0) prout("STAR CHART FOR THE KNOWN GALAXY");
    if (stdamtim != 1e30) {
        if (condit == IHDOCKED) {
            /* We are docked, so restore chart from base information -- these values won't update! */
            stdamtim = dsst.date;
            for (i=1; i <= 8 ; i++)
                for (j=1; j <= 8; j++)
                    if (starch[i][j] == 1) starch[i][j] = dsst.galaxy[i][j]+1000;
        }
        else {
            proutn("(Last surveillance update ");
            cramf(dsst.date-stdamtim, 0, 1);
            prout(" stardates ago.)");
        }
    }
    if (nn ==0) skip(1);

    prout("      1    2    3    4    5    6    7    8");
    prout("    ----------------------------------------");
    if (nn==0) prout("  -");
        for (i = 1; i <= 8; i++) {
            printf("%d -", i);
            for (j = 1; j <= 8; j++) {
                if (starch[i][j] < 0) // We know only about the bases
                    printf("  .1.");
                else if (starch[i][j] == 0) // Unknown
                    printf("  ...");
                else if (starch[i][j] > 999) {  // Memorized value
                    //printf("%5d", starch[i][j]-1000);
                    if ((starch[i][j]-1000) > 999) 
                        printf(RED "%5d" RESET, starch[i][j]-1000); 
                    else if ((starch[i][j]-1000) >= 100) 
                        printf(YEL "%5d" RESET, starch[i][j]-1000); 
                    else
                        printf(GRN "%5d" RESET, starch[i][j]-1000); 
                }
                else {
                    //printf("%5d", dsst.galaxy[i][j]); 
                    if (dsst.galaxy[i][j] > 999) 
                        printf(RED "%5d" RESET, dsst.galaxy[i][j]); 
                    else if (dsst.galaxy[i][j] >= 100) 
                        printf(YEL "%5d" RESET, dsst.galaxy[i][j]); 
                    else
                        printf(GRN "%5d" RESET, dsst.galaxy[i][j]); 
                }
            }
            prout("  -");
        }
    if (nn == 0) {
        skip(1);
        crmshp();
        proutn(" is currently in");
        cramlc(1, quadx, quady);
        skip(1);
    }
}


void colorPrint(char scanItem)
{
    switch (scanItem) {
        case IHR:
            printf(YEL);
            break;
        case IHK:
            printf(RED);
            break;
        case IHC:
            printf(RED);
            break;
        case IHS:
            printf(RED);
            break;
        case IHP:
            printf(GRN);
            break;
        case IHB:
            printf(BLU);
            break;
        case IHE:
            printf(BLU);
            break;
        case IHF:
            printf(BLU);
            break;
        case IHT:
            printf(MAG);
            break;
        case IHWEB:
            printf(MAG);
            break;
        case IHBLANK:
            printf(CYN);
            break;
        case IHSTAR:
            printf(CYN);
            break;
        default:
            break;
    }
    printf("%c ", scanItem);
    printf(RESET);
}


void srscan(int l) {
    static char requests[][3] =
    {"","da","co","po","ls","wa","en","to","sh","kl","ti"};
    char *cp;
    int leftside=TRUE, rightside=TRUE, i, j, jj, k=0, nn=FALSE;
    int goodScan=TRUE;
    switch (l) {
        case 1: // SRSCAN
            if (damage[DSRSENS] != 0) {
                /* Allow base's sensors if docked */
                if (condit != IHDOCKED) {
                    prout(RED "SHORT-RANGE SENSORS DAMAGED" RESET);
                    goodScan=FALSE;
                }
                else
                    prout(YEL "[Using starbase's sensors]" RESET);
            }
            if (goodScan)
                starch[quadx][quady] = damage[DRADIO]>0.0 ?
                    dsst.galaxy[quadx][quady]+1000:1;
            scan();
            if (isit("chart")) nn = TRUE;
            if (isit("no")) rightside = FALSE;
            chew();
            prout("\n    1 2 3 4 5 6 7 8 9 10");
            break;
        case 2: // REQUEST
            while (scan() == IHEOL)
                printf("Information desired? ");
            chew();
            for (k = 1; k <= 10; k++)
                if (strncmp(citem,requests[k],minsst(2,strlen(citem)))==0)
                    break;
            if (k > 10) {
                prout("UNRECOGNIZED REQUEST. Legal requests are:\n"
                        "  date, condition, position, lsupport, warpfactor,\n"
                        "  energy, torpedoes, shields, klingons, time.");
                return;
            }
            // no "break"
        case 3: // STATUS
            chew();
            leftside = FALSE;
            skip(1);
    }
    for (i = 1; i <= 10; i++) {
        int jj = (k!=0 ? k : i);
        if (leftside) {
            printf("%2d  ", i);
            for (j = 1; j <= 10; j++) {
                if (goodScan || (abs(i-sectx)<= 1 && abs(j-secty) <= 1))
                    colorPrint(quadsst[i][j]);
                //printf("%c ",quadsst[i][j]);
                else
                    printf("- ");
            }
        }
        if (rightside) {
            switch (jj) {
                case 1:
                    printf(" Stardate      %.1f", dsst.date);
                    break;
                case 2:
                    if (condit != IHDOCKED) newcnd();
                    switch (condit) {
                        case IHRED: cp = RED "RED" RESET; break;
                        case IHGREEN: cp = GRN "GREEN" RESET; break;
                        case IHYELLOW: cp = YEL "YELLOW" RESET; break;
                        case IHDOCKED: cp = WHT "DOCKED" RESET; break;
                    }
                    printf(" Condition     %s", cp);
#ifdef CLOAKING
                    if (iscloaked) printf(", CLOAKED");
#endif
                    break;
                case 3:
                    printf(" Position     ");
                    cramlc(0, quadx, quady);
                    putchar(',');
                    cramlc(0, sectx, secty);
                    break;
                case 4:
                    printf(" Life Support  ");
                    if (damage[DLIFSUP] != 0.0) {
                        if (condit == IHDOCKED)
                            printf(YEL "DAMAGED, supported by starbase" RESET);
                        else
                            printf(RED "DAMAGED, reserves=%4.2f" RESET, lsupres);
                    }
                    else
                        printf("ACTIVE");
                    break;
                case 5:
                    printf(" Warp Factor   %.1f", warpfac);
                    break;
                case 6:
                    printf(" Energy        %.2f", energy);
                    break;
                case 7:
                    printf(" Torpedoes     %d", torps);
                    break;
                case 8:
                    printf(" Shields       ");
                    if (damage[DSHIELD] != 0)
                        printf(YEL "DAMAGED," RESET);
                    else if (shldup)
                        printf(GRN "UP," RESET);
                    else
                        printf(RED "DOWN," RESET);
                    printf(" %d%% %.1f units",
                            (int)((100.0*shield)/inshld + 0.5), shield);
                    break;
                case 9:
                    printf(" Klingons Left %d", dsst.remkl);
                    break;
                case 10:
                    printf(" Time Left     %.2f", dsst.remtime);
                    break;
            }

        }
        skip(1);
        if (k!=0) return;
    }
    if (nn) chart(1);
}


void eta(void) {
    int key, ix1, ix2, iy1, iy2, prompt=FALSE;
    int wfl;
    double ttime, twarp, tpower;
    if (damage[DCOMPTR] != 0.0) {
        prout(RED "COMPUTER DAMAGED, USE A POCKET CALCULATOR." RESET);
        skip(1);
        return;
    }
    if (scan() != IHREAL) {
        prompt = TRUE;
        chew();
        proutn("Destination quadrant and/or sector? ");
        if (scan()!=IHREAL) {
            huh();
            return;
        }
    }
    iy1 = aaitem +0.5;
    if (scan() != IHREAL) {
        huh();
        return;
    }
    ix1 = aaitem + 0.5;
    if (scan() == IHREAL) {
        iy2 = aaitem + 0.5;
        if (scan() != IHREAL) {
            huh();
            return;
        }
        ix2 = aaitem + 0.5;
    }
    else {	// same quadrant
        ix2 = ix1;
        iy2 = iy1;
        ix1 = quady;	// ya got me why x and y are reversed!
        iy1 = quadx;
    }

    if (ix1 > 8 || ix1 < 1 || iy1 > 8 || iy1 < 1 ||
            ix2 > 10 || ix2 < 1 || iy2 > 10 || iy2 < 1) {
        huh();
        return;
    }
    dist = sqrt(square(iy1-quadx+0.1*(iy2-sectx))+
            square(ix1-quady+0.1*(ix2-secty)));
    wfl = FALSE;

    if (prompt) prout("Answer \"no\" if you don't know the value:");
    while (TRUE) {
        chew();
        proutn("Time or arrival date? ");
        if (scan()==IHREAL) {
            ttime = aaitem;
            if (ttime > dsst.date) ttime -= dsst.date; // Actually a star date
            if (ttime <= 1e-10 ||
                    (twarp=(floor(sqrt((10.0*dist)/ttime)*10.0)+1.0)/10.0) > 10) {
                prout("We'll never make it, sir.");
                chew();
                return;
            }
            if (twarp < 1.0) twarp = 1.0;
            break;
        }
        chew();
        proutn("Warp factor? ");
        if (scan()== IHREAL) {
            wfl = TRUE;
            twarp = aaitem;
            if (twarp<1.0 || twarp > 10.0) {
                huh();
                return;
            }
            break;
        }
        prout("Captain, certainly you can give me one of these.");
    }
    while (TRUE) {
        chew();
        ttime = (10.0*dist)/square(twarp);
        tpower = dist*twarp*twarp*twarp*(shldup+1);
        if (tpower >= energy) { // Suggestion from Ethan Staffin -- give amount needed
            prout("Insufficient energy, sir: we would need ");
            cramf(tpower, 1, 1);
            proutn (" units.");
            if (shldup==0 || tpower > energy*2.0) {
                if (!wfl) return;
                proutn("New warp factor to try? ");
                if (scan() == IHREAL) {
                    wfl = TRUE;
                    twarp = aaitem;
                    if (twarp<1.0 || twarp > 10.0) {
                        huh();
                        return;
                    }
                    continue;
                }
                else {
                    chew();
                    skip(1);
                    return;
                }
            }
            prout("But if you lower your shields,");
            proutn("remaining");
            tpower /= 2;
        }
        else
            proutn("Remaining");
        proutn(" energy will be ");
        cramf(energy-tpower, 1, 1);
        prout(".");
        if (wfl) {
            proutn("And we will arrive at stardate ");
            cramf(dsst.date+ttime, 1, 1);
            prout(".");
        }
        else if (twarp==1.0)
            prout("Any warp speed is adequate.");
        else {
            proutn("Minimum warp needed is ");
            cramf(twarp, 1, 2);
            skip(1);
            proutn("and we will arrive at stardate ");
            cramf(dsst.date+ttime, 1, 2);
            prout(".");
        }
        if (dsst.remtime < ttime)
            prout("Unfortunately, the Federation will be destroyed by then.");
        if (twarp > 6.0)
            prout("You'll be taking risks at that speed, Captain");
        if ((isatb==1 && dsst.isy == ix1 && dsst.isx == iy1 &&
                    future[FSCDBAS]< ttime+dsst.date)||
                (future[FCDBAS]<ttime+dsst.date && baty==ix1 && batx == iy1))
            prout("The starbase there will be destroyed by then.");
        proutn("New warp factor to try? ");
        if (scan() == IHREAL) {
            wfl = TRUE;
            twarp = aaitem;
            if (twarp<1.0 || twarp > 10.0) {
                huh();
                return;
            }
        }
        else {
            chew();
            skip(1);
            return;
        }
    }

}
