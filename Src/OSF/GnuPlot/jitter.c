/*
 * $Id: jitter.c,v 1.4 2015/12/19 21:45:35 sfeam Exp $
 */
/*
 * This file contains routines used to support the "set jitter" option.
 * This plot mode was inspired by the appearance of "beeswarm" plots produced
 * by R, but I do not know to what extent the algorithms used are the same.
 */
/*[
 * Copyright Ethan A Merritt 2015
 *
 * Gnuplot license:
 *
 * Permission to use, copy, and distribute this software and its
 * documentation for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 *
 * Permission to modify the software is granted, but not the right to
 * distribute the complete modified source code.  Modifications are to
 * be distributed as patches to the released version.  Permission to
 * distribute binaries produced by compiling modified sources is granted,
 * provided you
 *   1. distribute the corresponding source modifications from the
 *    released version in the form of a patch file along with the binaries,
 *   2. add special version identification to distinguish your version
 *    in addition to the base release version number,
 *   3. provide your name and address as the primary contact for the
 *    support of your modified version, and
 *   4. retain our contact information in regard to use of the base
 *    software.
 * Permission to distribute the released version of the source code along
 * with corresponding source modifications in the form of a patch file is
 * granted with same provisions 2 through 4 for binary distributions.
 *
 * This software is provided "as is" without express or implied warranty
 * to the extent permitted by applicable law.
 *
 * Alternative license:
 *
 * As an alternative to distributing code in this file under the gnuplot license,
 * you may instead comply with the terms below. In this case, redistribution and
 * use in source and binary forms, with or without modification, are permitted
 * provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.  Redistributions in binary
 * form must reproduce the above copyright notice, this list of conditions and
 * the following disclaimer in the documentation and/or other materials provided
 * with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
   ]*/
#include <gnuplot.h>
#pragma hdrstop

t_jitter jitter;

static int compare_xypoints(const void * arg1, const void * arg2);

static int compare_xypoints(const void * arg1, const void * arg2)
{
	GpCoordinate const * p1 = (GpCoordinate const *)arg1;
	GpCoordinate const * p2 = (GpCoordinate const *)arg2;
	/* Primary sort is on x */
	/* FIXME: I'd like to treat x coords within jitter.x as equal, */
	/*        but the GpCoordinate system mismatch makes this hard.  */
	if(p1->x > p2->x)
		return (1);
	else if(p1->x < p2->x)
		return (-1);
	else if(p1->y > p2->y)
		return (1);
	else if(p1->y < p2->y)
		return (-1);
	else
		return (0);
}
/*
 * "set jitter overlap <ydelta> spread <factor>"
 * displaces overlapping points in a point plot.
 * The jittering algorithm is inspired by the beeswarm plot variant in R.
 */
//static double jdist(GpCoordinate * pi, GpCoordinate * pj)
double GpGadgets::JDist(const GpCoordinate * pi, const GpCoordinate * pj)
{
	int delx = MapX(pi->x) - MapX(pj->x);
	int dely = MapY(pi->y) - MapY(pj->y);
	return sqrt((double)(delx*delx + dely*dely));
}

void jitter_points(CurvePoints * plot)
{
	int i, j;
	/* The "x" and "xscale" stored in jitter are really along y */
	double xjit, ygap;
	GpPosition yoverlap;
	yoverlap.x = 0;
	yoverlap.y = jitter.overlap.x;
	yoverlap.scaley = jitter.overlap.scalex;
	GpGg.MapPositionR(yoverlap, &xjit, &ygap, "jitter");
	/* Clear xhigh and yhigh, where we will later store the jitter offsets. */
	/* Store variable color temporarily in ylow so it is not lost by sorting. */
	for(i = 0; i < plot->p_count; i++) {
		if(plot->varcolor)
			plot->points[i].ylow = plot->varcolor[i];
		plot->points[i].xhigh = 0.0;
		plot->points[i].yhigh = 0.0;
	}
	qsort(plot->points, plot->p_count, sizeof(GpCoordinate), compare_xypoints);
	/* For each point, check whether subsequent points would overlap it. */
	/* If so, displace them in a fixed pattern */
	i = 0;
	while(i < plot->p_count - 1) {
		for(j = 1; i+j < plot->p_count; j++) {
			if(GpGg.JDist(&plot->points[i], &plot->points[i+j]) >= ygap)
				break;
			/* Displace point purely on x */
			xjit  = (j+1)/2 * jitter.spread * plot->lp_properties.p_size;
			if(jitter.limit > 0)
				while(xjit > jitter.limit)
					xjit -= jitter.limit;
			if((j & 01) != 0)
				xjit = -xjit;
			plot->points[i+j].xhigh = xjit;
			if(jitter.style == JITTER_SQUARE)
				plot->points[i+j].yhigh = plot->points[i].y - plot->points[i+j].y;
		}
		i += j;
	}
	/* Copy variable colors back to where the plotting code expects to find them */
	if(plot->varcolor)
		for(i = 0; i < plot->p_count; i++) {
			plot->varcolor[i] = plot->points[i].ylow;
			plot->points[i].ylow = plot->points[i].y;
		}
}
//
// process 'set jitter' command 
//
//void set_jitter()
void GpGadgets::SetJitter(GpCommand & rC)
{
	rC.CToken++;
	// Default overlap criterion 1 character (usually on y) 
	jitter.overlap.scalex = character;
	jitter.overlap.x = 1;
	jitter.spread = 1.0;
	jitter.limit = 0.0;
	jitter.style = JITTER_DEFAULT;
	while(!rC.EndOfCommand()) {
		if(rC.AlmostEq("over$lap")) {
			rC.CToken++;
			GetPositionDefault(rC, &jitter.overlap, character, 2);
		}
		else if(rC.Eq("spread")) {
			rC.CToken++;
			jitter.spread = rC.RealExpression();
			if(jitter.spread <= 0)
				jitter.spread = 1.0;
		}
		else if(rC.Eq("swarm")) {
			rC.CToken++;
			jitter.style = JITTER_SWARM;
		}
		else if(rC.Eq("square")) {
			rC.CToken++;
			jitter.style = JITTER_SQUARE;
		}
		else if(rC.Eq("wrap")) {
			rC.CToken++;
			jitter.limit = rC.RealExpression();
		}
		else
			IntError(rC, rC.CToken, "unrecognized keyword");
	}
}
//
// process 'show jitter' command 
//
void show_jitter()
{
	if(jitter.spread <= 0) {
		fprintf(stderr, "\tno jitter\n");
	}
	else {
		fprintf(stderr, "\toverlap criterion  %g %s coords\n", jitter.overlap.x, coord_msg[jitter.overlap.scalex]);
		fprintf(stderr, "\tspread multiplier on x: %g\n", jitter.spread);
		if(jitter.limit > 0)
			fprintf(stderr, "\twrap at %g character widths\n", jitter.limit);
		fprintf(stderr, "\tstyle: %s\n", jitter.style == JITTER_SQUARE ? "square" : "swarm");
	}
}

/* process 'unset jitter' command */
void unset_jitter()
{
	jitter.spread = 0;
}

/* called by the save command */
void save_jitter(FILE * fp)
{
	if(jitter.spread <= 0)
		fprintf(fp, "unset jitter\n");
	else {
		fprintf(fp, "set jitter overlap %s%g",
		    jitter.overlap.scalex == character ? "" : coord_msg[jitter.overlap.scalex], jitter.overlap.x);
		fprintf(fp, "  spread %g  wrap %g", jitter.spread, jitter.limit);
		fprintf(fp, jitter.style == JITTER_SQUARE ? " square\n" : "\n");
	}
}
