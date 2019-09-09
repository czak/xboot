#ifndef __GRAPHIC_REGION_H__
#define __GRAPHIC_REGION_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <string.h>

struct region_t {
	int x, y;
	int w, h;
	int area;
};

static inline void region_init(struct region_t * r, int x, int y, int w, int h)
{
	r->x = x;
	r->y = y;
	r->w = w;
	r->h = h;
	r->area = -1;
}

static inline int region_area(struct region_t * r)
{
	if(r->area < 0)
		r->area = r->w * r->h;
	return r->area;
}

static inline void region_clone(struct region_t * r, struct region_t * o)
{
	memcpy(r, o, sizeof(struct region_t));
}

static inline int region_isempty(struct region_t * r)
{
	if((r->w > 0) && (r->h > 0))
		return 0;
	return 1;
}

static inline int region_contains(struct region_t * r, struct region_t * o)
{
	int rr = r->x + r->w;
	int rb = r->y + r->h;
	int or = o->x + o->w;
	int ob = o->y + o->h;
	if((o->x >= r->x) && (o->x < rr) && (o->y >= r->y) && (o->y < rb) && (or > r->x) && (or <= rr) && (ob > r->y) && (ob <= rb))
		return 1;
	return 0;
}

static inline int region_intersect(struct region_t * r, struct region_t * a, struct region_t * b)
{
	int x0 = max(a->x, b->x);
	int x1 = min(a->x + a->w, b->x + b->w);
	if(x0 <= x1)
	{
		int y0 = max(a->y, b->y);
		int y1 = min(a->y + a->h, b->y + b->h);
		if(y0 <= y1)
		{
			r->x = x0;
			r->y = y0;
			r->w = x1 - x0;
			r->h = y1 - y0;
			r->area = -1;
			return 1;
		}
	}
	return 0;
}

static inline int region_union(struct region_t * r, struct region_t * a, struct region_t * b)
{
	int ar = a->x + a->w;
	int ab = a->y + a->h;
	int br = b->x + b->w;
	int bb = b->y + b->h;
	r->x = min(a->x, b->x);
	r->y = min(a->y, b->y);
	r->w = max(ar, br) - r->x;
	r->h = max(ab, bb) - r->y;
	r->area = -1;
	return 1;
}

enum {
	OUTCODE_BOTTOM	= 0x1,
	OUTCODE_TOP	= 0x2,
	OUTCODE_LEFT	= 0x4,
	OUTCODE_RIGHT	= 0x8,
};

static inline int compute_outcode(struct region_t * r, int x, int y)
{
	int code = 0;
	if(y < 0)
		code |= OUTCODE_TOP;
	else if(y >= r->y + r->h)
		code |= OUTCODE_BOTTOM;
	if(x < 0)
		code |= OUTCODE_LEFT;
	else if(x >= r->x + r->w)
		code |= OUTCODE_RIGHT;
	return code;
}

static inline int region_clip_line(struct region_t * r, int * x0, int * y0, int * x1, int * y1)
{
	int _x0 = *x0;
	int _y0 = *y0;
	int _x1 = *x1;
	int _y1 = *y1;
	int rx0 = r->x;
	int ry0 = r->y;
	int rx1 = r->x + r->w - 1;
	int ry1 = r->y + r->h - 1;
	int x = 0, y = 0;
	int code1, code2;

	if(_x0 >= rx0 && _x0 <= rx1 && _x1 >= rx0 && _x1 <= rx1 && _y0 >= ry0 && _y0 <= ry1 && _y1 >= ry0 && _y1 <= ry1)
		return 1;
	if((_x0 < rx0 && _x1 < rx0) || (_x0 > rx1 && _x1 > rx1) || (_y0 < ry0 && _y1 < ry0) || (_y0 > ry1 && _y1 > ry1))
		return 0;
	if(_y0 == _y1)
	{
		if(_x0 < rx0)
			*x0 = rx0;
		else if(_x0 > rx1)
			*x0 = rx1;
		if(_x1 < rx0)
			*x1 = rx0;
		else if(_x1 > rx1)
			*x1 = rx1;
		return 1;
	}
	if(_x0 == _x1)
	{
		if(_y0 < ry0)
			*y0 = ry0;
		else if(_y0 > ry1)
			*y0 = ry1;
		if(_y1 < ry0)
			*y1 = ry0;
		else if(_y1 > ry1)
			*y1 = ry1;
		return 1;
	}
	code1 = compute_outcode(r, _x0, _y0);
	code2 = compute_outcode(r, _x1, _y1);
	while(code1 || code2)
	{
		if(code1 & code2)
			return 0;
		if(code1)
		{
			if(code1 & OUTCODE_TOP)
			{
				y = ry0;
				x = _x0 + ((_x1 - _x0) * (y - _y0)) / (_y1 - _y0);
			}
			else if(code1 & OUTCODE_BOTTOM)
			{
				y = ry1;
				x = _x0 + ((_x1 - _x0) * (y - _y0)) / (_y1 - _y0);
			}
			else if(code1 & OUTCODE_LEFT)
			{
				x = rx0;
				y = _y0 + ((_y1 - _y0) * (x - _x0)) / (_x1 - _x0);
			}
			else if(code1 & OUTCODE_RIGHT)
			{
				x = rx1;
				y = _y0 + ((_y1 - _y0) * (x - _x0)) / (_x1 - _x0);
			}
			_x0 = x;
			_y0 = y;
			code1 = compute_outcode(r, x, y);
		}
		else
		{
			if(code2 & OUTCODE_TOP)
			{
				y = ry0;
				x = _x0 + ((_x1 - _x0) * (y - _y0)) / (_y1 - _y0);
			}
			else if(code2 & OUTCODE_BOTTOM)
			{
				y = ry1;
				x = _x0 + ((_x1 - _x0) * (y - _y0)) / (_y1 - _y0);
			}
			else if(code2 & OUTCODE_LEFT)
			{
				x = rx0;
				y = _y0 + ((_y1 - _y0) * (x - _x0)) / (_x1 - _x0);
			}
			else if(code2 & OUTCODE_RIGHT)
			{
				x = rx1;
				y = _y0 + ((_y1 - _y0) * (x - _x0)) / (_x1 - _x0);
			}
			_x1 = x;
			_y1 = y;
			code2 = compute_outcode(r, x, y);
		}
	}
	*x0 = _x0;
	*y0 = _y0;
	*x1 = _x1;
	*y1 = _y1;
	return 1;
}

struct region_list_t {
	struct region_t * region;
	unsigned int size;
	unsigned int count;
};

struct region_list_t * region_list_alloc(unsigned int size);
void region_list_free(struct region_list_t * rl);
void region_list_clone(struct region_list_t * rl, struct region_list_t * o);
void region_list_merge(struct region_list_t * rl, struct region_list_t * o);
void region_list_add(struct region_list_t * rl, struct region_t * r);
void region_list_clear(struct region_list_t * rl);

#ifdef __cplusplus
}
#endif

#endif /* __GRAPHIC_REGION_H__ */
