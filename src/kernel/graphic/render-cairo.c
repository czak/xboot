/*
 * kernel/graphic/render-cairo.c
 *
 * Copyright(c) 2007-2019 Jianjun Jiang <8192542@qq.com>
 * Official site: http://xboot.org
 * Mobile phone: +86-18665388956
 * QQ: 8192542
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <xboot.h>
#include <cairo.h>
#include <cairoint.h>
#include <cairo-ft.h>
#include <graphic/surface.h>

struct render_cairo_context_t {
	cairo_surface_t * cs;
	cairo_t * cr;
};

static void * render_cairo_create(struct surface_t * s)
{
	struct render_cairo_context_t * ctx;

	ctx = malloc(sizeof(struct render_cairo_context_t));
	if(!ctx)
		return NULL;
	ctx->cs = cairo_image_surface_create_for_data((unsigned char *)s->pixels, CAIRO_FORMAT_ARGB32, s->width, s->height, s->stride);
	ctx->cr = cairo_create(ctx->cs);
	return ctx;
}

static void render_cairo_destroy(void * pctx)
{
	struct render_cairo_context_t * ctx = (struct render_cairo_context_t *)pctx;
	cairo_destroy(ctx->cr);
	cairo_surface_destroy(ctx->cs);
	free(ctx);
}

static void render_cairo_blit(struct surface_t * s, struct matrix_t * m, struct surface_t * src, double alpha)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_save(cr);
	cairo_set_matrix(cr, (cairo_matrix_t *)m);
	cairo_set_source_surface(cr, ((struct render_cairo_context_t *)src->pctx)->cs, 0, 0);
	cairo_paint_with_alpha(cr, alpha);
	cairo_restore(cr);
}

static void render_cairo_mask(struct surface_t * s, struct matrix_t * m, struct surface_t * src, struct surface_t * mask)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_save(cr);
	cairo_set_matrix(cr, (cairo_matrix_t *)m);
	cairo_set_source_surface(cr, ((struct render_cairo_context_t *)src->pctx)->cs, 0, 0);
	cairo_mask_surface(cr, ((struct render_cairo_context_t *)mask->pctx)->cs, 0, 0);
	cairo_restore(cr);
}

static void render_cairo_fill(struct surface_t * s, struct matrix_t * m, struct region_t * r, struct color_t * c)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_save(cr);
	cairo_set_matrix(cr, (cairo_matrix_t *)m);
	cairo_rectangle(cr, r->x, r->y, r->w, r->h);
	cairo_set_source_rgba(cr, c->r / 255.0, c->g / 255.0, c->b / 255.0, c->a / 255.0);
	cairo_fill(cr);
	cairo_restore(cr);
}

static void render_cairo_text(struct surface_t * s, struct matrix_t * m, const char * utf8, struct color_t * c, void * sfont, int size)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_text_extents_t e;
	cairo_save(cr);
	cairo_set_font_face(cr, (cairo_font_face_t *)sfont);
	cairo_set_font_size(cr, size);
	cairo_move_to(cr, m->tx, m->ty);
	cairo_set_matrix(cr, (cairo_matrix_t *)m);
	cairo_text_extents(cr, utf8, &e);
	cairo_move_to(cr, e.x_bearing, -e.y_bearing);
	cairo_set_source_rgba(cr, c->r / 255.0, c->g / 255.0, c->b / 255.0, c->a / 255.0);
	cairo_show_text(cr, utf8);
	cairo_restore(cr);
}

static void render_cairo_extent(struct surface_t * s, const char * utf8, void * sfont, int size, struct region_t * e)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_text_extents_t ext;
	cairo_save(cr);
	cairo_set_font_face(cr, (cairo_font_face_t *)sfont);
	cairo_set_font_size(cr, size);
	cairo_text_extents(cr, utf8, &ext);
	cairo_restore(cr);
	region_init(e, ext.x_bearing, -ext.y_bearing, ext.width, ext.height);
}

static void render_cairo_shape_save(struct surface_t * s)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_save(cr);
}

static void render_cairo_shape_restore(struct surface_t * s)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_restore(cr);
}

static void render_cairo_shape_push_group(struct surface_t * s)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_push_group(cr);
}

static void render_cairo_shape_pop_group(struct surface_t * s)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_pop_group(cr);
}

static void render_cairo_shape_pop_group_to_source(struct surface_t * s)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_pop_group_to_source(cr);
}

static void render_cairo_shape_new_path(struct surface_t * s)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_new_path(cr);
}

static void render_cairo_shape_new_sub_path(struct surface_t * s)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_new_sub_path(cr);
}

static void render_cairo_shape_close_path(struct surface_t * s)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_close_path(cr);
}

static void render_cairo_shape_set_operator(struct surface_t * s, const char * type)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	switch(shash(type))
	{
	case 0x0f3b6d8c: /* "clear" */
		cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
		break;
	case 0x1c3aff76: /* "source" */
		cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
		break;
	case 0x7c9bf101: /* "over" */
		cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
		break;
	case 0x0059783c: /* "in" */
		cairo_set_operator(cr, CAIRO_OPERATOR_IN);
		break;
	case 0x0b889a9d: /* "out" */
		cairo_set_operator(cr, CAIRO_OPERATOR_OUT);
		break;
	case 0x7c943c79: /* "atop" */
		cairo_set_operator(cr, CAIRO_OPERATOR_ATOP);
		break;
	case 0x7c95a255: /* "dest" */
		cairo_set_operator(cr, CAIRO_OPERATOR_DEST);
		break;
	case 0x4f1e8f9e: /* "dest-over" */
		cairo_set_operator(cr, CAIRO_OPERATOR_DEST_OVER);
		break;
	case 0x0979cb99: /* "dest-in" */
		cairo_set_operator(cr, CAIRO_OPERATOR_DEST_IN);
		break;
	case 0x38b3599a: /* "dest-out" */
		cairo_set_operator(cr, CAIRO_OPERATOR_DEST_OUT);
		break;
	case 0x4f16db16: /* "dest-atop" */
		cairo_set_operator(cr, CAIRO_OPERATOR_DEST_ATOP);
		break;
	case 0x0b88c01e: /* "xor" */
		cairo_set_operator(cr, CAIRO_OPERATOR_XOR);
		break;
	case 0x0b885cce: /* "add" */
		cairo_set_operator(cr, CAIRO_OPERATOR_ADD);
		break;
	case 0xdf32bb4e: /* "saturate" */
		cairo_set_operator(cr, CAIRO_OPERATOR_SATURATE);
		break;
	case 0x46746f05: /* "multiply" */
		cairo_set_operator(cr, CAIRO_OPERATOR_MULTIPLY);
		break;
	case 0x1b5ffd45: /* "screen" */
		cairo_set_operator(cr, CAIRO_OPERATOR_SCREEN);
		break;
	case 0x7ee4b5c7: /* "overlay" */
		cairo_set_operator(cr, CAIRO_OPERATOR_OVERLAY);
		break;
	case 0xf83e845a: /* "darken" */
		cairo_set_operator(cr, CAIRO_OPERATOR_DARKEN);
		break;
	case 0x79c1c710: /* "lighten" */
		cairo_set_operator(cr, CAIRO_OPERATOR_LIGHTEN);
		break;
	case 0x8b9b2114: /* "color-dodge" */
		cairo_set_operator(cr, CAIRO_OPERATOR_COLOR_DODGE);
		break;
	case 0x61514968: /* "color-burn" */
		cairo_set_operator(cr, CAIRO_OPERATOR_COLOR_BURN);
		break;
	case 0xb5060029: /* "hard-light" */
		cairo_set_operator(cr, CAIRO_OPERATOR_HARD_LIGHT);
		break;
	case 0xfcd63c26: /* "soft-light" */
		cairo_set_operator(cr, CAIRO_OPERATOR_SOFT_LIGHT);
		break;
	case 0x52a92470: /* "difference" */
		cairo_set_operator(cr, CAIRO_OPERATOR_DIFFERENCE);
		break;
	case 0x6f499bff: /* "exclusion" */
		cairo_set_operator(cr, CAIRO_OPERATOR_EXCLUSION);
		break;
	case 0x5d66597b: /* "hsl-hue" */
		cairo_set_operator(cr, CAIRO_OPERATOR_HSL_HUE);
		break;
	case 0x1f46f0c3: /* "hsl-saturation" */
		cairo_set_operator(cr, CAIRO_OPERATOR_HSL_SATURATION);
		break;
	case 0x50050bf8: /* "hsl-color" */
		cairo_set_operator(cr, CAIRO_OPERATOR_HSL_COLOR);
		break;
	case 0x02d64a96: /* "hsl-luminosity" */
		cairo_set_operator(cr, CAIRO_OPERATOR_HSL_LUMINOSITY);
		break;
	default:
		break;
	}
}

static void render_cairo_shape_set_source(struct surface_t * s, void * pattern)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_set_source(cr, pattern);
}

static void * render_cairo_shape_get_source(struct surface_t * s)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	return cairo_get_source(cr);
}

static void render_cairo_shape_set_source_color(struct surface_t * s, double r, double g, double b, double a)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_set_source_rgba(cr, r, g, b, a);
}

static void render_cairo_shape_set_source_surface(struct surface_t * s, struct surface_t * o, double x, double y)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_set_source_surface(cr, ((struct render_cairo_context_t *)o->pctx)->cs, x, y);
}

static void render_cairo_shape_set_tolerance(struct surface_t * s, double tolerance)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_set_tolerance(cr, tolerance);
}

static void render_cairo_shape_set_miter_limit(struct surface_t * s, double limit)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_set_miter_limit(cr, limit);
}

static void render_cairo_shape_set_antialias(struct surface_t * s, const char * type)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	switch(shash(type))
	{
	case 0x0885548a: /* "default" */
		cairo_set_antialias(cr, CAIRO_ANTIALIAS_DEFAULT);
		break;
	case 0x7c9b47f5: /* "none" */
		cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
		break;
	case 0x7c977c78: /* "gray" */
		cairo_set_antialias(cr, CAIRO_ANTIALIAS_GRAY);
		break;
	case 0xb8639511: /* "subpixel" */
		cairo_set_antialias(cr, CAIRO_ANTIALIAS_SUBPIXEL);
		break;
	case 0x7c96aa13: /* "fast" */
		cairo_set_antialias(cr, CAIRO_ANTIALIAS_FAST);
		break;
	case 0x7c97716e: /* "good" */
		cairo_set_antialias(cr, CAIRO_ANTIALIAS_GOOD);
		break;
	case 0x7c948993: /* "best" */
		cairo_set_antialias(cr, CAIRO_ANTIALIAS_BEST);
		break;
	default:
		break;
	}
}

static void render_cairo_shape_set_fill_rule(struct surface_t * s, const char * type)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	switch(shash(type))
	{
	case 0xc8feafb5: /* "winding" */
		cairo_set_fill_rule(cr, CAIRO_FILL_RULE_WINDING);
		break;
	case 0x207e6937: /* "even-odd" */
		cairo_set_fill_rule(cr, CAIRO_FILL_RULE_EVEN_ODD);
		break;
	default:
		break;
	}
}

static void render_cairo_shape_set_line_width(struct surface_t * s, double width)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_set_line_width(cr, width);
}

static void render_cairo_shape_set_line_cap(struct surface_t * s, const char * type)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	switch(shash(type))
	{
	case 0x7c94cdc4: /* "butt" */
		cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
		break;
	case 0x104cc7ed: /* "round" */
		cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
		break;
	case 0x1c5eea16: /* "square" */
		cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
		break;
	default:
		break;
	}
}

static void render_cairo_shape_set_line_join(struct surface_t * s, const char * type)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	switch(shash(type))
	{
	case 0x0feefdc6: /* "miter" */
		cairo_set_line_join(cr, CAIRO_LINE_JOIN_MITER);
		break;
	case 0x104cc7ed: /* "round" */
		cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
		break;
	case 0x0f25c733: /* "bevel" */
		cairo_set_line_join(cr, CAIRO_LINE_JOIN_BEVEL);
		break;
	default:
		break;
	}
}

static void render_cairo_shape_set_dash(struct surface_t * s, const double * dashes, int ndashes, double offset)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_set_dash(cr, dashes, ndashes, offset);
}

static void render_cairo_shape_identity(struct surface_t * s)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_identity_matrix(cr);
}

static void render_cairo_shape_translate(struct surface_t * s, double tx, double ty)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_translate(cr, tx, ty);
}

static void render_cairo_shape_scale(struct surface_t * s, double sx, double sy)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_scale(cr, sx, sy);
}

static void render_cairo_shape_rotate(struct surface_t * s, double angle)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_rotate(cr, angle);
}

static void render_cairo_shape_transform(struct surface_t * s, struct matrix_t * m)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_transform(cr, (cairo_matrix_t *)m);
}

static void render_cairo_shape_set_matrix(struct surface_t * s, struct matrix_t * m)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_set_matrix(cr, (cairo_matrix_t *)m);
}

static void render_cairo_shape_get_matrix(struct surface_t * s, struct matrix_t * m)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_get_matrix(cr, (cairo_matrix_t *)m);
}

static void render_cairo_shape_move_to(struct surface_t * s, double x, double y)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_move_to(cr, x, y);
}

static void render_cairo_shape_rel_move_to(struct surface_t * s, double dx, double dy)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_rel_move_to(cr, dx, dy);
}

static void render_cairo_shape_line_to(struct surface_t * s, double x, double y)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_line_to(cr, x, y);
}

static void render_cairo_shape_rel_line_to(struct surface_t * s, double dx, double dy)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_rel_line_to(cr, dx, dy);
}

static void render_cairo_shape_curve_to(struct surface_t * s, double x1, double y1, double x2, double y2, double x3, double y3)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_curve_to(cr, x1, y1, x2, y2, x3, y3);
}

static void render_cairo_shape_rel_curve_to(struct surface_t * s, double dx1, double dy1, double dx2, double dy2, double dx3, double dy3)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_rel_curve_to(cr, dx1, dy1, dx2, dy2, dx3, dy3);
}

static void render_cairo_shape_rectangle(struct surface_t * s, double x, double y, double w, double h)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_rectangle(cr, x, y, w, h);
}

static void render_cairo_shape_rounded_rectangle(struct surface_t * s, double x, double y, double w, double h, double r)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_move_to(cr, x + r, y);
	cairo_line_to(cr, x + w - r, y);
	cairo_arc(cr, x + w - r, y + r, r, - M_PI / 2, 0);
	cairo_line_to(cr, x + w, y + h - r);
	cairo_arc(cr, x + w - r, y + h - r, r, 0, M_PI / 2);
	cairo_line_to(cr, x + r, y + h);
	cairo_arc(cr, x + r, y + h - r, r, M_PI / 2, M_PI);
	cairo_line_to(cr, x, y + r);
	cairo_arc(cr, x + r, y + r, r, M_PI, M_PI + M_PI / 2);
}

static void render_cairo_shape_arc(struct surface_t * s, double xc, double yc, double r, double a1, double a2)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_arc(cr, xc, yc, r, a1, a2);
}

static void render_cairo_shape_arc_negative(struct surface_t * s, double xc, double yc, double r, double a1, double a2)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_arc_negative(cr, xc, yc, r, a1, a2);
}

static void render_cairo_shape_stroke(struct surface_t * s)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_stroke(cr);
}

static void render_cairo_shape_stroke_preserve(struct surface_t * s)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_stroke_preserve(cr);
}

static void render_cairo_shape_fill(struct surface_t * s)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_fill(cr);
}

static void render_cairo_shape_fill_preserve(struct surface_t * s)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_fill_preserve(cr);
}

static void render_cairo_shape_reset_clip(struct surface_t * s)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_reset_clip(cr);
}

static void render_cairo_shape_clip(struct surface_t * s)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_clip(cr);
}

static void render_cairo_shape_clip_preserve(struct surface_t * s)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_clip_preserve(cr);
}

static void render_cairo_shape_mask(struct surface_t * s, void * pattern)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_mask(cr, pattern);
}

static void render_cairo_shape_mask_surface(struct surface_t * s, struct surface_t * o, double x, double y)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_mask_surface(cr, ((struct render_cairo_context_t *)s->pctx)->cs, x, y);
}

static void render_cairo_shape_paint(struct surface_t * s, double alpha)
{
	cairo_t * cr = ((struct render_cairo_context_t *)s->pctx)->cr;
	cairo_paint_with_alpha(cr, alpha);
}

static void * render_cairo_pattern_create_color(double r, double g, double b, double a)
{
	return cairo_pattern_create_rgba(r, g, b, a);
}

static void * render_cairo_pattern_create_linear(double x0, double y0, double x1, double y1)
{
	return cairo_pattern_create_linear(x0, y0, x1, y1);
}

static void * render_cairo_pattern_create_radial(double x0, double y0, double r0, double x1, double y1, double r1)
{
	return cairo_pattern_create_radial(x0, y0, r0, x1, y1, r1);
}

static void * render_cairo_font_create(void * font)
{
	return cairo_ft_font_face_create_for_ft_face((FT_Face)font, 0);
}

static void render_cairo_font_destroy(void * sfont)
{
	cairo_font_face_destroy((cairo_font_face_t *)sfont);
}

static void * render_cairo_pattern_create(struct surface_t * s)
{
	return cairo_pattern_create_for_surface(((struct render_cairo_context_t *)s->pctx)->cs);
}

static void render_cairo_pattern_destroy(void * pattern)
{
	cairo_pattern_destroy((cairo_pattern_t *)pattern);
}

static void render_cairo_pattern_add_color_stop(void * pattern, double o, double r, double g, double b, double a)
{
	cairo_pattern_add_color_stop_rgba((cairo_pattern_t *)pattern, o, r, g, b, a);
}

static void render_cairo_pattern_set_extend(void * pattern, const char * type)
{
	switch(shash(type))
	{
	case 0x7c9b47f5: /* "none" */
		cairo_pattern_set_extend((cairo_pattern_t *)pattern, CAIRO_EXTEND_NONE);
		break;
	case 0x192dec66: /* "repeat" */
		cairo_pattern_set_extend((cairo_pattern_t *)pattern, CAIRO_EXTEND_REPEAT);
		break;
	case 0x3e3a6a0a: /* "reflect" */
		cairo_pattern_set_extend((cairo_pattern_t *)pattern, CAIRO_EXTEND_REFLECT);
		break;
	case 0x0b889c3a: /* "pad" */
		cairo_pattern_set_extend((cairo_pattern_t *)pattern, CAIRO_EXTEND_PAD);
		break;
	default:
		break;
	}
}

static void render_cairo_pattern_set_filter(void * pattern, const char * type)
{
	switch(shash(type))
	{
	case 0x7c96aa13: /* "fast" */
		cairo_pattern_set_filter((cairo_pattern_t *)pattern, CAIRO_FILTER_FAST);
		break;
	case 0x7c97716e: /* "good" */
		cairo_pattern_set_filter((cairo_pattern_t *)pattern, CAIRO_FILTER_GOOD);
		break;
	case 0x7c948993: /* "best" */
		cairo_pattern_set_filter((cairo_pattern_t *)pattern, CAIRO_FILTER_BEST);
		break;
	case 0x09fa48d7: /* "nearest" */
		cairo_pattern_set_filter((cairo_pattern_t *)pattern, CAIRO_FILTER_NEAREST);
		break;
	case 0x8320f06b: /* "bilinear" */
		cairo_pattern_set_filter((cairo_pattern_t *)pattern, CAIRO_FILTER_BILINEAR);
		break;
	case 0xce4e6460: /* "gaussian" */
		cairo_pattern_set_filter((cairo_pattern_t *)pattern, CAIRO_FILTER_GAUSSIAN);
		break;
	default:
		break;
	}
}

static void render_cairo_pattern_set_matrix(void * pattern, struct matrix_t * m)
{
	cairo_pattern_set_matrix((cairo_pattern_t *)pattern, (cairo_matrix_t *)m);
}

static struct render_t render_cairo = {
	.name	 					= "cairo",

	.create						= render_cairo_create,
	.destroy					= render_cairo_destroy,

	.blit						= render_cairo_blit,
	.mask						= render_cairo_mask,
	.fill						= render_cairo_fill,
	.text						= render_cairo_text,
	.extent						= render_cairo_extent,

	.filter_haldclut			= render_default_filter_haldclut,
	.filter_grayscale			= render_default_filter_grayscale,
	.filter_sepia				= render_default_filter_sepia,
	.filter_invert				= render_default_filter_invert,
	.filter_threshold			= render_default_filter_threshold,
	.filter_colorize			= render_default_filter_colorize,
	.filter_hue					= render_default_filter_hue,
	.filter_saturate			= render_default_filter_saturate,
	.filter_brightness			= render_default_filter_brightness,
	.filter_contrast			= render_default_filter_contrast,
	.filter_blur				= render_default_filter_blur,

	.shape_save					= render_cairo_shape_save,
	.shape_restore				= render_cairo_shape_restore,
	.shape_push_group			= render_cairo_shape_push_group,
	.shape_pop_group			= render_cairo_shape_pop_group,
	.shape_pop_group_to_source	= render_cairo_shape_pop_group_to_source,
	.shape_new_path				= render_cairo_shape_new_path,
	.shape_new_sub_path			= render_cairo_shape_new_sub_path,
	.shape_close_path			= render_cairo_shape_close_path,
	.shape_set_operator			= render_cairo_shape_set_operator,
	.shape_set_source			= render_cairo_shape_set_source,
	.shape_get_source			= render_cairo_shape_get_source,
	.shape_set_source_color		= render_cairo_shape_set_source_color,
	.shape_set_source_surface	= render_cairo_shape_set_source_surface,
	.shape_set_tolerance		= render_cairo_shape_set_tolerance,
	.shape_set_miter_limit		= render_cairo_shape_set_miter_limit,
	.shape_set_antialias		= render_cairo_shape_set_antialias,
	.shape_set_fill_rule		= render_cairo_shape_set_fill_rule,
	.shape_set_line_width		= render_cairo_shape_set_line_width,
	.shape_set_line_cap			= render_cairo_shape_set_line_cap,
	.shape_set_line_join		= render_cairo_shape_set_line_join,
	.shape_set_dash				= render_cairo_shape_set_dash,
	.shape_identity				= render_cairo_shape_identity,
	.shape_translate			= render_cairo_shape_translate,
	.shape_scale				= render_cairo_shape_scale,
	.shape_rotate				= render_cairo_shape_rotate,
	.shape_transform			= render_cairo_shape_transform,
	.shape_set_matrix			= render_cairo_shape_set_matrix,
	.shape_get_matrix			= render_cairo_shape_get_matrix,
	.shape_move_to				= render_cairo_shape_move_to,
	.shape_rel_move_to			= render_cairo_shape_rel_move_to,
	.shape_line_to				= render_cairo_shape_line_to,
	.shape_rel_line_to			= render_cairo_shape_rel_line_to,
	.shape_curve_to				= render_cairo_shape_curve_to,
	.shape_rel_curve_to			= render_cairo_shape_rel_curve_to,
	.shape_rectangle			= render_cairo_shape_rectangle,
	.shape_rounded_rectangle	= render_cairo_shape_rounded_rectangle,
	.shape_arc					= render_cairo_shape_arc,
	.shape_arc_negative			= render_cairo_shape_arc_negative,
	.shape_stroke				= render_cairo_shape_stroke,
	.shape_stroke_preserve		= render_cairo_shape_stroke_preserve,
	.shape_fill					= render_cairo_shape_fill,
	.shape_fill_preserve		= render_cairo_shape_fill_preserve,
	.shape_reset_clip			= render_cairo_shape_reset_clip,
	.shape_clip					= render_cairo_shape_clip,
	.shape_clip_preserve		= render_cairo_shape_clip_preserve,
	.shape_mask					= render_cairo_shape_mask,
	.shape_mask_surface			= render_cairo_shape_mask_surface,
	.shape_paint				= render_cairo_shape_paint,

	.font_create				= render_cairo_font_create,
	.font_destroy				= render_cairo_font_destroy,

	.pattern_create				= render_cairo_pattern_create,
	.pattern_create_color		= render_cairo_pattern_create_color,
	.pattern_create_linear		= render_cairo_pattern_create_linear,
	.pattern_create_radial		= render_cairo_pattern_create_radial,
	.pattern_destroy			= render_cairo_pattern_destroy,
	.pattern_add_color_stop		= render_cairo_pattern_add_color_stop,
	.pattern_set_extend			= render_cairo_pattern_set_extend,
	.pattern_set_filter			= render_cairo_pattern_set_filter,
	.pattern_set_matrix			= render_cairo_pattern_set_matrix,
};

static __init void render_cairo_init(void)
{
	register_render(&render_cairo);
}

static __exit void render_cairo_exit(void)
{
	unregister_render(&render_cairo);
}

core_initcall(render_cairo_init);
core_exitcall(render_cairo_exit);