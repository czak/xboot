/*
 * realview-gpio.c
 *
 * Copyright(c) 2007-2016 Jianjun Jiang <8192542@qq.com>
 * Official site: http://xboot.org
 * Mobile phone: +86-18665388956
 * QQ: 8192542
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <xboot.h>
#include <realview-gpio.h>
#include <realview/reg-gpio.h>

struct realview_gpiochip_data_t
{
	const char * name;
	int base;
	int ngpio;
	physical_addr_t regbase;
};

struct realview_gpiochip_pdata_t
{
	const char * name;
	int base;
	int ngpio;
	virtual_addr_t regbase;
};

static struct realview_gpiochip_data_t gpiochip_datas[] = {
	{
		.name		= "GPIO0",
		.base		= REALVIEW_GPIO0(0),
		.ngpio		= 8,
		.regbase	= REALVIEW_GPIO0_BASE,
	}, {
		.name		= "GPIO1",
		.base		= REALVIEW_GPIO1(0),
		.ngpio		= 8,
		.regbase	= REALVIEW_GPIO1_BASE,
	}, {
		.name		= "GPIO2",
		.base		= REALVIEW_GPIO2(0),
		.ngpio		= 8,
		.regbase	= REALVIEW_GPIO2_BASE,
	},
};

static void realview_gpiochip_set_cfg(struct gpiochip_t * chip, int offset, int cfg)
{
}

static int realview_gpiochip_get_cfg(struct gpiochip_t * chip, int offset)
{
	return 0;
}

static void realview_gpiochip_set_pull(struct gpiochip_t * chip, int offset, enum gpio_pull_t pull)
{
}

static enum gpio_pull_t realview_gpiochip_get_pull(struct gpiochip_t * chip, int offset)
{
	return GPIO_PULL_NONE;
}

static void realview_gpiochip_set_drv(struct gpiochip_t * chip, int offset, enum gpio_drv_t drv)
{
}

static enum gpio_drv_t realview_gpiochip_get_drv(struct gpiochip_t * chip, int offset)
{
	return GPIO_DRV_LOW;
}

static void realview_gpiochip_set_rate(struct gpiochip_t * chip, int offset, enum gpio_rate_t rate)
{
}

static enum gpio_rate_t realview_gpiochip_get_rate(struct gpiochip_t * chip, int offset)
{
	return GPIO_RATE_SLOW;
}

static void realview_gpiochip_set_dir(struct gpiochip_t * chip, int offset, enum gpio_direction_t dir)
{
	struct realview_gpiochip_pdata_t * pdat = (struct realview_gpiochip_pdata_t *)chip->priv;
	u8_t val;

	if(offset >= chip->ngpio)
		return;

	switch(dir)
	{
	case GPIO_DIRECTION_INPUT:
		val = read8(pdat->regbase + GPIO_DIR);
		val &= ~(1 << offset);
		write8(pdat->regbase + GPIO_DIR, val);
		break;

	case GPIO_DIRECTION_OUTPUT:
		val = read8(pdat->regbase + GPIO_DIR);
		val |= 1 << offset;
		write8(pdat->regbase + GPIO_DIR, val);
		break;

	default:
		break;
	}
}

static enum gpio_direction_t realview_gpiochip_get_dir(struct gpiochip_t * chip, int offset)
{
	struct realview_gpiochip_pdata_t * pdat = (struct realview_gpiochip_pdata_t *)chip->priv;
	u8_t val;

	if(offset >= chip->ngpio)
		return GPIO_DIRECTION_UNKOWN;

	val = read8(pdat->regbase + GPIO_DIR);
	if((val & (1 << offset)) == 0)
		return GPIO_DIRECTION_INPUT;
	return GPIO_DIRECTION_OUTPUT;
}

static void realview_gpiochip_set_value(struct gpiochip_t * chip, int offset, int value)
{
	struct realview_gpiochip_pdata_t * pdat = (struct realview_gpiochip_pdata_t *)chip->priv;
	write8(pdat->regbase + (1 << (offset + 2)), !!value << offset);
}

static int realview_gpiochip_get_value(struct gpiochip_t * chip, int offset)
{
	struct realview_gpiochip_pdata_t * pdat = (struct realview_gpiochip_pdata_t *)chip->priv;
	return !!read8(pdat->regbase + (1 << (offset + 2)));
}

static const char * realview_gpiochip_to_irq(struct gpiochip_t * chip, int offset)
{
	return 0;
}

static __init void realview_gpiochip_init(void)
{
	struct realview_gpiochip_pdata_t * pdat;
	struct gpiochip_t * chip;
	int i;

	for(i = 0; i < ARRAY_SIZE(gpiochip_datas); i++)
	{
		pdat = malloc(sizeof(struct realview_gpiochip_pdata_t));
		if(!pdat)
			continue;

		chip = malloc(sizeof(struct gpiochip_t));
		if(!chip)
		{
			free(pdat);
			continue;
		}

		pdat->name = gpiochip_datas[i].name;
		pdat->base = gpiochip_datas[i].base;
		pdat->ngpio = gpiochip_datas[i].ngpio;
		pdat->regbase = phys_to_virt(gpiochip_datas[i].regbase);
		chip->name = pdat->name;
		chip->base = pdat->base;
		chip->ngpio = pdat->ngpio;
		chip->set_cfg = realview_gpiochip_set_cfg;
		chip->get_cfg = realview_gpiochip_get_cfg;
		chip->set_pull = realview_gpiochip_set_pull;
		chip->get_pull = realview_gpiochip_get_pull;
		chip->set_drv = realview_gpiochip_set_drv;
		chip->get_drv = realview_gpiochip_get_drv;
		chip->set_rate = realview_gpiochip_set_rate;
		chip->get_rate = realview_gpiochip_get_rate;
		chip->set_dir = realview_gpiochip_set_dir;
		chip->get_dir = realview_gpiochip_get_dir;
		chip->set_value = realview_gpiochip_set_value;
		chip->get_value = realview_gpiochip_get_value;
		chip->to_irq = realview_gpiochip_to_irq;
		chip->priv = pdat;

		register_gpiochip(chip);
	}
}
core_initcall(realview_gpiochip_init);
