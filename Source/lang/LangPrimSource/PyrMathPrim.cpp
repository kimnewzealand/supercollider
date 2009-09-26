/*
	SuperCollider real time audio synthesis system
    Copyright (c) 2002 James McCartney. All rights reserved.
	http://www.audiosynth.com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "PyrKernel.h"
#include "PyrPrimitive.h"
#include "PyrMathPrim.h"
#include "MiscInlineMath.h"
#include "SC_InlineUnaryOp.h"
#include "SC_InlineBinaryOp.h"
#include "PyrSignal.h"
#include "PyrParseNode.h"
#include "PyrMessage.h"
#include "clz.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "SC_Endian.h"

const int INT_MAX_BY_PyrSlot = INT_MAX / sizeof(PyrSlot);

inline bool IsSignal(PyrSlot* slot) { return ((slot)->utag == tagObj && (slot)->uo->classptr == class_signal); }
inline bool NotSignal(PyrSlot* slot) { return ((slot)->utag != tagObj || (slot)->uo->classptr != class_signal); }

int prAddNum(VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a, *b;
	PyrSymbol *msg;

	a = g->sp - 1;
	b = g->sp;

	switch (a->utag) {
		case tagInt :
			switch (b->utag) {
				case tagInt :
					a->ui = a->ui + b->ui;
					break;
				case tagChar : case tagPtr :
				case tagNil : case tagFalse : case tagTrue :
					goto send_normal_2;
				case tagSym :
					a->us = b->us; a->utag = tagSym;
					break;
				case tagObj :
					if (isKindOf(b->uo, class_signal)) {
						a->uo = signal_add_xf(g, b->uo, a->ui); a->utag = tagObj;
					} else {
						goto send_normal_2;
					}
					break;
				default :
					a->uf = a->ui + b->uf;
					break;
			}
			break;
		case tagChar : case tagPtr :
		case tagNil : case tagFalse : case tagTrue :
			goto send_normal_2;
		case tagSym :
			// leave self in 'a'
			break;
		case tagObj :
			if (isKindOf(a->uo, class_signal)) {
				switch (b->utag) {
					case tagInt :
						a->uo = signal_add_xf(g, a->uo, b->ui); //a->utag = tagObj;
						break;
					case tagChar : case tagPtr :
					case tagNil : case tagFalse : case tagTrue :
						goto send_normal_2;
					case tagSym :
						a->us = b->us; a->utag = tagSym;
						break;
					case tagObj :
						if (isKindOf(b->uo, class_signal)) {
							a->uo = signal_add_xx(g, a->uo, b->uo); //a->utag = tagObj;
						} else goto send_normal_2;
						break;
					default : // double
						a->uo = signal_add_xf(g, a->uo, b->uf); //a->utag = tagObj;
						break;
				}
			} else {
				goto send_normal_2;
			}
			break;
		default : // double
			switch (b->utag) {
				case tagInt :
					a->uf = a->uf + b->ui;
					break;
				case tagChar : case tagPtr :
				case tagNil : case tagFalse : case tagTrue :
					goto send_normal_2;
				case tagSym :
					a->us = b->us; a->utag = tagSym;
					break;
				case tagObj :
					if (isKindOf(b->uo, class_signal)) {
						a->uo = signal_add_xf(g, b->uo, a->uf);  a->utag = tagObj; break;
					} else goto send_normal_2;
					break;
				default : // double
					a->uf = a->uf + b->uf; break;
					break;
			}
			break;
	}
	g->sp-- ; // drop
	g->numpop = 0;
#if TAILCALLOPTIMIZE
	g->tailCall = 0;
#endif
	return errNone;

	send_normal_2:
	if (numArgsPushed != -1)  // special case flag meaning it is a primitive
		return errFailed;	// arguments remain on the stack

	msg = gSpecialBinarySelectors[g->primitiveIndex];
	sendMessage(g, msg, 2);
	return errNone;
}

int prMulNum(VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a, *b;
	PyrSymbol *msg;

	a = g->sp - 1;
	b = g->sp;

	switch (a->utag) {
		case tagInt :
			switch (b->utag) {
				case tagInt :
					a->ui = a->ui * b->ui; //a->utag = tagInt;
					break;
				case tagChar : case tagPtr :
				case tagNil : case tagFalse : case tagTrue :
					goto send_normal_2;
				case tagSym :
					a->us = b->us; a->utag = tagSym;
					break;
				case tagObj :
					if (isKindOf(b->uo, class_signal)) {
						a->uo = signal_mul_xf(g, b->uo, a->ui); a->utag = tagObj;
					} else {
						goto send_normal_2;
					}
					break;
				default :
					a->uf = a->ui * b->uf;
					break;
			}
			break;
		case tagChar : case tagPtr :
		case tagNil : case tagFalse : case tagTrue :
			goto send_normal_2;
		case tagSym :
			//a->us = a->us; a->utag = tagSym;
			break;
		case tagObj :
			if (isKindOf(a->uo, class_signal)) {
				switch (b->utag) {
					case tagInt :
						a->uo = signal_mul_xf(g, a->uo, b->ui); //a->utag = tagObj;
						break;
					case tagChar : case tagPtr :
					case tagNil : case tagFalse : case tagTrue :
						goto send_normal_2;
					case tagSym :
						a->us = b->us; a->utag = tagSym;
						break;
					case tagObj :
						if (isKindOf(b->uo, class_signal)) {
							a->uo = signal_mul_xx(g, a->uo, b->uo); //a->utag = tagObj;
						} else goto send_normal_2;
						break;
					default : // double
						a->uo = signal_mul_xf(g, a->uo, b->uf); //a->utag = tagObj;
						break;
				}
			} else {
				goto send_normal_2;
			}
			break;
		default : // double
			switch (b->utag) {
				case tagInt :
					a->uf = a->uf * b->ui;
					break;
				case tagChar : case tagPtr :
				case tagNil : case tagFalse : case tagTrue :
					goto send_normal_2;
				case tagSym :
					a->us = b->us; a->utag = tagSym;
					break;
				case tagObj :
					if (isKindOf(b->uo, class_signal)) {
						a->uo = signal_mul_xf(g, b->uo, a->uf);  a->utag = tagObj; break;
					} else goto send_normal_2;
					break;
				default : // double
					a->uf = a->uf * b->uf; break;
					break;
			}
			break;
	}
	g->sp-- ; // drop
	g->numpop = 0;
#if TAILCALLOPTIMIZE
	g->tailCall = 0;
#endif
	return errNone;

	send_normal_2:
	if (numArgsPushed != -1)  // special case flag meaning it is a primitive
		return errFailed;	// arguments remain on the stack

	msg = gSpecialBinarySelectors[g->primitiveIndex];
	sendMessage(g, msg, 2);
	return errNone;
}

int prSubNum(VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a, *b;
	PyrSymbol *msg;

	a = g->sp - 1;
	b = g->sp;

	switch (a->utag) {
		case tagInt :
			switch (b->utag) {
				case tagInt :
					a->ui = a->ui - b->ui; //a->utag = tagInt;
					break;
				case tagChar : case tagPtr :
				case tagNil : case tagFalse : case tagTrue :
					goto send_normal_2;
				case tagSym :
					a->us = b->us; a->utag = tagSym;
					break;
				case tagObj :
					if (isKindOf(b->uo, class_signal)) {
						a->uo = signal_sub_fx(g, a->ui, b->uo); a->utag = tagObj;
					} else {
						goto send_normal_2;
					}
					break;
				default :
					a->uf = a->ui - b->uf;
					break;
			}
			break;
		case tagChar : case tagPtr :
		case tagNil : case tagFalse : case tagTrue :
			goto send_normal_2;
		case tagSym :
			//a->us = a->us; a->utag = tagSym;
			break;
		case tagObj :
			if (isKindOf(a->uo, class_signal)) {
				switch (b->utag) {
					case tagInt :
						a->uo = signal_sub_xf(g, a->uo, b->ui); //a->utag = tagObj;
						break;
					case tagChar : case tagPtr :
					case tagNil : case tagFalse : case tagTrue :
						goto send_normal_2;
					case tagSym :
						a->us = b->us; a->utag = tagSym;
						break;
					case tagObj :
						if (isKindOf(b->uo, class_signal)) {
							a->uo = signal_sub_xx(g, a->uo, b->uo); //a->utag = tagObj;
						} else goto send_normal_2;
						break;
					default : // double
						a->uo = signal_sub_xf(g, a->uo, b->uf); //a->utag = tagObj;
						break;
				}
			} else {
				goto send_normal_2;
			}
			break;
		default : // double
			switch (b->utag) {
				case tagInt :
					a->uf = a->uf - b->ui;
					break;
				case tagChar : case tagPtr :
				case tagNil : case tagFalse : case tagTrue :
					goto send_normal_2;
				case tagSym :
					a->us = b->us; a->utag = tagSym;
					break;
				case tagObj :
					if (isKindOf(b->uo, class_signal)) {
						a->uo = signal_sub_fx(g, a->uf, b->uo);  a->utag = tagObj; break;
					} else goto send_normal_2;
					break;
				default : // double
					a->uf = a->uf - b->uf; break;
					break;
			}
			break;
	}
	g->sp-- ; // drop
	g->numpop = 0;
#if TAILCALLOPTIMIZE
	g->tailCall = 0;
#endif
	return errNone;

	send_normal_2:
	if (numArgsPushed != -1)  // special case flag meaning it is a primitive
		return errFailed;	// arguments remain on the stack

	msg = gSpecialBinarySelectors[g->primitiveIndex];
	sendMessage(g, msg, 2);
	return errNone;
}

int prAddInt(VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a, *b;
	PyrSymbol *msg;

	a = g->sp - 1;
	b = g->sp;

	switch (b->utag) {
		case tagInt :
			a->ui = a->ui + b->ui; //a->utag = tagInt;
			break;
		case tagChar : case tagPtr :
		case tagNil : case tagFalse : case tagTrue :
			goto send_normal_2;
		case tagSym :
			a->us = b->us; a->utag = tagSym;
			break;
		case tagObj :
			if (isKindOf(b->uo, class_signal)) {
				a->uo = signal_add_xf(g, b->uo, a->ui); a->utag = tagObj;
			} else {
				goto send_normal_2;
			}
			break;
		default :
			a->uf = a->ui + b->uf;
			break;
	}
	g->sp-- ; // drop
	g->numpop = 0;
#if TAILCALLOPTIMIZE
	g->tailCall = 0;
#endif
	return errNone;

	send_normal_2:
	if (numArgsPushed != -1)  // special case flag meaning it is a primitive
		return errFailed;	// arguments remain on the stack

	msg = gSpecialBinarySelectors[g->primitiveIndex];
	sendMessage(g, msg, 2);
	return errNone;
}



int prAddFloat(VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a, *b;
	PyrSymbol *msg;

	a = g->sp - 1;
	b = g->sp;

	switch (b->utag) {
		case tagInt :
			a->uf = a->uf + b->ui; break;
			break;
		case tagChar : case tagPtr :
		case tagNil : case tagFalse : case tagTrue :
			goto send_normal_2;
		case tagSym :
			a->us = b->us; a->utag = tagSym;
			break;
		case tagObj :
			if (isKindOf(b->uo, class_signal)) {
				a->uo = signal_add_xf(g, b->uo, a->uf); a->utag = tagObj;
			} else {
				goto send_normal_2;
			}
			break;
		default :
			a->uf = a->uf + b->uf;
			break;
	}
	g->sp-- ; // drop
	g->numpop = 0;
#if TAILCALLOPTIMIZE
	g->tailCall = 0;
#endif
	return errNone;

	send_normal_2:
	if (numArgsPushed != -1)  // special case flag meaning it is a primitive
		return errFailed;	// arguments remain on the stack

	msg = gSpecialBinarySelectors[g->primitiveIndex];
	sendMessage(g, msg, 2);
	return errNone;
}

int prSubInt(VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a, *b;
	PyrSymbol *msg;

	a = g->sp - 1;
	b = g->sp;

	switch (b->utag) {
		case tagInt :
			a->ui = a->ui - b->ui; //a->utag = tagInt;
			break;
		case tagChar : case tagPtr :
		case tagNil : case tagFalse : case tagTrue :
			goto send_normal_2;
		case tagSym :
			a->us = b->us; a->utag = tagSym;
			break;
		case tagObj :
			if (isKindOf(b->uo, class_signal)) {
				a->uo = signal_sub_xf(g, b->uo, a->ui); a->utag = tagObj;
			} else {
				goto send_normal_2;
			}
			break;
		default :
			a->uf = a->ui - b->uf;
			break;
	}
	g->sp-- ; // drop
	g->numpop = 0;
#if TAILCALLOPTIMIZE
	g->tailCall = 0;
#endif
	return errNone;

	send_normal_2:
	if (numArgsPushed != -1)  // special case flag meaning it is a primitive
		return errFailed;	// arguments remain on the stack

	msg = gSpecialBinarySelectors[g->primitiveIndex];
	sendMessage(g, msg, 2);
	return errNone;
}



int prSubFloat(VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a, *b;
	PyrSymbol *msg;

	a = g->sp - 1;
	b = g->sp;

	switch (b->utag) {
		case tagInt :
			a->uf = a->uf - b->ui; break;
			break;
		case tagChar : case tagPtr :
		case tagNil : case tagFalse : case tagTrue :
			goto send_normal_2;
		case tagSym :
			a->us = b->us; a->utag = tagSym;
			break;
		case tagObj :
			if (isKindOf(b->uo, class_signal)) {
				a->uo = signal_sub_xf(g, b->uo, a->uf); a->utag = tagObj; break;
			} else {
				goto send_normal_2;
			}
			break;
		default :
			a->uf = a->uf - b->uf;
			break;
	}
	g->sp-- ; // drop
	g->numpop = 0;
#if TAILCALLOPTIMIZE
	g->tailCall = 0;
#endif
	return errNone;

	send_normal_2:
	if (numArgsPushed != -1)  // special case flag meaning it is a primitive
		return errFailed;	// arguments remain on the stack

	msg = gSpecialBinarySelectors[g->primitiveIndex];
	sendMessage(g, msg, 2);
	return errNone;
}

int prMulInt(VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a, *b;
	PyrSymbol *msg;

	a = g->sp - 1;
	b = g->sp;

	switch (b->utag) {
		case tagInt :
			a->ui = a->ui * b->ui; //a->utag = tagInt; break;
			break;
		case tagChar : case tagPtr :
		case tagNil : case tagFalse : case tagTrue :
			goto send_normal_2;
		case tagSym :
			a->us = b->us; a->utag = tagSym;
			break;
		case tagObj :
			if (isKindOf(b->uo, class_signal)) {
				a->uo = signal_mul_xf(g, b->uo, a->ui); a->utag = tagObj; break;
			} else {
				goto send_normal_2;
			}
			break;
		default :
			a->uf = a->ui * b->uf;
			break;
	}
	g->sp-- ; // drop
	g->numpop = 0;
#if TAILCALLOPTIMIZE
	g->tailCall = 0;
#endif
	return errNone;

	send_normal_2:
	if (numArgsPushed != -1)  // special case flag meaning it is a primitive
		return errFailed;	// arguments remain on the stack

	msg = gSpecialBinarySelectors[g->primitiveIndex];
	sendMessage(g, msg, 2);
	return errNone;
}



int prMulFloat(VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a, *b;
	PyrSymbol *msg;

	a = g->sp - 1;
	b = g->sp;

	switch (b->utag) {
		case tagInt :
			a->uf = a->uf * b->ui; break;
			break;
		case tagChar : case tagPtr :
		case tagNil : case tagFalse : case tagTrue :
			goto send_normal_2;
		case tagSym :
			a->us = b->us; a->utag = tagSym;
			break;
		case tagObj :
			if (isKindOf(b->uo, class_signal)) {
				a->uo = signal_mul_xf(g, b->uo, a->uf); a->utag = tagObj; break;
			} else {
				goto send_normal_2;
			}
			break;
		default :
			a->uf = a->uf * b->uf;
			break;
	}
	g->sp-- ; // drop
	g->numpop = 0;
#if TAILCALLOPTIMIZE
	g->tailCall = 0;
#endif
	return errNone;

	send_normal_2:
	if (numArgsPushed != -1)  // special case flag meaning it is a primitive
		return errFailed;	// arguments remain on the stack

	msg = gSpecialBinarySelectors[g->primitiveIndex];
	sendMessage(g, msg, 2);
	return errNone;
}


int prNthPrime(VMGlobals *g, int numArgsPushed);
int prNthPrime(VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a;
	int n, p;

	a = g->sp;
	n = a->ui;
	p = nthPrime(n);
	if (p == 0) {
		SetNil(a);
	} else {
		SetInt(a, p);
	}
	return errNone;
}

int prPrevPrime(VMGlobals *g, int numArgsPushed);
int prPrevPrime(VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a;
	int n, p, i;

	a = g->sp;
	n = a->ui;
	i = prevPrime(n);
	p = nthPrime(i);
	if (p == 0) {
		SetNil(a);
	} else {
		SetInt(a, p);
	}
	return errNone;
}

int prNextPrime(VMGlobals *g, int numArgsPushed);
int prNextPrime(VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a;
	int n, p, i;

	a = g->sp;
	n = a->ui;
	i = nextPrime(n);
	p = nthPrime(i);
	if (p == 0) {
		SetNil(a);
	} else {
		SetInt(a, p);
	}
	return errNone;
}


int prIsPrime(VMGlobals *g, int numArgsPushed);
int prIsPrime(VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a;
	int n, p, sqrtn, i;

	a = g->sp;
	n = a->ui;
	SetNil(a);
	if (n <= 2) {
		if (n == 2) { SetTrue(a); }
		else { SetFalse(a); }
	} else if (n <= nthPrime(NUMPRIMES-1)) {
		// do a search of the primes table
		i = findPrime(n);
		if (i >= 0) { SetTrue(a); }
		else { SetFalse(a); }
	} else {
#ifdef SC_WIN32
		sqrtn = (int)sqrt(static_cast<double>(n));
#else
		sqrtn = (int)sqrt(n);
#endif
		for (i=0; i<NUMPRIMES; ++i) {
			p = nthPrime(i);
			if (n % p == 0) { SetFalse(a); break; }
			if (p >= sqrtn) { SetTrue(a); break; }
		}
	}
	return errNone;
}

int prIndexOfPrime(VMGlobals *g, int numArgsPushed);
int prIndexOfPrime(VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a;
	int n, p;

	a = g->sp;
	n = a->ui;
	if (n <= 2) {
		if (n == 2) { SetInt(a, 0); }
		else { SetNil(a); }
	} else if (n <= nthPrime(NUMPRIMES-1)) {
		p = findPrime(n);
		if (p < 0) {
			SetNil(a);
		} else {
			SetInt(a, p);
		}
	} else {
		SetNil(a);
	}
	return errNone;
}


int prAs32Bits(VMGlobals *g, int numArgsPushed);
int prAs32Bits(VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;
	// return an integer that is a bit pattern for the 32 bit float representation
	union { float f; int32 i; } u;
	u.f = a->uf;
	SetInt(a, u.i);
	return errNone;
}

int prHigh32Bits(VMGlobals *g, int numArgsPushed);
int prHigh32Bits(VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;

#if BYTE_ORDER == BIG_ENDIAN
	union { struct { uint32 hi, lo; } i; double f; } du;
#else
	union { struct { uint32 lo, hi; } i; double f; } du;
#endif

	du.f = a->uf;
	SetInt(a, du.i.hi);
	return errNone;
}

int prLow32Bits(VMGlobals *g, int numArgsPushed);
int prLow32Bits(VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;

#if BYTE_ORDER == BIG_ENDIAN
	union { struct { uint32 hi, lo; } i; double f; } du;
#else
	union { struct { uint32 lo, hi; } i; double f; } du;
#endif

	du.f = a->uf;
	SetInt(a, du.i.lo);
	return errNone;
}

int prFrom32Bits(VMGlobals *g, int numArgsPushed);
int prFrom32Bits(VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp - 1;
	PyrSlot *b = g->sp;
	int err, word;

	err = slotIntVal(b, &word);
	if (err) return err;

	union { float f; int32 i; } u;
	u.i = word;
	SetFloat(a, u.f);

	return errNone;
}

int prFrom64Bits(VMGlobals *g, int numArgsPushed);
int prFrom64Bits(VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp - 2;
	PyrSlot *b = g->sp - 1;
	PyrSlot *c = g->sp;
	int err, hi, lo;

	err = slotIntVal(b, &hi);
	if (err) return err;

	err = slotIntVal(c, &lo);
	if (err) return err;

#if BYTE_ORDER == BIG_ENDIAN
	union { struct { uint32 hi, lo; } i; double f; } du;
#else
	union { struct { uint32 lo, hi; } i; double f; } du;
#endif
	du.i.hi = hi;
	du.i.lo = lo;
	SetFloat(a, du.f);

	return errNone;
}

int mathClipInt(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a, *b, *c;
	double lo, hi;
	int err;

	a = g->sp - 2;
	b = g->sp - 1;
	c = g->sp;

	if (IsSym(b)) {
		*a = *b;
	} else if (IsSym(c)) {
		*a = *c;
	} else if (IsInt(b) && IsInt(c)) {
		a->ui = sc_clip(a->ui, b->ui, c->ui);
	} else {
		err = slotDoubleVal(b, &lo);
		if (err) return err;
		err = slotDoubleVal(c, &hi);
		if (err) return err;
		a->uf = sc_clip(a->ui, lo, hi);
	}
	return errNone;
}

int mathClipFloat(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a, *b, *c;
	double lo, hi;
	int err;

	a = g->sp - 2;
	b = g->sp - 1;
	c = g->sp;

	if (IsSym(b)) {
		*a = *b;
	} else if (IsSym(c)) {
		*a = *c;
	} else {
		err = slotDoubleVal(b, &lo);
		if (err) return err;
		err = slotDoubleVal(c, &hi);
		if (err) return err;
		a->uf = sc_clip(a->uf, lo, hi);
	}
	return errNone;
}

int mathClipSignal(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a, *b, *c;
	float lo, hi;
	int err;
	PyrObject *sig;

	a = g->sp - 2;
	b = g->sp - 1;
	c = g->sp;

	if (IsSym(b)) {
		*a = *b;
	} else if (IsSym(c)) {
		*a = *c;
	} else if (IsSignal(b) && IsSignal(c)) {
		sig = signal_clip_x(g, a->uo, b->uo, c->uo);
		SetObject(a, sig);
	} else {
		err = slotFloatVal(b, &lo);
		if (err) return err;
		err = slotFloatVal(c, &hi);
		if (err) return err;
		sig = signal_clip_f(g, a->uo, lo, hi);
		SetObject(a, sig);
	}
	return errNone;
}

int mathWrapInt(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a, *b, *c;
	int err;

	a = g->sp - 2;
	b = g->sp - 1;
	c = g->sp;

	if (IsSym(b)) {
		*a = *b;
	} else if (IsSym(c)) {
		*a = *c;
	} else if (IsInt(b) && IsInt(c)) {
		a->ui = sc_mod((int)(a->ui - b->ui), (int)(c->ui - b->ui + 1)) + b->ui;
	} else {
		double x, lo, hi;
		x = a->ui;
		err = slotDoubleVal(b, &lo);
		if (err) return err;
		err = slotDoubleVal(c, &hi);
		if (err) return err;
		a->uf = sc_mod(x - lo, hi - lo) + lo;
	}
	return errNone;
}

int mathWrapFloat(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a, *b, *c;
	double lo, hi;
	int err;

	a = g->sp - 2;
	b = g->sp - 1;
	c = g->sp;

	if (IsSym(b)) {
		*a = *b;
	} else if (IsSym(c)) {
		*a = *c;
	} else {
		err = slotDoubleVal(b, &lo);
		if (err) return err;
		err = slotDoubleVal(c, &hi);
		if (err) return err;
		a->uf = sc_mod(a->uf - lo, hi - lo) + lo;
	}
	return errNone;
}

int mathWrapSignal(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a, *b, *c;
	float lo, hi;
	int err;
	PyrObject *sig;

	a = g->sp - 2;
	b = g->sp - 1;
	c = g->sp;

	if (IsSym(b)) {
		*a = *b;
	} else if (IsSym(c)) {
		*a = *c;
	} else if (IsSignal(b) && IsSignal(c)) {
		sig = signal_wrap_x(g, a->uo, b->uo, c->uo);
		SetObject(a, sig);
	} else {
		err = slotFloatVal(b, &lo);
		if (err) return err;
		err = slotFloatVal(c, &hi);
		if (err) return err;
		sig = signal_wrap_f(g, a->uo, lo, hi);
		SetObject(a, sig);
	}
	return errNone;
}

int mathFoldInt(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a, *b, *c;
	int err;

	a = g->sp - 2;
	b = g->sp - 1;
	c = g->sp;

	if (IsSym(b)) {
		*a = *b;
	} else if (IsSym(c)) {
		*a = *c;
	} else if (IsInt(b) && IsInt(c)) {
		a->ui = sc_fold(a->ui, b->ui, c->ui);
	} else {
		double x, lo, hi;
		x = a->ui;
		err = slotDoubleVal(b, &lo);
		if (err) return err;
		err = slotDoubleVal(c, &hi);
		if (err) return err;
		a->uf = sc_fold(x, lo, hi);
	}
	return errNone;
}

int mathFoldFloat(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a, *b, *c;
	double lo, hi;
	int err;

	a = g->sp - 2;
	b = g->sp - 1;
	c = g->sp;

	if (IsSym(b)) {
		*a = *b;
	} else if (IsSym(c)) {
		*a = *c;
	} else {
		err = slotDoubleVal(b, &lo);
		if (err) return err;
		err = slotDoubleVal(c, &hi);
		if (err) return err;
		a->uf = sc_fold(a->uf, lo, hi);
	}
	return errNone;
}

int mathFoldSignal(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a, *b, *c;
	float lo, hi;
	int err;
	PyrObject *sig;

	a = g->sp - 2;
	b = g->sp - 1;
	c = g->sp;

	if (IsSym(b)) {
		*a = *b;
	} else if (IsSym(c)) {
		*a = *c;
	} else if (IsSignal(b) && IsSignal(c)) {
		sig = signal_fold_x(g, a->uo, b->uo, c->uo);
		SetObject(a, sig);
	} else {
		err = slotFloatVal(b, &lo);
		if (err) return err;
		err = slotFloatVal(c, &hi);
		if (err) return err;
		sig = signal_fold_f(g, a->uo, lo, hi);
		SetObject(a, sig);
	}
	return errNone;
}

int prSimpleNumberSeries(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp - 2;
	PyrSlot *b = g->sp - 1;
	PyrSlot *c = g->sp;

	int err, size;

	if (IsInt(a) && (IsInt(b) || IsNil(b)) && IsInt(c)) {
		int first, second, last, step;
		first = a->ui;
		last = c->ui;
		second = IsInt(b) ? b->ui : (first < last ? first + 1 : first - 1);
		step = second - first;

		if ( step == 0 )
			size = 1;
		else
			size = ((last - first) / step) + 1;
		if(size<1 || size > INT_MAX_BY_PyrSlot){
			post("prSimpleNumberSeries: array size %i exceeds limit (%i)\n", size, INT_MAX_BY_PyrSlot);
			return errFailed;
		}

		PyrObject *obj = newPyrArray(g->gc, size, 0, true);
		obj->size = size;
		PyrSlot *slots = obj->slots;
		if(step==1){
			// Faster iteration for common case
			if(first==0){
				for (int i=0; i<size; ++i) {
					SetInt(slots+i, i);
				}
			}else{
				for (int i=0; i<size; ++i) {
					SetInt(slots+i, first++);
				}
			}
		}else{
			int val = first;
			for (int i=0; i<size; ++i) {
				SetInt(slots+i, val);
				val += step;
			}
		}
		SetObject(a, obj);
	} else {
		double first, second, last, step;
		err = slotDoubleVal(a, &first);
		if (err) return err;
		err = slotDoubleVal(c, &last);
		if (err) return err;
		err = slotDoubleVal(b, &second);
		if (err) {
			if (first < last) second = first + 1.;
			else second = first - 1.;
		}

		step = second - first;
		size = (int)floor((last - first) / step + 0.001) + 1;
		if(size<1 || size > INT_MAX_BY_PyrSlot){
			post("prSimpleNumberSeries: array size %i exceeds limit (%i)\n", size, INT_MAX_BY_PyrSlot);
			return errFailed;
		}
		PyrObject *obj = newPyrArray(g->gc, size, 0, true);
		obj->size = size;
		PyrSlot *slots = obj->slots;
		if(first==0. && step==1.){
			// Faster iteration for common case
			for (long i=0; i<size; ++i) {
				SetFloat(slots+i, i);
			}
		}else{
			double val = first;
			for (long i=0; i<size; ++i) {
				val = first + step * i;
				SetFloat(slots+i, val);
			}
		}
		SetObject(a, obj);
	}
	return errNone;

}

/*

	asFraction {|maxDenominator=100|
		var mediant, lower, upper, temp;
		var n,d, k, k1;
		if (this < 0) {
			#n, d = this.neg.asFraction(maxDenominator);
			^[n.neg, d]
		};
		if (this < 1.0) {
			upper = [1.0, this.reciprocal.floor];
			lower = [1.0, upper[1]+1.0];
		}{
			lower = [this.floor, 1.0];
			upper = [lower[0]+1.0, 1.0];
		};
		mediant = [lower[0] + upper[0], lower[1] + upper[1]];
		loop {
			mediant = [lower[0] + upper[0], lower[1] + upper[1]];
			case
			{ (this * mediant[1]) > mediant[0] }
			{
				if (maxDenominator < mediant[1]) {^upper};
				d = upper[0] - (this * upper[1]);
				if (d == 0) {^upper};
				lower = mediant;
				k = floor(((this * lower[1]) - lower[0]) / d);
				k1 = k + 1;
				temp = [lower[0] + (k1 * upper[0]), lower[1] + (k1 * upper[1])];
				lower = [lower[0] + (k * upper[0]), lower[1] + (k * upper[1])];
				upper = temp;
			}
			{ (this * mediant[1]) == mediant[0] }
			{
				if (maxDenominator >= mediant[1]) {^mediant};
				if (lower[1] < upper[1]) {^lower};
				^upper
			}
			{
				if (maxDenominator < mediant[1]) {^lower};
				d = lower[0] - (this * lower[1]);
				if (d == 0) {^lower};
				upper = mediant;
				k = floor(((this * upper[1]) - upper[0]) / d);
				k1 = k + 1;
				temp = [(k1 * lower[0]) + upper[0], (k1 * lower[1]) + upper[1]];
				upper = [(k * lower[0]) + upper[0], (k * lower[1]) + upper[1]];
				lower = temp;
			};
		}
	}

*/

int prAsFraction(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp - 2;
	PyrSlot *b = g->sp - 1;
	PyrSlot *c = g->sp;

	double mediant_num, lower_num, upper_num, temp_num;
	double mediant_den, lower_den, upper_den, temp_den;
	double x, d;
	int k, k1;
	int maxDenominator;
	int err;
	bool neg = false;

	err = slotDoubleVal(a, &x);
	if (err) return err;

	err = slotIntVal(b, &maxDenominator);
	if (err) return err;

	bool faster = IsTrue(c);

	PyrObject *obj = newPyrArray(g->gc, 2, 0, true);
	obj->size = 2;
	PyrSlot *slots = obj->slots;
	SetObject(a, obj);

	if (x < 0.0) {
		x = -x;
		neg = true;
	}

	if (x < 1.0) {
		upper_num = 1.0;
		upper_den = floor(1./x);
		lower_num = 1.0;
		lower_den = upper_den + 1.;
	} else {
		lower_num = floor(x);
		lower_den = 1.0;
		upper_num = lower_num + 1.;
		upper_den = 1.0;
	}

	while (true) {
		mediant_num = lower_num + upper_num;
		mediant_den = lower_den + upper_den;
		//post("  md %g %g    %g %g    %g %g\n", mediant_num, mediant_den, lower_num, lower_den, upper_num, upper_den);

		if (x * mediant_den > mediant_num) {
			d = upper_num - (x * upper_den);
			if (maxDenominator < mediant_den || fabs(d) < 1e-5) {
				if (neg) upper_num = -upper_num;
				SetInt(slots+0, (int)upper_num);
				SetInt(slots+1, (int)upper_den);
				return errNone;
			}
			lower_num = mediant_num;
			lower_den = mediant_den;
			if (faster) {
				k = (int)floor(((x * lower_den) - lower_num) / d);
				if (k < 10000) {
					k1 = k + 1;
					temp_num = lower_num + (k1 * upper_num);
					temp_den = lower_den + (k1 * upper_den);
					lower_num = lower_num + (k * upper_num);
					lower_den = lower_den + (k * upper_den);
					upper_num = temp_num;
					upper_den = temp_den;
				}
			}
		} else if (x * mediant_den == mediant_num) {
			if (maxDenominator >= mediant_den) {
				if (neg) mediant_num = -mediant_num;
				SetInt(slots+0, (int)mediant_num);
				SetInt(slots+1, (int)mediant_den);
				return errNone;
			} else if (lower_den < upper_den) {
				if (neg) lower_num = -lower_num;
				SetInt(slots+0, (int)lower_num);
				SetInt(slots+1, (int)lower_den);
				return errNone;
			} else {
				if (neg) upper_num = -upper_num;
				SetInt(slots+0, (int)upper_num);
				SetInt(slots+1, (int)upper_den);
				return errNone;
			}
		} else {
			d = lower_num - (x * lower_den);
			if (maxDenominator < mediant_den || fabs(d) < 1e-5) {
				if (neg) lower_num = -lower_num;
				SetInt(slots+0, (int)lower_num);
				SetInt(slots+1, (int)lower_den);
				return errNone;
			}
			upper_num = mediant_num;
			upper_den = mediant_den;
			if (faster) {
				k = (int)floor(((x * upper_den) - upper_num) / d);
				if (k < 10000) {
					k1 = k + 1;
					temp_num = (k1 * lower_num) + upper_num;
					temp_den = (k1 * lower_den) + upper_den;
					upper_num = (k * lower_num) + upper_num;
					upper_den = (k * lower_den) + upper_den;
					lower_num = temp_num;
					lower_den = temp_den;
				}
			}
		}
	}
}

void initMathPrimitives()
{
	int base, index;

	base = nextPrimitiveIndex();
	index = 0;
	definePrimitive(base, index++, "_AddInt", prAddInt, 2, 0);
	definePrimitive(base, index++, "_SubInt", prSubInt, 2, 0);
	definePrimitive(base, index++, "_MulInt", prMulInt, 2, 0);
	definePrimitive(base, index++, "_AddFloat", prAddFloat, 2, 0);
	definePrimitive(base, index++, "_SubFloat", prSubFloat, 2, 0);
	definePrimitive(base, index++, "_MulFloat", prMulFloat, 2, 0);
	definePrimitive(base, index++, "_NthPrime", prNthPrime, 1, 0);
	definePrimitive(base, index++, "_PrevPrime", prPrevPrime, 1, 0);
	definePrimitive(base, index++, "_NextPrime", prNextPrime, 1, 0);
	definePrimitive(base, index++, "_IsPrime", prIsPrime, 1, 0);
	definePrimitive(base, index++, "_IndexOfPrime", prIndexOfPrime, 1, 0);
	definePrimitive(base, index++, "_As32Bits", prAs32Bits, 1, 0);
	definePrimitive(base, index++, "_High32Bits", prHigh32Bits, 1, 0);
	definePrimitive(base, index++, "_Low32Bits", prLow32Bits, 1, 0);
	definePrimitive(base, index++, "_From32Bits", prFrom32Bits, 2, 0);
	definePrimitive(base, index++, "_From64Bits", prFrom64Bits, 3, 0);

	definePrimitive(base, index++, "_ClipInt", mathClipInt, 3, 0);
	definePrimitive(base, index++, "_ClipFloat", mathClipFloat, 3, 0);
	definePrimitive(base, index++, "_ClipSignal", mathClipSignal, 3, 0);
	definePrimitive(base, index++, "_WrapInt", mathWrapInt, 3, 0);
	definePrimitive(base, index++, "_WrapFloat", mathWrapFloat, 3, 0);
	definePrimitive(base, index++, "_WrapSignal", mathWrapSignal, 3, 0);
	definePrimitive(base, index++, "_FoldInt", mathFoldInt, 3, 0);
	definePrimitive(base, index++, "_FoldFloat", mathFoldFloat, 3, 0);
	definePrimitive(base, index++, "_FoldSignal", mathFoldSignal, 3, 0);

	definePrimitive(base, index++, "_SimpleNumberSeries", prSimpleNumberSeries, 3, 0);
	definePrimitive(base, index++, "_AsFraction", prAsFraction, 3, 0);
}

