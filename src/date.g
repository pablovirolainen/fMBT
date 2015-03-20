{
/*
 * fMBT, free Model Based Testing tool
 * Copyright (c) 2011, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>

typedef struct _node {
  GDateTime* date;
  bool rel;
  long i;
  int year,month,day,hour,min,sec;
  GTimeZone *zone;
  _node():date(NULL),rel(false),i(0),
          year(0),month(0),day(0),hour(0),min(0),sec(0),zone(NULL)
  { }
    
  _node(int _y,int _m,int _d,int _h,int _mi, int s):date(NULL),rel(true),i(0),
      year(_y),month(_m),day(_d),hour(_h),min(_mi),sec(s)
      { }
} node;

int date_node_size = sizeof (node);

#define D_ParseNode_User node

}

time: spec { $$=$0; } | items { 
            $$.zone=$0.zone;
            if ($0.date) { 
                $$.date=$0.date;
            } else {
                //printf("items hour %i ,min %i,sec %i\n",$0.hour,$0.min,$0.sec);
                GDateTime* tmp;
                if ($$.zone) {
                    tmp=g_date_time_new_now($$.zone);
                } else {
                    tmp=g_date_time_new_now_local();
                }
                //printf(g_date_time_format($$.date,"%a %e.%m %Y %R\n"));
                $$.date= g_date_time_add_full(tmp,
                                              $0.year,$0.month,$0.day,$0.hour,
                                              $0.min,$0.sec);
                g_date_time_unref(tmp);
            }
            if (!$$.date) {
                // ERROR!
            }
            $$.date=g_date_time_to_local($$.date);
            printf(g_date_time_format($$.date,"%a %e.%m %Y %R\n"));
        };

timezone: 'TZ' '=' "[a-zA-Z0-9]*" {
            char* tmp_str=strndup($n2.start_loc.s,$n2.end-$n2.start_loc.s);
            printf("Timezone %s\n",tmp_str);
            $$.zone=g_time_zone_new(tmp_str);
            printf("Zone at %p\n",$$.zone);
            free(tmp_str);
          };

// The easy case first :)
spec: '@' int {
            GTimeVal t;
            t.tv_sec=$1.i;
            t.tv_usec=0;
            $$.date = g_date_time_new_from_timeval_utc(&t);
        } |
       '@' int '.' int {
            GTimeVal t;
            t.tv_sec=$1.i;
            t.tv_usec=$3.i;
            $$.date = g_date_time_new_from_timeval_utc(&t);
        };

items: { $$ = _node(); $$.rel=true; } | timezone { $$.zone=$0.zone; $$.rel=true; } | items item {
            if ($1.zone) {
                $$.zone=$1.zone;
            } else {
                $$.zone=$0.zone;
            }
            if (!$1.rel && $0.date) {
                // error..
            }
            if ($1.rel) {
                if ($0.rel) {
                    $$ = _node($0.year+$1.year,
                               $0.month+$1.month,
                               $0.day+$1.day,
                               $0.hour+$1.hour,
                               $0.min+$1.min,
                               $0.sec+$1.sec);
                } else {
                    $$.date = g_date_time_add_full($0.date,
                                                   $1.year,$1.month,$1.day,$1.hour,
                                                   $1.min,$1.sec);
                    g_date_time_unref($0.date);
                }
            } else {
              $$=$1;
            }
        } | items rel_daymonth {
            $$.zone=$0.zone;
            GDateTime* tmp;
            if ($0.rel) {
                if ($$.zone) {
                    $$.date=g_date_time_new_now($$.zone);
                    if (!$$.date) {
                        // ERROR!
                        abort();
                    }
                } else {
                    $$.date=g_date_time_new_now_local();
                    if (!$$.date) {
                        // ERROR!
                        abort();
                    }
                }
            } else {
                $$.date=$0.date;
            }

            if (!$$.date) {
                // ERROR!
                abort();
            }

            if ($1.year) {
                tmp=$$.date;
                $$.date=g_date_time_add_years($$.date,$1.year);
                g_date_time_unref(tmp);
            }
            if ($1.month) {
                tmp=$$.date;
                int wmonth=$1.month - g_date_time_get_month($$.date);
                if (wmonth<=0) {
                    wmonth+=12;
                }
                $$.date=g_date_time_add_months($$.date,wmonth);
                g_date_time_unref(tmp);
            }

            if ($1.day) {
                tmp=$$.date;
                int wday=$1.day - g_date_time_get_day_of_week($$.date);
                if (wday<=0) {
                    wday+=7;
                }
                $$.date=g_date_time_add_days($$.date,wday);
                g_date_time_unref(tmp);
            }
            $$.rel=false;

        } | items set_clock {
            $$.zone=$0.zone;
            GDateTime* tmp;
            if ($0.rel) {
                if ($$.zone) {
                    $$.date=g_date_time_new_now($$.zone);
                } else {
                    $$.date=g_date_time_new_now_local();
                }
                //printf(g_date_time_format($$.date,"%a %e.%m %Y %R\n"));
            } else {
                $$.date=$0.date;
            }

            tmp=$$.date;

            if (!tmp) {
                // ERROR!
            }

            printf("%p\n",$$.zone);
            if ($$.zone) {
                $$.date=g_date_time_new($$.zone,g_date_time_get_year($$.date),g_date_time_get_month($$.date),g_date_time_get_day_of_month($$.date),$1.hour,$1.min,$1.sec);
            } else {
                $$.date=g_date_time_new_local(g_date_time_get_year($$.date),g_date_time_get_month($$.date),g_date_time_get_day_of_month($$.date),$1.hour,$1.min,$1.sec);
            }
            g_date_time_unref(tmp);

            if (!$$.date) {
                // ERROR!
            }

            $$.rel=false;
        } ;

set_clock: iso_8601_time {
            $$=$0;
        } |
        iso_8601_time 'am' {
            $$=$0;
            //printf("AM hour %i ,min %i,sec %i\n",$$.hour,$$.min,$$.sec); 
        } |
        iso_8601_time 'pm' {
            $$=$0;
            if ($$.hour<12) {
                $$.hour+=12;
            } 
            //printf("PM hour %i ,min %i,sec %i\n",$$.hour,$$.min,$$.sec); 
      } ;

item: iso_8601_datetime { $$ = $0; } |
      rel               { $$ = $0; } ;

iso_8601_datetime:
    iso_8601_date 'T' iso_8601_time { $$.date = g_date_time_new_local(
                $0.year,$0.month,$0.day,$2.hour,$2.min,$2.sec); 
            if (!$$.date) {
                // ERROR!
            } } |
    timezone iso_8601_date 'T' iso_8601_time { $$.date = g_date_time_new($0.zone,
                $1.year,$1.month,$1.day,$3.hour,$3.min,$3.sec); 
            if (!$$.date) {
                // ERROR!
            } } ;

iso_8601_date:
    uint sint sint { $$ = _node($0.i,$1.i,$2.i,0,0,0); };

iso_8601_time:
       int                  { $$=_node(0,0,0,$0.i,0,0); }
     | int ':' int          { $$=_node(0,0,0,$0.i,$2.i,0); }
     | int ':' int ':' int  { $$=_node(0,0,0,$0.i,$2.i,$4.i); } ;

// We don't need X Y ago ?

rel: simple_rel { $$ = $0; };
//     rel_day    { $$ = $0; } ;

rel_day: 'tomorrow'           { $$ = _node(0,0, 1,0,0,0); } |
        'yesterday'           { $$ = _node(0,0,-1,0,0,0); } |
        ('today' | 'now')     { $$ = _node(0,0, 0,0,0,0); } ;

prefix: 'last' { $$.i=-1; } | 'next' { $$.i=1; }; // What would  'first' and 'now' mean?

daymonth: 'monday'    { $$ = _node(0,0,1,0,0,0); } |
          'tuesday'   { $$ = _node(0,0,2,0,0,0); } |
          'wednesday' { $$ = _node(0,0,3,0,0,0); } |
          'thursday'  { $$ = _node(0,0,4,0,0,0); } |
          'friday'    { $$ = _node(0,0,5,0,0,0); } |
          'saturday'  { $$ = _node(0,0,6,0,0,0); } |
          'sunday'    { $$ = _node(0,0,7,0,0,0); } |
          'january'   { $$ = _node(0,1,0,0,0,0); } |
          'february'  { $$ = _node(0,2,0,0,0,0); } |
          'march'     { $$ = _node(0,3,0,0,0,0); } |
          'april'     { $$ = _node(0,4,0,0,0,0); } |
          'may'       { $$ = _node(0,5,0,0,0,0); } |
          'june'      { $$ = _node(0,6,0,0,0,0); } |
          'july'      { $$ = _node(0,7,0,0,0,0); } |
          'august'    { $$ = _node(0,8,0,0,0,0); } |
          'september' { $$ = _node(0,9,0,0,0,0); } |
          'october'   { $$ = _node(0,10,0,0,0,0); } |
          'november'  { $$ = _node(0,11,0,0,0,0); } |
          'december'  { $$ = _node(0,12,0,0,0,0); } ;

rel_daymonth: prefix daymonth {
            $$=$1;
            $$.year *= $0.i;
            $$.month*= $0.i;
            $$.day  *= $0.i;
            $$.hour *= $0.i;
            $$.min  *= $0.i;
            $$.sec  *= $0.i;
        };

rel_spec: 'year' { $$ = _node(1,0,0,0,0,0); } |
         'month' { $$ = _node(0,1,0,0,0,0); } |
         'week'  { $$ = _node(0,0,7,0,0,0); } |
         'day'   { $$ = _node(0,0,1,0,0,0); } |
         'hour'  { $$ = _node(0,0,0,1,0,0); } |
         'min'   { $$ = _node(0,0,0,0,1,0); } |
         'sec'   { $$ = _node(0,0,0,0,0,1); } ;
//         's'     { $$ = _node(0,0,0,0,0,1); } ;

int_or_prefix: int { $$.i=$0.i; } | prefix { $$.i=$0.i; };

simple_rel: int_or_prefix rel_spec { $$=$1;
            $$.year *= $0.i;
            $$.month*= $0.i;
            $$.day  *= $0.i;
            $$.hour *= $0.i;
            $$.min  *= $0.i;
            $$.sec  *= $0.i;
        };

int: ustr { $$.i = atoll($n0.start_loc.s); } |
     sstr { $$.i = atoll($n0.start_loc.s); } |
     pstr { $$.i = atoll($n0.start_loc.s); } ;

ustr: "[0-9]+";
sstr: "\\-[0-9]+";
pstr: "\\+[0-9]+";

uint: ustr { $$.i = atoll($n0.start_loc.s); };
sint: sstr { $$.i = atoll($n0.start_loc.s); };
