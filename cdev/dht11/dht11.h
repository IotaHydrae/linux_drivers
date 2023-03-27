/*
	dht11.c - a header file for dht11 driver.

	Copyright (C) 2021 Zheng Hua <writeforever@foxmail.com>

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License along
	with this program; if not, write to the Free Software Foundation, Inc.,
	51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

/* ioctl defines */
#define DHT11_IOC_MAGIC			'd'
#define DHT11_IOCINIT			_IOW(DHT11_IOC_MAGIC, 0, int)
#define DHT11_IOCREADBYTE		_IOWR(DHT11_IOC_MAGIC, 1, unsigned char)
