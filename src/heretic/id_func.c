//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2014-2017 RestlessRodent
// Copyright(C) 2015-2018 Fabian Greffrath
// Copyright(C) 2016-2023 Julia Nechaevskaya
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//


#include <stdio.h>
#include <stdint.h>

#include "i_timer.h"
#include "m_misc.h"
#include "v_trans.h"
#include "v_video.h"
#include "doomdef.h"
#include "p_local.h"
#include "r_local.h"

#include "id_vars.h"
#include "id_func.h"


// =============================================================================
//
//                        Render Counters and Widgets
//
// =============================================================================

ID_Render_t IDRender;
ID_Widget_t IDWidget;

char ID_Level_Time[64];
char ID_Total_Time[64];
char ID_Local_Time[64];

enum
{
    widget_kills,
    widget_items,
    widget_secret
} widgetcolor_t;

static byte *ID_WidgetColor (const int i)
{
    switch (i)
    {
        case widget_kills:
        {
            return
                IDWidget.totalkills == 0 ? cr[CR_GREEN] :
                IDWidget.kills == 0 ? cr[CR_RED] :
                IDWidget.kills < IDWidget.totalkills ? cr[CR_YELLOW] : cr[CR_GREEN];
            break;
        }
        case widget_items:
        {
            return
                IDWidget.totalitems == 0 ? cr[CR_GREEN] :
                IDWidget.items == 0 ? cr[CR_RED] :
                IDWidget.items < IDWidget.totalitems ? cr[CR_YELLOW] : cr[CR_GREEN];
            break;
        }
        case widget_secret:
        {
            return
                IDWidget.totalsecrets == 0 ? cr[CR_GREEN] :
                IDWidget.secrets == 0 ? cr[CR_RED] :
                IDWidget.secrets < IDWidget.totalsecrets ? cr[CR_YELLOW] : cr[CR_GREEN];
            break;
        }
    }
    return NULL;
}

// -----------------------------------------------------------------------------
// ID_LeftWidgets.
//  [JN] Draw all the widgets and counters.
// -----------------------------------------------------------------------------

void ID_LeftWidgets (void)
{
    //
    // Located on the top
    //
    if (widget_location == 1)
    {
        // K/I/S stats
        if (widget_kis == 1
        || (widget_kis == 2 && automapactive))
        {
            if (!deathmatch)
            {
                char str1[16];  // kills
                char str2[16];  // items
                char str3[16];  // secret

                // Kills:
                MN_DrTextA("K:", 0 - WIDESCREENDELTA, 10, cr[CR_GRAY]);
                /*
                // TODO - extra kills?
                if (IDWidget.extrakills)
                {
                    sprintf(str1, "%d+%d/%d ", IDWidget.kills, IDWidget.extrakills, IDWidget.totalkills);
                }
                else
                */
                {
                    sprintf(str1, "%d/%d ", IDWidget.kills, IDWidget.totalkills);
                }
                MN_DrTextA(str1, 0 - WIDESCREENDELTA + 16, 10, ID_WidgetColor(widget_kills));

                // Items:
                MN_DrTextA("I:", 0 - WIDESCREENDELTA, 20, cr[CR_GRAY]);
                sprintf(str2, "%d/%d ", IDWidget.items, IDWidget.totalitems);
                MN_DrTextA(str2, 0 - WIDESCREENDELTA + 16, 20, ID_WidgetColor(widget_items));

                // Secret:
                MN_DrTextA("S:", 0 - WIDESCREENDELTA, 30, cr[CR_GRAY]);
                sprintf(str3, "%d/%d ", IDWidget.secrets, IDWidget.totalsecrets);
                MN_DrTextA(str3, 0 - WIDESCREENDELTA + 16, 30, ID_WidgetColor(widget_secret));
            }
            else
            {
                char str1[16] = {0};  // Green
                char str2[16] = {0};  // Yellow
                char str3[16] = {0};  // Red
                char str4[16] = {0};  // Blue

                // Green
                if (playeringame[0])
                {
                    MN_DrTextA("G:", 0 - WIDESCREENDELTA, 10, cr[CR_GREEN]);
                    sprintf(str1, "%d", IDWidget.frags_g);
                    MN_DrTextA(str1, 0 - WIDESCREENDELTA + 16, 10, cr[CR_GREEN]);
                }
                // Yellow
                if (playeringame[1])
                {
                    MN_DrTextA("Y:", 0 - WIDESCREENDELTA, 20, cr[CR_YELLOW]);
                    sprintf(str2, "%d", IDWidget.frags_i);
                    MN_DrTextA(str2, 0 - WIDESCREENDELTA + 16, 20, cr[CR_YELLOW]);
                }
                // Red
                if (playeringame[2])
                {
                    MN_DrTextA("R:", 0 - WIDESCREENDELTA, 30, cr[CR_RED]);
                    sprintf(str3, "%d", IDWidget.frags_b);
                    MN_DrTextA(str3, 0 - WIDESCREENDELTA + 16, 30, cr[CR_RED]);
                }
                // Blue
                if (playeringame[3])
                {
                    MN_DrTextA("B:", 0 - WIDESCREENDELTA, 40, cr[CR_RED]);
                    sprintf(str4, "%d", IDWidget.frags_r);
                    MN_DrTextA(str4, 0 - WIDESCREENDELTA + 16, 40, cr[CR_RED]);
                }
            }
        }

        // Level / DeathMatch timer. Time gathered in G_Ticker.
        if (widget_time == 1
        || (widget_time == 2 && automapactive))
        {
            MN_DrTextA("TIME", 0 - WIDESCREENDELTA, 40, cr[CR_GRAY]);
            MN_DrTextA(ID_Level_Time, 0 - WIDESCREENDELTA, 50, cr[CR_LIGHTGRAY]);
        }

        // Total time. Time gathered in G_Ticker.
        if (widget_totaltime == 1
        || (widget_totaltime == 2 && automapactive))
        {
            MN_DrTextA("TOTAL", 0 - WIDESCREENDELTA, 60, cr[CR_GRAY]);
            MN_DrTextA(ID_Total_Time, 0 - WIDESCREENDELTA, 70, cr[CR_LIGHTGRAY]);
        }

        // Player coords
        if (widget_coords == 1
        || (widget_coords == 2 && automapactive))
        {
            char str[128];
            
            MN_DrTextA("X:", 0 - WIDESCREENDELTA, 80, cr[CR_GRAY]);
            MN_DrTextA("Y:", 0 - WIDESCREENDELTA, 90, cr[CR_GRAY]);
            MN_DrTextA("ANG:", 0 - WIDESCREENDELTA, 100, cr[CR_GRAY]);

            sprintf(str, "%d", IDWidget.x);
            MN_DrTextA(str, 0 - WIDESCREENDELTA + 16, 80, cr[CR_GREEN]);
            sprintf(str, "%d", IDWidget.y);
            MN_DrTextA(str, 0 - WIDESCREENDELTA + 16, 90, cr[CR_GREEN]);
            sprintf(str, "%d", IDWidget.ang);
            MN_DrTextA(str, 0 - WIDESCREENDELTA + 32, 100, cr[CR_GREEN]);
        }

        // Render counters
        if (widget_render)
        {
            char spr[32];
            char seg[32];
            char opn[64];
            char vis[32];

            // Sprites
            MN_DrTextA("SPR:", 0 - WIDESCREENDELTA, 110, cr[CR_GRAY]);
            M_snprintf(spr, 16, "%d", IDRender.numsprites);
            MN_DrTextA(spr, 32 - WIDESCREENDELTA, 110, cr[CR_GREEN]);

            // Segments
            MN_DrTextA("SEG:", 0 - WIDESCREENDELTA, 120, cr[CR_GRAY]);
            M_snprintf(seg, 16, "%d", IDRender.numsegs);
            MN_DrTextA(seg, 32 - WIDESCREENDELTA, 120, cr[CR_GREEN]);

            // Openings
            MN_DrTextA("OPN:", 0 - WIDESCREENDELTA, 130, cr[CR_GRAY]);
            M_snprintf(opn, 16, "%d", IDRender.numopenings);
            MN_DrTextA(opn, 32 - WIDESCREENDELTA, 130, cr[CR_GREEN]);

            // Planes
            MN_DrTextA("PLN:", 0 - WIDESCREENDELTA, 140, cr[CR_GRAY]);
            M_snprintf(vis, 32, "%d", IDRender.numplanes);
            MN_DrTextA(vis, 32 - WIDESCREENDELTA, 140, cr[CR_GREEN]);
        }
    }
    //
    // Located at the bottom
    //
    else
    {
        int yy = 0;

        // Shift widgets one line up if Level Name widget is set to "always".
        if (widget_levelname && !automapactive)
        {
            yy -= 10;
        }

        // Render counters
        if (widget_render)
        {
            char spr[32];
            char seg[32];
            char opn[64];
            char vis[32];
            const int yy1 = widget_coords ? 0 : 45;

            // Sprites
            MN_DrTextA("SPR:", 0 - WIDESCREENDELTA, 25 + yy1, cr[CR_GRAY]);
            M_snprintf(spr, 16, "%d", IDRender.numsprites);
            MN_DrTextA(spr, 32 - WIDESCREENDELTA, 25 + yy1, cr[CR_GREEN]);

            // Segments
            MN_DrTextA("SEG:", 0 - WIDESCREENDELTA, 35 + yy1, cr[CR_GRAY]);
            M_snprintf(seg, 16, "%d", IDRender.numsegs);
            MN_DrTextA(seg, 32 - WIDESCREENDELTA, 35 + yy1, cr[CR_GREEN]);

            // Openings
            MN_DrTextA("OPN:", 0 - WIDESCREENDELTA, 45 + yy1, cr[CR_GRAY]);
            M_snprintf(opn, 16, "%d", IDRender.numopenings);
            MN_DrTextA(opn, 32 - WIDESCREENDELTA, 45 + yy1, cr[CR_GREEN]);

            // Planes
            MN_DrTextA("PLN:", 0 - WIDESCREENDELTA, 55 + yy1, cr[CR_GRAY]);
            M_snprintf(vis, 32, "%d", IDRender.numplanes);
            MN_DrTextA(vis, 32 - WIDESCREENDELTA, 55 + yy1, cr[CR_GREEN]);
        }

        // Player coords
        if (widget_coords == 1
        || (widget_coords == 2 && automapactive))
        {
            char str[128];

            MN_DrTextA("X:", 0 - WIDESCREENDELTA, 75, cr[CR_GRAY]);
            MN_DrTextA("Y:", 0 - WIDESCREENDELTA, 85, cr[CR_GRAY]);
            MN_DrTextA("ANG:", 0 - WIDESCREENDELTA, 95, cr[CR_GRAY]);

            sprintf(str, "%d", IDWidget.x);
            MN_DrTextA(str, 16 - WIDESCREENDELTA, 75, cr[CR_GREEN]);
            sprintf(str, "%d", IDWidget.y);
            MN_DrTextA(str, 16 - WIDESCREENDELTA, 85, cr[CR_GREEN]);
            sprintf(str, "%d", IDWidget.ang);
            MN_DrTextA(str, 32 - WIDESCREENDELTA, 95, cr[CR_GREEN]);
        }

        if (automapactive)
        {
            yy -= 10;
        }

        // K/I/S stats
        if (widget_kis == 1
        || (widget_kis == 2 && automapactive))
        {
            if (!deathmatch)
            {
                char str1[8], str2[16];  // kills
                char str3[8], str4[16];  // items
                char str5[8], str6[16];  // secret
        
                // Kills:
                sprintf(str1, "K ");
                MN_DrTextA(str1, 0 - WIDESCREENDELTA, 145 + yy, cr[CR_GRAY]);
                /*
                // TODO - extra kills?
                if (IDWidget.extrakills)
                {
                    sprintf(str2, "%d+%d/%d ", IDWidget.kills, IDWidget.extrakills, IDWidget.totalkills);
                }
                else
                */
                {
                    sprintf(str2, "%d/%d ", IDWidget.kills, IDWidget.totalkills);
                }
                MN_DrTextA(str2, 0 - WIDESCREENDELTA + MN_TextAWidth(str1), 145 + yy, ID_WidgetColor(widget_kills));

                // Items:
                sprintf(str3, "I ");
                MN_DrTextA(str3, 0 - WIDESCREENDELTA + MN_TextAWidth(str1) + MN_TextAWidth(str2), 145 + yy, cr[CR_GRAY]);

                sprintf(str4, "%d/%d ", IDWidget.items, IDWidget.totalitems);
                MN_DrTextA(str4, 0 - WIDESCREENDELTA     +
                                     MN_TextAWidth(str1) +
                                     MN_TextAWidth(str2) +
                                     MN_TextAWidth(str3), 145 + yy, ID_WidgetColor(widget_items));

                // Secret:
                sprintf(str5, "S ");
                MN_DrTextA(str5, 0 - WIDESCREENDELTA     +
                                     MN_TextAWidth(str1) +
                                     MN_TextAWidth(str2) +
                                     MN_TextAWidth(str3) +
                                     MN_TextAWidth(str4), 145 + yy, cr[CR_GRAY]);

                sprintf(str6, "%d/%d", IDWidget.secrets, IDWidget.totalsecrets);
                MN_DrTextA(str6, 0 - WIDESCREENDELTA     +
                                     MN_TextAWidth(str1) +
                                     MN_TextAWidth(str2) + 
                                     MN_TextAWidth(str3) +
                                     MN_TextAWidth(str4) +
                                     MN_TextAWidth(str5), 145 + yy, ID_WidgetColor(widget_secret));
            }
            else
            {
                char str1[8] = {0}, str2[16] = {0};  // Green
                char str3[8] = {0}, str4[16] = {0};  // Yellow
                char str5[8] = {0}, str6[16] = {0};  // Red
                char str7[8] = {0}, str8[16] = {0};  // Blue

                // Green
                if (playeringame[0])
                {
                    sprintf(str1, "G ");
                    MN_DrTextA(str1, 0 - WIDESCREENDELTA, 145 + yy, cr[CR_GREEN]);

                    sprintf(str2, "%d ", IDWidget.frags_g);
                    MN_DrTextA(str2, 0 - WIDESCREENDELTA +
                                         MN_TextAWidth(str1), 145 + yy, cr[CR_GREEN]);
                }
                // Yellow
                if (playeringame[1])
                {
                    sprintf(str3, "I ");
                    MN_DrTextA(str3, 0 - WIDESCREENDELTA +
                                         MN_TextAWidth(str1) +
                                         MN_TextAWidth(str2),
                                         145 + yy, cr[CR_YELLOW]);

                    sprintf(str4, "%d ", IDWidget.frags_i);
                    MN_DrTextA(str4, 0 - WIDESCREENDELTA +
                                         MN_TextAWidth(str1) +
                                         MN_TextAWidth(str2) +
                                         MN_TextAWidth(str3),
                                         145 + yy, cr[CR_YELLOW]);
                }
                // Red
                if (playeringame[2])
                {
                    sprintf(str5, "R ");
                    MN_DrTextA(str5, 0 - WIDESCREENDELTA +
                                         MN_TextAWidth(str1) +
                                         MN_TextAWidth(str2) +
                                         MN_TextAWidth(str3) +
                                         MN_TextAWidth(str4),
                                         145 + yy, cr[CR_RED]);

                    sprintf(str6, "%d ", IDWidget.frags_b);
                    MN_DrTextA(str6, 0 - WIDESCREENDELTA +
                                         MN_TextAWidth(str1) +
                                         MN_TextAWidth(str2) +
                                         MN_TextAWidth(str3) +
                                         MN_TextAWidth(str4) +
                                         MN_TextAWidth(str5),
                                         145 + yy, cr[CR_RED]);
                }
                // Blue
                if (playeringame[3])
                {
                    sprintf(str7, "B ");
                    MN_DrTextA(str7, 0 - WIDESCREENDELTA +
                                         MN_TextAWidth(str1) +
                                         MN_TextAWidth(str2) +
                                         MN_TextAWidth(str3) +
                                         MN_TextAWidth(str4) +
                                         MN_TextAWidth(str5) +
                                         MN_TextAWidth(str6),
                                         145 + yy, cr[CR_BLUE2]);

                    sprintf(str8, "%d ", IDWidget.frags_r);
                    MN_DrTextA(str8, 0 - WIDESCREENDELTA +
                                         MN_TextAWidth(str1) +
                                         MN_TextAWidth(str2) +
                                         MN_TextAWidth(str3) +
                                         MN_TextAWidth(str4) +
                                         MN_TextAWidth(str5) +
                                         MN_TextAWidth(str6) +
                                         MN_TextAWidth(str7),
                                         145 + yy, cr[CR_BLUE2]);
                }
            }
        }

        if (widget_kis)
        {
            yy -= 10;
        }

        // Total time. Time gathered in G_Ticker.
        if (widget_totaltime == 1
        || (widget_totaltime == 2 && automapactive))
        {
            char stra[8];

            sprintf(stra, "TOTAL ");
            MN_DrTextA(stra, 0 - WIDESCREENDELTA, 145 + yy, cr[CR_GRAY]);
            MN_DrTextA(ID_Total_Time, 0 - WIDESCREENDELTA + MN_TextAWidth(stra), 145 + yy, cr[CR_LIGHTGRAY]);
        }

        if (widget_totaltime)
        {
            yy -= 10;
        }

        // Level / DeathMatch timer. Time gathered in G_Ticker.
        if (widget_time == 1
        || (widget_time == 2 && automapactive))
        {
            char stra[8];

            sprintf(stra, "TIME ");
            MN_DrTextA(stra, 0 - WIDESCREENDELTA, 145 + yy, cr[CR_GRAY]);
            MN_DrTextA(ID_Level_Time, 0 - WIDESCREENDELTA + MN_TextAWidth(stra), 145 + yy, cr[CR_LIGHTGRAY]);
        }

    }
}


// -----------------------------------------------------------------------------
// Draws CRL stats.
//  [JN] Draw all the widgets and counters.
// -----------------------------------------------------------------------------

void CRL_StatDrawer (void)
{
    
    /*
    int yy = 0;
    int yy2 = 0;
    const int CRL_MAX_count_old = (int)(lastvisplane - visplanes);
    const int TotalVisPlanes = CRLData.numcheckplanes + CRLData.numfindplanes;

    // Count MAX visplanes for moving
    if (CRL_MAX_count_old > CRL_MAX_count)
    {
        // Set count
        CRL_MAX_count = CRL_MAX_count_old;
        // Set position and angle.
        // We have to account uncapped framerate for better precision.
        CRL_Get_MAX();
    }

    // Player coords
    if (crl_widget_coords)
    {
        char x[8] = {0}, xpos[128] = {0};
        char y[8] = {0}, ypos[128] = {0};
        char ang[128];

        M_snprintf(x, 8, "X: ");
        MN_DrTextA(x, 0, 30, cr[CR_GRAY]);
        M_snprintf(xpos, 128, "%d ", CRLWidgets.x);
        MN_DrTextA(xpos, MN_TextAWidth(x), 30, cr[CR_GREEN]);

        M_snprintf(y, 8, "Y: ");
        MN_DrTextA(y, MN_TextAWidth(x) + 
                      MN_TextAWidth(xpos), 30, cr[CR_GRAY]);
        M_snprintf(ypos, 128, "%d", CRLWidgets.y);
        MN_DrTextA(ypos, MN_TextAWidth(x) +
                         MN_TextAWidth(xpos) +
                         MN_TextAWidth(y), 30, cr[CR_GREEN]);

        MN_DrTextA("ANG:", 0, 40, cr[CR_GRAY]);
        M_snprintf(ang, 16, "%d", CRLWidgets.ang);
        MN_DrTextA(ang, 32, 40, cr[CR_GREEN]);
    }

    if (crl_widget_playstate)
    {
        if (crl_widget_playstate == 1
        || (crl_widget_playstate == 2 && CRL_plats_counter > CRL_MaxPlats))
        {
            char plt[32];

            MN_DrTextA("PLT:", 0, 53, CRL_StatColor_Str(CRL_plats_counter, CRL_MaxPlats));
            M_snprintf(plt, 16, "%d/%d", CRL_plats_counter, CRL_MaxPlats);
            MN_DrTextA(plt, 32, 53, CRL_StatColor_Val(CRL_plats_counter, CRL_MaxPlats));
        }

        // Animated lines (64 max)
        if (crl_widget_playstate == 1
        || (crl_widget_playstate == 2 && CRL_lineanims_counter > CRL_MaxAnims))
        {
            char ani[32];

            MN_DrTextA("ANI:", 0, 63, CRL_StatColor_Str(CRL_lineanims_counter, CRL_MaxAnims));
            M_snprintf(ani, 16, "%d/%d", CRL_lineanims_counter, CRL_MaxAnims);
            MN_DrTextA(ani, 32, 63, CRL_StatColor_Val(CRL_lineanims_counter, CRL_MaxAnims));
        }
    }

    // Shift down render counters if no level time/KIS stats are active.
    if (!crl_widget_time)
    {
        yy += 10;
    }
    if (!crl_widget_kis)
    {
        yy += 10;
    }

    // Render counters
    if (crl_widget_render)
    {
        // Sprites (vanilla: 128, doom+: 1024)
        if (crl_widget_render == 1
        || (crl_widget_render == 2 && CRLData.numsprites >= CRL_MaxVisSprites))
        {
            char spr[32];
            
            MN_DrTextA("SPR:", 0, 75 + yy, CRL_StatColor_Str(CRLData.numsprites, CRL_MaxVisSprites));
            M_snprintf(spr, 16, "%d/%d", CRLData.numsprites, CRL_MaxVisSprites);
            MN_DrTextA(spr, 32, 75 + yy, CRL_StatColor_Val(CRLData.numsprites, CRL_MaxVisSprites));
        }

        // Segments (256 max)
        if (crl_widget_render == 1
        || (crl_widget_render == 2 && CRLData.numsegs >= CRL_MaxDrawSegs))
        {
            char seg[32];

            MN_DrTextA("SEG:", 0, 85 + yy, CRL_StatColor_Str(CRLData.numsegs, CRL_MaxDrawSegs));
            M_snprintf(seg, 16, "%d/%d", CRLData.numsegs, CRL_MaxDrawSegs);
            MN_DrTextA(seg, 32, 85 + yy, CRL_StatColor_Val(CRLData.numsegs, CRL_MaxDrawSegs));
        }

        // Solid segments (32 max)
        if (crl_widget_render == 1
        || (crl_widget_render == 2 && CRLData.numsolidsegs >= 32))
        {
            char ssg[32];

            MN_DrTextA("SSG:", 0, 95 + yy, CRL_StatColor_Str(CRLData.numsolidsegs, 32));
            M_snprintf(ssg, 16, "%d/32", CRLData.numsolidsegs);
            MN_DrTextA(ssg, 32, 95 + yy, CRL_StatColor_Val(CRLData.numsolidsegs, 32));
        }

        // Openings
        if (crl_widget_render == 1
        || (crl_widget_render == 2 && CRLData.numopenings >= CRL_MaxOpenings))
        {
            char opn[64];

            MN_DrTextA("OPN:", 0, 105 + yy, CRL_StatColor_Str(CRLData.numopenings, CRL_MaxOpenings));
            M_snprintf(opn, 16, "%d/%d", CRLData.numopenings, CRL_MaxOpenings);
            MN_DrTextA(opn, 32, 105 + yy, CRL_StatColor_Val(CRLData.numopenings, CRL_MaxOpenings));
        }

        // Planes (vanilla: 128, doom+: 1024)
        if (crl_widget_render == 1
        || (crl_widget_render == 2 && TotalVisPlanes >= CRL_MaxVisPlanes))
        {
            char pln[32];

            MN_DrTextA("PLN:", 0, 115 + yy, TotalVisPlanes >= CRL_MaxVisPlanes ? 
                      (gametic & 8 ? cr[CR_GRAY] : cr[CR_LIGHTGRAY]) : cr[CR_GRAY]);
            M_snprintf(pln, 32, "%d/%d (MAX: %d)", TotalVisPlanes, CRL_MaxVisPlanes, CRL_MAX_count);
            MN_DrTextA(pln, 32, 115 + yy, TotalVisPlanes >= CRL_MaxVisPlanes ?
                      (gametic & 8 ? cr[CR_RED] : cr[CR_YELLOW]) : cr[CR_GREEN]);
        }
    }

    if (!crl_widget_kis)
    {
        yy2 += 10;
    }

    // Level timer
    if (crl_widget_time)
    {
        char stra[8];
        char strb[16];
        const int time = leveltime / TICRATE;
        
        M_snprintf(stra, 8, "TIME ");
        MN_DrTextA(stra, 0, 125 + yy2, cr[CR_GRAY]);
        M_snprintf(strb, 16, "%02d:%02d:%02d", time/3600, (time%3600)/60, time%60);
        MN_DrTextA(strb, MN_TextAWidth(stra), 125 + yy2, cr[CR_LIGHTGRAY]);
    }

    // K/I/S stats
    if (crl_widget_kis)
    {
        char str1[8], str2[16];  // kills
        char str3[8], str4[16];  // items
        char str5[8], str6[16];  // secret

        // Kills:
        M_snprintf(str1, 8, "K ");
        MN_DrTextA(str1, 0, 135, cr[CR_GRAY]);
        M_snprintf(str2, 16, "%d/%d ", CRLWidgets.kills, CRLWidgets.totalkills);
        MN_DrTextA(str2, MN_TextAWidth(str1), 135,
                         CRLWidgets.totalkills == 0 ? cr[CR_GREEN] :
                         CRLWidgets.kills == 0 ? cr[CR_RED] :
                         CRLWidgets.kills < CRLWidgets.totalkills ? cr[CR_YELLOW] : cr[CR_GREEN]);

        // Items:
        M_snprintf(str3, 8, "I ");
        MN_DrTextA(str3, MN_TextAWidth(str1) +
                         MN_TextAWidth(str2), 135, cr[CR_GRAY]);
        M_snprintf(str4, 16, "%d/%d ", CRLWidgets.items, CRLWidgets.totalitems);
        MN_DrTextA(str4, MN_TextAWidth(str1) +
                         MN_TextAWidth(str2) +
                         MN_TextAWidth(str3), 135,
                         CRLWidgets.totalitems == 0 ? cr[CR_GREEN] :
                         CRLWidgets.items == 0 ? cr[CR_RED] :
                         CRLWidgets.items < CRLWidgets.totalitems ? cr[CR_YELLOW] : cr[CR_GREEN]);

        // Secrets:
        M_snprintf(str5, 8, "S ");
        MN_DrTextA(str5, MN_TextAWidth(str1) +
                         MN_TextAWidth(str2) +
                         MN_TextAWidth(str3) +
                         MN_TextAWidth(str4), 135, cr[CR_GRAY]);
        M_snprintf(str6, 16, "%d/%d ", CRLWidgets.secrets, CRLWidgets.totalsecrets);
        MN_DrTextA(str6, MN_TextAWidth(str1) +
                         MN_TextAWidth(str2) +
                         MN_TextAWidth(str3) +
                         MN_TextAWidth(str4) +
                         MN_TextAWidth(str5), 135,
                         CRLWidgets.totalsecrets == 0 ? cr[CR_GREEN] :
                         CRLWidgets.secrets == 0 ? cr[CR_RED] :
                         CRLWidgets.secrets < CRLWidgets.totalsecrets ? cr[CR_YELLOW] : cr[CR_GREEN]);
    }
*/
}

// -----------------------------------------------------------------------------
// ID_RightWidgets.
//  [JN] Draw actual frames per second value.
//  Some M_StringWidth adjustments are needed for proper positioning
//  in case of custom font is thinner or thicker.
// -----------------------------------------------------------------------------

void ID_RightWidgets (void)
{
    int yy = 0;

    // [JN] If demo timer is active and running,
    // shift FPS and time widgets one line down.
    if ((demoplayback && (demo_timer == 1 || demo_timer == 3))
    ||  (demorecording && (demo_timer == 2 || demo_timer == 3)))
    {
        yy += 10;
    }

    // [JN] FPS counter
    if (vid_showfps)
    {
        char fps[8];
        char fps_str[4];

        sprintf(fps, "%d", id_fps_value);
        sprintf(fps_str, "FPS");

        MN_DrTextA(fps, ORIGWIDTH + WIDESCREENDELTA - 11 - MN_TextAWidth(fps) 
                                  - MN_TextAWidth(fps_str), 10 + yy, cr[CR_LIGHTGRAY_DARK1]);

        MN_DrTextA(fps_str, ORIGWIDTH + WIDESCREENDELTA - 7 - MN_TextAWidth(fps_str), 10 + yy, cr[CR_LIGHTGRAY_DARK1]);

        yy += 10;
    }

    // [JN] Local time. Time gathered in G_Ticker.
    if (msg_local_time)
    {
        MN_DrTextA(ID_Local_Time, ORIGWIDTH + WIDESCREENDELTA - 7
                              - MN_TextAWidth(ID_Local_Time), 10 + yy, cr[CR_GRAY]);
    }
}

// -----------------------------------------------------------------------------
// ID_HealthColor, ID_DrawTargetsHealth
//  [JN] Indicates and colorizes current target's health.
// -----------------------------------------------------------------------------

static byte *ID_HealthColor (const int val1, const int val2)
{
    return
        val1 <= val2/4 ? cr[CR_RED]    :
        val1 <= val2/2 ? cr[CR_YELLOW] :
                         cr[CR_GREEN]  ;
}

void ID_DrawTargetsHealth (void)
{
    char str[16];
    player_t *player = &players[displayplayer];

    if (player->targetsheathTics <= 0 || !player->targetsheath)
    {
        return;  // No tics or target is dead, nothing to display.
    }

    sprintf(str, "%d/%d", player->targetsheath, player->targetsmaxheath);

    if (widget_health == 1)  // Top
    {
        MN_DrTextACentered(str, 10, ID_HealthColor(player->targetsheath,
                                                   player->targetsmaxheath));
    }
    else
    if (widget_health == 2)  // Top + name
    {
        MN_DrTextACentered(player->targetsname, 10, ID_HealthColor(player->targetsheath,
                                                                   player->targetsmaxheath));
        MN_DrTextACentered(str, 20, ID_HealthColor(player->targetsheath,
                                                   player->targetsmaxheath));
    }
    else
    if (widget_health == 3)  // Bottom
    {
        MN_DrTextACentered(str, 145, ID_HealthColor(player->targetsheath,
                                                    player->targetsmaxheath));
    }
    else
    if (widget_health == 4)  // Bottom + name
    {
        MN_DrTextACentered(player->targetsname, 145, ID_HealthColor(player->targetsheath,
                                                     player->targetsmaxheath));
        MN_DrTextACentered(str, 135, ID_HealthColor(player->targetsheath,
                                                    player->targetsmaxheath));
    }
}

// =============================================================================
//
//                                 Crosshair
//
// =============================================================================

// -----------------------------------------------------------------------------
// [JN] Crosshair graphical patches in Doom GFX format.
// -----------------------------------------------------------------------------

static const byte xhair_cross1[] =
{
    0x07, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 
    0x2A, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x31, 0x00, 0x00, 0x00, 
    0x3E, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x45, 0x00, 0x00, 0x00, 
    0x03, 0x01, 0x5A, 0x5A, 0x5A, 0xFF, 0x03, 0x01, 0x53, 0x53, 0x53, 0xFF, 
    0xFF, 0x00, 0x02, 0x5A, 0x5A, 0x53, 0x53, 0x05, 0x02, 0x53, 0x53, 0x5A, 
    0x5A, 0xFF, 0xFF, 0x03, 0x01, 0x53, 0x53, 0x53, 0xFF, 0x03, 0x01, 0x5A, 
    0x5A, 0x5A, 0xFF
};

static const byte xhair_cross2[] =
{
    0x07, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 
    0x25, 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x2C, 0x00, 0x00, 0x00, 
    0x34, 0x00, 0x00, 0x00, 0x3A, 0x00, 0x00, 0x00, 0x3B, 0x00, 0x00, 0x00, 
    0xFF, 0xFF, 0x03, 0x01, 0x5A, 0x5A, 0x5A, 0xFF, 0x02, 0x03, 0x5A, 0x5A, 
    0x53, 0x5A, 0x5A, 0xFF, 0x03, 0x01, 0x5A, 0x5A, 0x5A, 0xFF, 0xFF, 0xFF
};

static const byte xhair_x[] =
{
    0x07, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 
    0x25, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x3B, 0x00, 0x00, 0x00, 
    0x3C, 0x00, 0x00, 0x00, 0x47, 0x00, 0x00, 0x00, 0x52, 0x00, 0x00, 0x00, 
    0xFF, 0x01, 0x01, 0x5A, 0x5A, 0x5A, 0x05, 0x01, 0x5A, 0x5A, 0x5A, 0xFF, 
    0x02, 0x01, 0x53, 0x53, 0x53, 0x04, 0x01, 0x53, 0x53, 0x53, 0xFF, 0xFF, 
    0x02, 0x01, 0x53, 0x53, 0x53, 0x04, 0x01, 0x53, 0x53, 0x53, 0xFF, 0x01, 
    0x01, 0x5A, 0x5A, 0x5A, 0x05, 0x01, 0x5A, 0x5A, 0x5A, 0xFF, 0xFF
};

static const byte xhair_circle[] =
{
    0x07, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 
    0x25, 0x00, 0x00, 0x00, 0x2D, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 
    0x43, 0x00, 0x00, 0x00, 0x4E, 0x00, 0x00, 0x00, 0x56, 0x00, 0x00, 0x00, 
    0xFF, 0x02, 0x03, 0x5A, 0x5A, 0x53, 0x5A, 0x5A, 0xFF, 0x01, 0x01, 0x5A, 
    0x5A, 0x5A, 0x05, 0x01, 0x5A, 0x5A, 0x5A, 0xFF, 0x01, 0x01, 0x53, 0x53, 
    0x53, 0x05, 0x01, 0x53, 0x53, 0x53, 0xFF, 0x01, 0x01, 0x5A, 0x5A, 0x5A, 
    0x05, 0x01, 0x5A, 0x5A, 0x5A, 0xFF, 0x02, 0x03, 0x5A, 0x5A, 0x53, 0x5A, 
    0x5A, 0xFF, 0xFF
};

static const byte xhair_angle[] =
{
    0x07, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 
    0x25, 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x27, 0x00, 0x00, 0x00, 
    0x2F, 0x00, 0x00, 0x00, 0x35, 0x00, 0x00, 0x00, 0x3B, 0x00, 0x00, 0x00, 
    0xFF, 0xFF, 0xFF, 0x03, 0x03, 0x53, 0x53, 0x5A, 0x60, 0x60, 0xFF, 0x03, 
    0x01, 0x5A, 0x5A, 0x5A, 0xFF, 0x03, 0x01, 0x60, 0x60, 0x60, 0xFF, 0xFF
};

static const byte xhair_triangle[] =
{
    0x07, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 
    0x25, 0x00, 0x00, 0x00, 0x2B, 0x00, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 
    0x3D, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00, 0x00, 0x4A, 0x00, 0x00, 0x00, 
    0xFF, 0x05, 0x01, 0x60, 0x60, 0x60, 0xFF, 0x04, 0x02, 0x5A, 0x5A, 0x5A, 
    0x5A, 0xFF, 0x03, 0x01, 0x53, 0x53, 0x53, 0x05, 0x01, 0x53, 0x53, 0x53, 
    0xFF, 0x04, 0x02, 0x5A, 0x5A, 0x5A, 0x5A, 0xFF, 0x05, 0x01, 0x60, 0x60, 
    0x60, 0xFF, 0xFF
};

static const byte xhair_dot[] =
{
    0x07, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 
    0x25, 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x27, 0x00, 0x00, 0x00, 
    0x2D, 0x00, 0x00, 0x00, 0x2E, 0x00, 0x00, 0x00, 0x2F, 0x00, 0x00, 0x00, 
    0xFF, 0xFF, 0xFF, 0x03, 0x01, 0x53, 0x53, 0x53, 0xFF, 0xFF, 0xFF, 0xFF
};

// -----------------------------------------------------------------------------
// ID_CrosshairShape
//  [JN] Decides which patch should be drawn, depending on "xhair_draw" variable.
// -----------------------------------------------------------------------------

static patch_t *ID_CrosshairShape (void)
{
    return
        xhair_draw == 1 ? (patch_t*) &xhair_cross1   :
        xhair_draw == 2 ? (patch_t*) &xhair_cross2   :
        xhair_draw == 3 ? (patch_t*) &xhair_x        :
        xhair_draw == 4 ? (patch_t*) &xhair_circle   :
        xhair_draw == 5 ? (patch_t*) &xhair_angle    :
        xhair_draw == 6 ? (patch_t*) &xhair_triangle :
                          (patch_t*) &xhair_dot;
}

// -----------------------------------------------------------------------------
// ID_CrosshairColor
//  [JN] Coloring routine, depending on "xhair_color" variable.
// -----------------------------------------------------------------------------

static byte *ID_CrosshairColor (int type)
{
    const player_t *player = &players[displayplayer];

    switch (type)
    {
        case 0:
        {
            // Static/uncolored.
            return
                cr[CR_RED];
            break;
        }
        case 1:
        {
            // Health.
            // Values are same to status bar coloring (ST_WidgetColor).
            return
                player->health >= 67 ? cr[CR_GREEN]  :
                player->health >= 34 ? cr[CR_YELLOW] :
                                       cr[CR_RED]    ;
            break;
        }
        case 2:
        {
            // Target highlight.
            // "linetarget" is gathered via intercept-safe call 
            // of P_AimLineAttack in G_Ticker.
            return
                linetarget ? cr[CR_BLUE2] : cr[CR_RED];
            break;
        }
        case 3:
        {
            // Target highlight+health.
            return
                linetarget           ? cr[CR_BLUE2]  :
                player->health >= 67 ? cr[CR_GREEN]  :
                player->health >= 34 ? cr[CR_YELLOW] :
                                       cr[CR_RED]    ;
            break;
        }
    }

    return NULL;
}

// -----------------------------------------------------------------------------
// ID_DrawCrosshair
//  [JN] Drawing routine, called via D_Display.
// -----------------------------------------------------------------------------

void ID_DrawCrosshair (void)
{
    const int xx = (ORIGWIDTH  / 2) - 3;
    const int yy = (ORIGHEIGHT / 2) - 3 - (dp_screen_size <= 10 ? 16 : 0);

    dp_translation = ID_CrosshairColor(xhair_color);
    V_DrawPatch(xx, yy, ID_CrosshairShape());
    dp_translation = NULL;
}

// =============================================================================
//
//                             Demo enhancements
//
// =============================================================================

// -----------------------------------------------------------------------------
// ID_DemoTimer
//  [crispy] Demo Timer widget
// -----------------------------------------------------------------------------

void ID_DemoTimer (const int time)
{
    const int hours = time / (3600 * TICRATE);
    const int mins = time / (60 * TICRATE) % 60;
    const float secs = (float)(time % (60 * TICRATE)) / TICRATE;
    char n[16];
    int x = 237;

    if (hours)
    {
        M_snprintf(n, sizeof(n), "%02i:%02i:%05.02f", hours, mins, secs);
    }
    else
    {
        M_snprintf(n, sizeof(n), "%02i:%05.02f", mins, secs);
        x += 20;
    }

    MN_DrTextA(n, x + WIDESCREENDELTA, 10, cr[CR_LIGHTGRAY]);
}

// -----------------------------------------------------------------------------
// ID_DemoBar
//  [crispy] print a bar indicating demo progress at the bottom of the screen
// -----------------------------------------------------------------------------

void ID_DemoBar (void)
{
    static boolean colors_set = false;
    static int black = 0;
    static int white = 0;
    const int i = SCREENWIDTH * defdemotics / deftotaldemotics;

    // [JN] Don't rely on palette indexes,
    // try to find nearest colors instead.
    if (!colors_set)
    {
#ifndef CRISPY_TRUECOLOR
        black = I_GetPaletteIndex(0, 0, 0);
        white = I_GetPaletteIndex(255, 255, 255);
#else
        black = I_MapRGB(0, 0, 0);
        white = I_MapRGB(255, 255, 255);
#endif
        colors_set = true;
    }

#ifndef CRISPY_TRUECOLOR
    V_DrawHorizLine(0, SCREENHEIGHT - 2, i, black); // [crispy] black
    V_DrawHorizLine(0, SCREENHEIGHT - 1, i, white); // [crispy] white
#else
    V_DrawHorizLine(0, SCREENHEIGHT - 2, i, black); // [crispy] black
    V_DrawHorizLine(0, SCREENHEIGHT - 1, i, white); // [crispy] white
#endif
}