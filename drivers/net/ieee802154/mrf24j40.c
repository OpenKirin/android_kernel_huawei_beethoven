/*
 * Driver for Microchip MRF24J40 802.15.4 Wireless-PAN Networking controller
 *
 * Copyright (C) 2012 Alan Ott <alan@signal11.us>
 *                    Signal 11 Software
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
 */

#include <linux/spi/spi.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/regmap.h>
#include <linux/ieee802154.h>
#include <net/cfg802154.h>
#include <net/mac802154.h>

/* MRF24J40 Short Address Registers */
#define REG_RXMCR	0x00  /* Receive MAC control */
#define REG_PANIDL	0x01  /* PAN ID (low) */
#define REG_PANIDH	0x02  /* PAN ID (high) */
#define REG_SADRL	0x03  /* Short address (low) */
#define REG_SADRH	0x04  /* Short address (high) */
#define REG_EADR0	0x05  /* Long address (low) (high is EADR7) */
#define REG_EADR1	0x06
#define REG_EADR2	0x07
#define REG_EADR3	0x08
#define REG_EADR4	0x09
#define REG_EADR5	0x0A
#define REG_EADR6	0x0B
#define REG_EADR7	0x0C
#define REG_RXFLUSH	0x0D
#define REG_ORDER	0x10
#define REG_TXMCR	0x11  /* Transmit MAC control */
#define REG_ACKTMOUT	0x12
#define REG_ESLOTG1	0x13
#define REG_SYMTICKL	0x14
#define REG_SYMTICKH	0x15
#define REG_PACON0	0x16  /* Power Amplifier Control */
#define REG_PACON1	0x17  /* Power Amplifier Control */
#define REG_PACON2	0x18  /* Power Amplifier Control */
#define REG_TXBCON0	0x1A
#define REG_TXNCON	0x1B  /* Transmit Normal FIFO Control */
#define REG_TXG1CON	0x1C
#define REG_TXG2CON	0x1D
#define REG_ESLOTG23	0x1E
#define REG_ESLOTG45	0x1F
#define REG_ESLOTG67	0x20
#define REG_TXPEND	0x21
#define REG_WAKECON	0x22
#define REG_FROMOFFSET	0x23
#define REG_TXSTAT	0x24  /* TX MAC Status Register */
#define REG_TXBCON1	0x25
#define REG_GATECLK	0x26
#define REG_TXTIME	0x27
#define REG_HSYMTMRL	0x28
#define REG_HSYMTMRH	0x29
#define REG_SOFTRST	0x2A  /* Soft Reset */
#define REG_SECCON0	0x2C
#define REG_SECCON1	0x2D
#define REG_TXSTBL	0x2E  /* TX Stabilization */
#define REG_RXSR	0x30
#define REG_INTSTAT	0x31  /* Interrupt Status */
#define REG_INTCON	0x32  /* Interrupt Control */
#define REG_GPIO	0x33  /* GPIO */
#define REG_TRISGPIO	0x34  /* GPIO direction */
#define REG_SLPACK	0x35
#define REG_RFCTL	0x36  /* RF Control Mode Register */
#define REG_SECCR2	0x37
#define REG_BBREG0	0x38
#define REG_BBREG1	0x39  /* Baseband Registers */
#define REG_BBREG2	0x3A  /* */
#define REG_BBREG3	0x3B
#define REG_BBREG4	0x3C
#define REG_BBREG6	0x3E  /* */
#define REG_CCAEDTH	0x3F  /* Energy Detection Threshold */

/* MRF24J40 Long Address Registers */
#define REG_RFCON0	0x200  /* RF Control Registers */
#define REG_RFCON1	0x201
#define REG_RFCON2	0x202
#define REG_RFCON3	0x203
#define REG_RFCON5	0x205
#define REG_RFCON6	0x206
#define REG_RFCON7	0x207
#define REG_RFCON8	0x208
#define REG_SLPCAL0	0x209
#define REG_SLPCAL1	0x20A
#define REG_SLPCAL2	0x20B
#define REG_RFSTATE	0x20F
#define REG_RSSI	0x210
#define REG_SLPCON0	0x211  /* Sleep Clock Control Registers */
#define REG_SLPCON1	0x220
#define REG_WAKETIMEL	0x222  /* Wake-up Time Match Value Low */
#define REG_WAKETIMEH	0x223  /* Wake-up Time Match Value High */
#define REG_REMCNTL	0x224
#define REG_REMCNTH	0x225
#define REG_MAINCNT0	0x226
#define REG_MAINCNT1	0x227
#define REG_MAINCNT2	0x228
#define REG_MAINCNT3	0x229
#define REG_TESTMODE	0x22F  /* Test mode */
#define REG_ASSOEAR0	0x230
#define REG_ASSOEAR1	0x231
#define REG_ASSOEAR2	0x232
#define REG_ASSOEAR3	0x233
#define REG_ASSOEAR4	0x234
#define REG_ASSOEAR5	0x235
#define REG_ASSOEAR6	0x236
#define REG_ASSOEAR7	0x237
#define REG_ASSOSAR0	0x238
#define REG_ASSOSAR1	0x239
#define REG_UNONCE0	0x240
#define REG_UNONCE1	0x241
#define REG_UNONCE2	0x242
#define REG_UNONCE3	0x243
#define REG_UNONCE4	0x244
#define REG_UNONCE5	0x245
#define REG_UNONCE6	0x246
#define REG_UNONCE7	0x247
#define REG_UNONCE8	0x248
#define REG_UNONCE9	0x249
#define REG_UNONCE10	0x24A
#define REG_UNONCE11	0x24B
#define REG_UNONCE12	0x24C
#define REG_RX_FIFO	0x300  /* Receive FIFO */

/* Device configuration: Only channels 11-26 on page 0 are supported. */
#define MRF24J40_CHAN_MIN 11
#define MRF24J40_CHAN_MAX 26
#define CHANNEL_MASK (((u32)1 << (MRF24J40_CHAN_MAX + 1)) \
		      - ((u32)1 << MRF24J40_CHAN_MIN))

#define TX_FIFO_SIZE 128 /* From datasheet */
#define RX_FIFO_SIZE 144 /* From datasheet */
#define SET_CHANNEL_DELAY_US 192 /* From datasheet */

enum mrf24j40_modules { MRF24J40, MRF24J40MA, MRF24J40MC };

/* Device Private Data */
struct mrf24j40 {
	struct spi_device *spi;
	struct ieee802154_hw *hw;

	struct regmap *regmap_short;
	struct regmap *regmap_long;
	struct mutex buffer_mutex; /* only used to protect buf */
	struct completion tx_complete;
	u8 *buf; /* 3 bytes. Used for SPI single-register transfers. */
};

/* regmap information for short address register access */
#define MRF24J40_SHORT_WRITE	0x01
#define MRF24J40_SHORT_READ	0x00
#define MRF24J40_SHORT_NUMREGS	0x3F

/* regmap information for long address register access */
#define MRF24J40_LONG_ACCESS	0x80
#define MRF24J40_LONG_NUMREGS	0x38F

/* Read/Write SPI Commands for Short and Long Address registers. */
#define MRF24J40_READSHORT(reg) ((reg) << 1)
#define MRF24J40_WRITESHORT(reg) ((reg) << 1 | 1)
#define MRF24J40_READLONG(reg) (1 << 15 | (reg) << 5)
#define MRF24J40_WRITELONG(reg) (1 << 15 | (reg) << 5 | 1 << 4)

/* The datasheet indicates the theoretical maximum for SCK to be 10MHz */
#define MAX_SPI_SPEED_HZ 10000000

#define printdev(X) (&X->spi->dev)

static bool
mrf24j40_short_reg_writeable(struct device *dev, unsigned int reg)
{
	switch (reg) {
	case REG_RXMCR:
	case REG_PANIDL:
	case REG_PANIDH:
	case REG_SADRL:
	case REG_SADRH:
	case REG_EADR0:
	case REG_EADR1:
	case REG_EADR2:
	case REG_EADR3:
	case REG_EADR4:
	case REG_EADR5:
	case REG_EADR6:
	case REG_EADR7:
	case REG_RXFLUSH:
	case REG_ORDER:
	case REG_TXMCR:
	case REG_ACKTMOUT:
	case REG_ESLOTG1:
	case REG_SYMTICKL:
	case REG_SYMTICKH:
	case REG_PACON0:
	case REG_PACON1:
	case REG_PACON2:
	case REG_TXBCON0:
	case REG_TXNCON:
	case REG_TXG1CON:
	case REG_TXG2CON:
	case REG_ESLOTG23:
	case REG_ESLOTG45:
	case REG_ESLOTG67:
	case REG_TXPEND:
	case REG_WAKECON:
	case REG_FROMOFFSET:
	case REG_TXBCON1:
	case REG_GATECLK:
	case REG_TXTIME:
	case REG_HSYMTMRL:
	case REG_HSYMTMRH:
	case REG_SOFTRST:
	case REG_SECCON0:
	case REG_SECCON1:
	case REG_TXSTBL:
	case REG_RXSR:
	case REG_INTCON:
	case REG_TRISGPIO:
	case REG_GPIO:
	case REG_RFCTL:
	case REG_SLPACK:
	case REG_BBREG0:
	case REG_BBREG1:
	case REG_BBREG2:
	case REG_BBREG3:
	case REG_BBREG4:
	case REG_BBREG6:
	case REG_CCAEDTH:
		return true;
	default:
		return false;
	}
}

static bool
mrf24j40_short_reg_readable(struct device *dev, unsigned int reg)
{
	bool rc;

	/* all writeable are also readable */
	rc = mrf24j40_short_reg_writeable(dev, reg);
	if (rc)
		return rc;

	/* readonly regs */
	switch (reg) {
	case REG_TXSTAT:
	case REG_INTSTAT:
		return true;
	default:
		return false;
	}
}

static bool
mrf24j40_short_reg_volatile(struct device *dev, unsigned int reg)
{
	/* can be changed during runtime */
	switch (reg) {
	case REG_TXSTAT:
	case REG_INTSTAT:
	case REG_RXFLUSH:
	case REG_TXNCON:
	case REG_SOFTRST:
	case REG_RFCTL:
	case REG_TXBCON0:
	case REG_TXG1CON:
	case REG_TXG2CON:
	case REG_TXBCON1:
	case REG_SECCON0:
	case REG_RXSR:
	case REG_SLPACK:
	case REG_SECCR2:
	case REG_BBREG6:
	/* use them in spi_async and regmap so it's volatile */
	case REG_BBREG1:
		return true;
	default:
		return false;
	}
}

static bool
mrf24j40_short_reg_precious(struct device *dev, unsigned int reg)
{
	/* don't clear irq line on read */
	switch (reg) {
	case REG_INTSTAT:
		return true;
	default:
		return false;
	}
}

static const struct regmap_config mrf24j40_short_regmap = {
	.name = "mrf24j40_short",
	.reg_bits = 7,
	.val_bits = 8,
	.pad_bits = 1,
	.write_flag_mask = MRF24J40_SHORT_WRITE,
	.read_flag_mask = MRF24J40_SHORT_READ,
	.cache_type = REGCACHE_RBTREE,
	.max_register = MRF24J40_SHORT_NUMREGS,
	.writeable_reg = mrf24j40_short_reg_writeable,
	.readable_reg = mrf24j40_short_reg_readable,
	.volatile_reg = mrf24j40_short_reg_volatile,
	.precious_reg = mrf24j40_short_reg_precious,
};

static bool
mrf24j40_long_reg_writeable(struct device *dev, unsigned int reg)
{
	switch (reg) {
	case REG_RFCON0:
	case REG_RFCON1:
	case REG_RFCON2:
	case REG_RFCON3:
	case REG_RFCON5:
	case REG_RFCON6:
	case REG_RFCON7:
	case REG_RFCON8:
	case REG_SLPCAL2:
	case REG_SLPCON0:
	case REG_SLPCON1:
	case REG_WAKETIMEL:
	case REG_WAKETIMEH:
	case REG_REMCNTL:
	case REG_REMCNTH:
	case REG_MAINCNT0:
	case REG_MAINCNT1:
	case REG_MAINCNT2:
	case REG_MAINCNT3:
	case REG_TESTMODE:
	case REG_ASSOEAR0:
	case REG_ASSOEAR1:
	case REG_ASSOEAR2:
	case REG_ASSOEAR3:
	case REG_ASSOEAR4:
	case REG_ASSOEAR5:
	case REG_ASSOEAR6:
	case REG_ASSOEAR7:
	case REG_ASSOSAR0:
	case REG_ASSOSAR1:
	case REG_UNONCE0:
	case REG_UNONCE1:
	case REG_UNONCE2:
	case REG_UNONCE3:
	case REG_UNONCE4:
	case REG_UNONCE5:
	case REG_UNONCE6:
	case REG_UNONCE7:
	case REG_UNONCE8:
	case REG_UNONCE9:
	case REG_UNONCE10:
	case REG_UNONCE11:
	case REG_UNONCE12:
		return true;
	default:
		return false;
	}
}

static bool
mrf24j40_long_reg_readable(struct device *dev, unsigned int reg)
{
	bool rc;

	/* all writeable are also readable */
	rc = mrf24j40_long_reg_writeable(dev, reg);
	if (rc)
		return rc;

	/* readonly regs */
	switch (reg) {
	case REG_SLPCAL0:
	case REG_SLPCAL1:
	case REG_RFSTATE:
	case REG_RSSI:
		return true;
	default:
		return false;
	}
}

static bool
mrf24j40_long_reg_volatile(struct device *dev, unsigned int reg)
{
	/* can be changed during runtime */
	switch (reg) {
	case REG_SLPCAL0:
	case REG_SLPCAL1:
	case REG_SLPCAL2:
	case REG_RFSTATE:
	case REG_RSSI:
	case REG_MAINCNT3:
		return true;
	default:
		return false;
	}
}

static const struct regmap_config mrf24j40_long_regmap = {
	.name = "mrf24j40_long",
	.reg_bits = 11,
	.val_bits = 8,
	.pad_bits = 5,
	.write_flag_mask = MRF24J40_LONG_ACCESS,
	.read_flag_mask = MRF24J40_LONG_ACCESS,
	.cache_type = REGCACHE_RBTREE,
	.max_register = MRF24J40_LONG_NUMREGS,
	.writeable_reg = mrf24j40_long_reg_writeable,
	.readable_reg = mrf24j40_long_reg_readable,
	.volatile_reg = mrf24j40_long_reg_volatile,
};

static int mrf24j40_long_regmap_write(void *context, const void *data,
				      size_t count)
{
	struct spi_device *spi = context;
	u8 buf[3];

	if (count > 3)
		return -EINVAL;

	/* regmap supports read/write mask only in frist byte
	 * long write access need to set the 12th bit, so we
	 * make special handling for write.
	 */
	memcpy(buf, data, count);
	buf[1] |= (1 << 4);

	return spi_write(spi, buf, count);
}

static int
mrf24j40_long_regmap_read(void *context, const void *reg, size_t reg_size,
			  void *val, size_t val_size)
{
	struct spi_device *spi = context;

	return spi_write_then_read(spi, reg, reg_size, val, val_size);
}

static const struct regmap_bus mrf24j40_long_regmap_bus = {
	.write = mrf24j40_long_regmap_write,
	.read = mrf24j40_long_regmap_read,
	.reg_format_endian_default = REGMAP_ENDIAN_BIG,
	.val_format_endian_default = REGMAP_ENDIAN_BIG,
};

static int write_short_reg(struct mrf24j40 *devrec, u8 reg, u8 value)
{
	int ret;
	struct spi_message msg;
	struct spi_transfer xfer = {
		.len = 2,
		.tx_buf = devrec->buf,
		.rx_buf = devrec->buf,
	};

	spi_message_init(&msg);
	spi_message_add_tail(&xfer, &msg);

	mutex_lock(&devrec->buffer_mutex);
	devrec->buf[0] = MRF24J40_WRITESHORT(reg);
	devrec->buf[1] = value;

	ret = spi_sync(devrec->spi, &msg);
	if (ret)
		dev_err(printdev(devrec),
			"SPI write Failed for short register 0x%hhx\n", reg);

	mutex_unlock(&devrec->buffer_mutex);
	return ret;
}

static int read_short_reg(struct mrf24j40 *devrec, u8 reg, u8 *val)
{
	int ret = -1;
	struct spi_message msg;
	struct spi_transfer xfer = {
		.len = 2,
		.tx_buf = devrec->buf,
		.rx_buf = devrec->buf,
	};

	spi_message_init(&msg);
	spi_message_add_tail(&xfer, &msg);

	mutex_lock(&devrec->buffer_mutex);
	devrec->buf[0] = MRF24J40_READSHORT(reg);
	devrec->buf[1] = 0;

	ret = spi_sync(devrec->spi, &msg);
	if (ret)
		dev_err(printdev(devrec),
			"SPI read Failed for short register 0x%hhx\n", reg);
	else
		*val = devrec->buf[1];

	mutex_unlock(&devrec->buffer_mutex);
	return ret;
}

static int read_long_reg(struct mrf24j40 *devrec, u16 reg, u8 *value)
{
	int ret;
	u16 cmd;
	struct spi_message msg;
	struct spi_transfer xfer = {
		.len = 3,
		.tx_buf = devrec->buf,
		.rx_buf = devrec->buf,
	};

	spi_message_init(&msg);
	spi_message_add_tail(&xfer, &msg);

	cmd = MRF24J40_READLONG(reg);
	mutex_lock(&devrec->buffer_mutex);
	devrec->buf[0] = cmd >> 8 & 0xff;
	devrec->buf[1] = cmd & 0xff;
	devrec->buf[2] = 0;

	ret = spi_sync(devrec->spi, &msg);
	if (ret)
		dev_err(printdev(devrec),
			"SPI read Failed for long register 0x%hx\n", reg);
	else
		*value = devrec->buf[2];

	mutex_unlock(&devrec->buffer_mutex);
	return ret;
}

/* This function relies on an undocumented write method. Once a write command
   and address is set, as many bytes of data as desired can be clocked into
   the device. The datasheet only shows setting one byte at a time. */
static int write_tx_buf(struct mrf24j40 *devrec, u16 reg,
			const u8 *data, size_t length)
{
	int ret;
	u16 cmd;
	u8 lengths[2];
	struct spi_message msg;
	struct spi_transfer addr_xfer = {
		.len = 2,
		.tx_buf = devrec->buf,
	};
	struct spi_transfer lengths_xfer = {
		.len = 2,
		.tx_buf = &lengths, /* TODO: Is DMA really required for SPI? */
	};
	struct spi_transfer data_xfer = {
		.len = length,
		.tx_buf = data,
	};

	/* Range check the length. 2 bytes are used for the length fields.*/
	if (length > TX_FIFO_SIZE-2) {
		dev_err(printdev(devrec), "write_tx_buf() was passed too large a buffer. Performing short write.\n");
		length = TX_FIFO_SIZE-2;
	}

	spi_message_init(&msg);
	spi_message_add_tail(&addr_xfer, &msg);
	spi_message_add_tail(&lengths_xfer, &msg);
	spi_message_add_tail(&data_xfer, &msg);

	cmd = MRF24J40_WRITELONG(reg);
	mutex_lock(&devrec->buffer_mutex);
	devrec->buf[0] = cmd >> 8 & 0xff;
	devrec->buf[1] = cmd & 0xff;
	lengths[0] = 0x0; /* Header Length. Set to 0 for now. TODO */
	lengths[1] = length; /* Total length */

	ret = spi_sync(devrec->spi, &msg);
	if (ret)
		dev_err(printdev(devrec), "SPI write Failed for TX buf\n");

	mutex_unlock(&devrec->buffer_mutex);
	return ret;
}

static int mrf24j40_read_rx_buf(struct mrf24j40 *devrec,
				u8 *data, u8 *len, u8 *lqi)
{
	u8 rx_len;
	u8 addr[2];
	u8 lqi_rssi[2];
	u16 cmd;
	int ret;
	struct spi_message msg;
	struct spi_transfer addr_xfer = {
		.len = 2,
		.tx_buf = &addr,
	};
	struct spi_transfer data_xfer = {
		.len = 0x0, /* set below */
		.rx_buf = data,
	};
	struct spi_transfer status_xfer = {
		.len = 2,
		.rx_buf = &lqi_rssi,
	};

	/* Get the length of the data in the RX FIFO. The length in this
	 * register exclues the 1-byte length field at the beginning. */
	ret = read_long_reg(devrec, REG_RX_FIFO, &rx_len);
	if (ret)
		goto out;

	/* Range check the RX FIFO length, accounting for the one-byte
	 * length field at the beginning. */
	if (rx_len > RX_FIFO_SIZE-1) {
		dev_err(printdev(devrec), "Invalid length read from device. Performing short read.\n");
		rx_len = RX_FIFO_SIZE-1;
	}

	if (rx_len > *len) {
		/* Passed in buffer wasn't big enough. Should never happen. */
		dev_err(printdev(devrec), "Buffer not big enough. Performing short read\n");
		rx_len = *len;
	}

	/* Set up the commands to read the data. */
	cmd = MRF24J40_READLONG(REG_RX_FIFO+1);
	addr[0] = cmd >> 8 & 0xff;
	addr[1] = cmd & 0xff;
	data_xfer.len = rx_len;

	spi_message_init(&msg);
	spi_message_add_tail(&addr_xfer, &msg);
	spi_message_add_tail(&data_xfer, &msg);
	spi_message_add_tail(&status_xfer, &msg);

	ret = spi_sync(devrec->spi, &msg);
	if (ret) {
		dev_err(printdev(devrec), "SPI RX Buffer Read Failed.\n");
		goto out;
	}

	*lqi = lqi_rssi[0];
	*len = rx_len;

#ifdef DEBUG
	print_hex_dump(KERN_DEBUG, "mrf24j40 rx: ",
		       DUMP_PREFIX_OFFSET, 16, 1, data, *len, 0);
	pr_debug("mrf24j40 rx: lqi: %02hhx rssi: %02hhx\n",
		 lqi_rssi[0], lqi_rssi[1]);
#endif

out:
	return ret;
}

static int mrf24j40_tx(struct ieee802154_hw *hw, struct sk_buff *skb)
{
	struct mrf24j40 *devrec = hw->priv;
	u8 val;
	int ret = 0;

	dev_dbg(printdev(devrec), "tx packet of %d bytes\n", skb->len);

	ret = write_tx_buf(devrec, 0x000, skb->data, skb->len);
	if (ret)
		goto err;

	reinit_completion(&devrec->tx_complete);

	/* Set TXNTRIG bit of TXNCON to send packet */
	ret = read_short_reg(devrec, REG_TXNCON, &val);
	if (ret)
		goto err;
	val |= 0x1;
	/* Set TXNACKREQ if the ACK bit is set in the packet. */
	if (skb->data[0] & IEEE802154_FC_ACK_REQ)
		val |= 0x4;
	write_short_reg(devrec, REG_TXNCON, val);

	/* Wait for the device to send the TX complete interrupt. */
	ret = wait_for_completion_interruptible_timeout(
						&devrec->tx_complete,
						5 * HZ);
	if (ret == -ERESTARTSYS)
		goto err;
	if (ret == 0) {
		dev_warn(printdev(devrec), "Timeout waiting for TX interrupt\n");
		ret = -ETIMEDOUT;
		goto err;
	}

	/* Check for send error from the device. */
	ret = read_short_reg(devrec, REG_TXSTAT, &val);
	if (ret)
		goto err;
	if (val & 0x1) {
		dev_dbg(printdev(devrec), "Error Sending. Retry count exceeded\n");
		ret = -ECOMM; /* TODO: Better error code ? */
	} else
		dev_dbg(printdev(devrec), "Packet Sent\n");

err:

	return ret;
}

static int mrf24j40_ed(struct ieee802154_hw *hw, u8 *level)
{
	/* TODO: */
	pr_warn("mrf24j40: ed not implemented\n");
	*level = 0;
	return 0;
}

static int mrf24j40_start(struct ieee802154_hw *hw)
{
	struct mrf24j40 *devrec = hw->priv;

	dev_dbg(printdev(devrec), "start\n");

	/* Clear TXNIE and RXIE. Enable interrupts */
	return regmap_update_bits(devrec->regmap_short, REG_INTCON,
				  0x01 | 0x08, 0x00);
}

static void mrf24j40_stop(struct ieee802154_hw *hw)
{
	struct mrf24j40 *devrec = hw->priv;

	dev_dbg(printdev(devrec), "stop\n");

	/* Set TXNIE and RXIE. Disable Interrupts */
	regmap_update_bits(devrec->regmap_short, REG_INTCON, 0x01 | 0x08,
			   0x01 | 0x08);
}

static int mrf24j40_set_channel(struct ieee802154_hw *hw, u8 page, u8 channel)
{
	struct mrf24j40 *devrec = hw->priv;
	u8 val;
	int ret;

	dev_dbg(printdev(devrec), "Set Channel %d\n", channel);

	WARN_ON(page != 0);
	WARN_ON(channel < MRF24J40_CHAN_MIN);
	WARN_ON(channel > MRF24J40_CHAN_MAX);

	/* Set Channel TODO */
	val = (channel-11) << 4 | 0x03;
	ret = regmap_update_bits(devrec->regmap_long, REG_RFCON0, 0xf0, val);
	if (ret)
		return ret;

	/* RF Reset */
	ret = regmap_update_bits(devrec->regmap_short, REG_RFCTL, 0x04, 0x04);
	if (ret)
		return ret;

	ret = regmap_update_bits(devrec->regmap_short, REG_RFCTL, 0x04, 0x00);
	if (!ret)
		udelay(SET_CHANNEL_DELAY_US); /* per datasheet */

	return ret;
}

static int mrf24j40_filter(struct ieee802154_hw *hw,
			   struct ieee802154_hw_addr_filt *filt,
			   unsigned long changed)
{
	struct mrf24j40 *devrec = hw->priv;

	dev_dbg(printdev(devrec), "filter\n");

	if (changed & IEEE802154_AFILT_SADDR_CHANGED) {
		/* Short Addr */
		u8 addrh, addrl;

		addrh = le16_to_cpu(filt->short_addr) >> 8 & 0xff;
		addrl = le16_to_cpu(filt->short_addr) & 0xff;

		regmap_write(devrec->regmap_short, REG_SADRH, addrh);
		regmap_write(devrec->regmap_short, REG_SADRL, addrl);
		dev_dbg(printdev(devrec),
			"Set short addr to %04hx\n", filt->short_addr);
	}

	if (changed & IEEE802154_AFILT_IEEEADDR_CHANGED) {
		/* Device Address */
		u8 i, addr[8];

		memcpy(addr, &filt->ieee_addr, 8);
		for (i = 0; i < 8; i++)
			regmap_write(devrec->regmap_short, REG_EADR0 + i,
				     addr[i]);

#ifdef DEBUG
		pr_debug("Set long addr to: ");
		for (i = 0; i < 8; i++)
			pr_debug("%02hhx ", addr[7 - i]);
		pr_debug("\n");
#endif
	}

	if (changed & IEEE802154_AFILT_PANID_CHANGED) {
		/* PAN ID */
		u8 panidl, panidh;

		panidh = le16_to_cpu(filt->pan_id) >> 8 & 0xff;
		panidl = le16_to_cpu(filt->pan_id) & 0xff;
		regmap_write(devrec->regmap_short, REG_PANIDH, panidh);
		regmap_write(devrec->regmap_short, REG_PANIDL, panidl);

		dev_dbg(printdev(devrec), "Set PANID to %04hx\n", filt->pan_id);
	}

	if (changed & IEEE802154_AFILT_PANC_CHANGED) {
		/* Pan Coordinator */
		u8 val;
		int ret;

		if (filt->pan_coord)
			val = 0x8;
		else
			val = 0x0;
		ret = regmap_update_bits(devrec->regmap_short, REG_RXMCR, 0x8,
					 val);
		if (ret)
			return ret;

		/* REG_SLOTTED is maintained as default (unslotted/CSMA-CA).
		 * REG_ORDER is maintained as default (no beacon/superframe).
		 */

		dev_dbg(printdev(devrec), "Set Pan Coord to %s\n",
			filt->pan_coord ? "on" : "off");
	}

	return 0;
}

static int mrf24j40_handle_rx(struct mrf24j40 *devrec)
{
	u8 len = RX_FIFO_SIZE;
	u8 lqi = 0;
	u8 val;
	int ret = 0;
	int ret2;
	struct sk_buff *skb;

	/* Turn off reception of packets off the air. This prevents the
	 * device from overwriting the buffer while we're reading it. */
	ret = read_short_reg(devrec, REG_BBREG1, &val);
	if (ret)
		goto out;
	val |= 4; /* SET RXDECINV */
	write_short_reg(devrec, REG_BBREG1, val);

	skb = dev_alloc_skb(len);
	if (!skb) {
		ret = -ENOMEM;
		goto out;
	}

	ret = mrf24j40_read_rx_buf(devrec, skb_put(skb, len), &len, &lqi);
	if (ret < 0) {
		dev_err(printdev(devrec), "Failure reading RX FIFO\n");
		kfree_skb(skb);
		ret = -EINVAL;
		goto out;
	}

	/* TODO: Other drivers call ieee20154_rx_irqsafe() here (eg: cc2040,
	 * also from a workqueue).  I think irqsafe is not necessary here.
	 * Can someone confirm? */
	ieee802154_rx_irqsafe(devrec->hw, skb, lqi);

	dev_dbg(printdev(devrec), "RX Handled\n");

out:
	/* Turn back on reception of packets off the air. */
	ret2 = read_short_reg(devrec, REG_BBREG1, &val);
	if (ret2)
		return ret2;
	val &= ~0x4; /* Clear RXDECINV */
	write_short_reg(devrec, REG_BBREG1, val);

	return ret;
}

static const struct ieee802154_ops mrf24j40_ops = {
	.owner = THIS_MODULE,
	.xmit_sync = mrf24j40_tx,
	.ed = mrf24j40_ed,
	.start = mrf24j40_start,
	.stop = mrf24j40_stop,
	.set_channel = mrf24j40_set_channel,
	.set_hw_addr_filt = mrf24j40_filter,
};

static irqreturn_t mrf24j40_isr(int irq, void *data)
{
	struct mrf24j40 *devrec = data;
	u8 intstat;
	int ret;

	/* Read the interrupt status */
	ret = read_short_reg(devrec, REG_INTSTAT, &intstat);
	if (ret)
		goto out;

	/* Check for TX complete */
	if (intstat & 0x1)
		complete(&devrec->tx_complete);

	/* Check for Rx */
	if (intstat & 0x8)
		mrf24j40_handle_rx(devrec);

out:
	return IRQ_HANDLED;
}

static int mrf24j40_hw_init(struct mrf24j40 *devrec)
{
	int ret;

	/* Initialize the device.
		From datasheet section 3.2: Initialization. */
	ret = regmap_write(devrec->regmap_short, REG_SOFTRST, 0x07);
	if (ret)
		goto err_ret;

	ret = regmap_write(devrec->regmap_short, REG_PACON2, 0x98);
	if (ret)
		goto err_ret;

	ret = regmap_write(devrec->regmap_short, REG_TXSTBL, 0x95);
	if (ret)
		goto err_ret;

	ret = regmap_write(devrec->regmap_long, REG_RFCON0, 0x03);
	if (ret)
		goto err_ret;

	ret = regmap_write(devrec->regmap_long, REG_RFCON1, 0x01);
	if (ret)
		goto err_ret;

	ret = regmap_write(devrec->regmap_long, REG_RFCON2, 0x80);
	if (ret)
		goto err_ret;

	ret = regmap_write(devrec->regmap_long, REG_RFCON6, 0x90);
	if (ret)
		goto err_ret;

	ret = regmap_write(devrec->regmap_long, REG_RFCON7, 0x80);
	if (ret)
		goto err_ret;

	ret = regmap_write(devrec->regmap_long, REG_RFCON8, 0x10);
	if (ret)
		goto err_ret;

	ret = regmap_write(devrec->regmap_long, REG_SLPCON1, 0x21);
	if (ret)
		goto err_ret;

	ret = regmap_write(devrec->regmap_short, REG_BBREG2, 0x80);
	if (ret)
		goto err_ret;

	ret = regmap_write(devrec->regmap_short, REG_CCAEDTH, 0x60);
	if (ret)
		goto err_ret;

	ret = regmap_write(devrec->regmap_short, REG_BBREG6, 0x40);
	if (ret)
		goto err_ret;

	ret = regmap_write(devrec->regmap_short, REG_RFCTL, 0x04);
	if (ret)
		goto err_ret;

	ret = regmap_write(devrec->regmap_short, REG_RFCTL, 0x0);
	if (ret)
		goto err_ret;

	udelay(192);

	/* Set RX Mode. RXMCR<1:0>: 0x0 normal, 0x1 promisc, 0x2 error */
	ret = regmap_update_bits(devrec->regmap_short, REG_RXMCR, 0x03, 0x00);
	if (ret)
		goto err_ret;

	if (spi_get_device_id(devrec->spi)->driver_data == MRF24J40MC) {
		/* Enable external amplifier.
		 * From MRF24J40MC datasheet section 1.3: Operation.
		 */
		regmap_update_bits(devrec->regmap_long, REG_TESTMODE, 0x07,
				   0x07);

		/* Set GPIO3 as output. */
		regmap_update_bits(devrec->regmap_short, REG_TRISGPIO, 0x08,
				   0x08);

		/* Set GPIO3 HIGH to enable U5 voltage regulator */
		regmap_update_bits(devrec->regmap_short, REG_GPIO, 0x08, 0x08);

		/* Reduce TX pwr to meet FCC requirements.
		 * From MRF24J40MC datasheet section 3.1.1
		 */
		regmap_write(devrec->regmap_long, REG_RFCON3, 0x28);
	}

	return 0;

err_ret:
	return ret;
}

static void  mrf24j40_phy_setup(struct mrf24j40 *devrec)
{
	ieee802154_random_extended_addr(&devrec->hw->phy->perm_extended_addr);
	devrec->hw->phy->current_channel = 11;
}

static int mrf24j40_probe(struct spi_device *spi)
{
	int ret = -ENOMEM;
	struct ieee802154_hw *hw;
	struct mrf24j40 *devrec;

	dev_info(&spi->dev, "probe(). IRQ: %d\n", spi->irq);

	/* Register with the 802154 subsystem */

	hw = ieee802154_alloc_hw(sizeof(*devrec), &mrf24j40_ops);
	if (!hw)
		goto err_ret;

	devrec = hw->priv;
	devrec->spi = spi;
	spi_set_drvdata(spi, devrec);
	devrec->hw = hw;
	devrec->hw->parent = &spi->dev;
	devrec->hw->phy->supported.channels[0] = CHANNEL_MASK;
	devrec->hw->flags = IEEE802154_HW_TX_OMIT_CKSUM | IEEE802154_HW_AFILT;

	devrec->regmap_short = devm_regmap_init_spi(spi,
						    &mrf24j40_short_regmap);
	if (IS_ERR(devrec->regmap_short)) {
		ret = PTR_ERR(devrec->regmap_short);
		dev_err(&spi->dev, "Failed to allocate short register map: %d\n",
			ret);
		goto err_register_device;
	}

	devrec->regmap_long = devm_regmap_init(&spi->dev,
					       &mrf24j40_long_regmap_bus,
					       spi, &mrf24j40_long_regmap);
	if (IS_ERR(devrec->regmap_long)) {
		ret = PTR_ERR(devrec->regmap_long);
		dev_err(&spi->dev, "Failed to allocate long register map: %d\n",
			ret);
		goto err_register_device;
	}

	devrec->buf = devm_kzalloc(&spi->dev, 3, GFP_KERNEL);
	if (!devrec->buf)
		goto err_register_device;

	if (spi->max_speed_hz > MAX_SPI_SPEED_HZ) {
		dev_warn(&spi->dev, "spi clock above possible maximum: %d",
			 MAX_SPI_SPEED_HZ);
		return -EINVAL;
	}

	mutex_init(&devrec->buffer_mutex);
	init_completion(&devrec->tx_complete);

	ret = mrf24j40_hw_init(devrec);
	if (ret)
		goto err_register_device;

	mrf24j40_phy_setup(devrec);

	ret = devm_request_threaded_irq(&spi->dev,
					spi->irq,
					NULL,
					mrf24j40_isr,
					IRQF_TRIGGER_LOW|IRQF_ONESHOT,
					dev_name(&spi->dev),
					devrec);

	if (ret) {
		dev_err(printdev(devrec), "Unable to get IRQ");
		goto err_register_device;
	}

	dev_dbg(printdev(devrec), "registered mrf24j40\n");
	ret = ieee802154_register_hw(devrec->hw);
	if (ret)
		goto err_register_device;

	return 0;

err_register_device:
	ieee802154_free_hw(devrec->hw);
err_ret:
	return ret;
}

static int mrf24j40_remove(struct spi_device *spi)
{
	struct mrf24j40 *devrec = spi_get_drvdata(spi);

	dev_dbg(printdev(devrec), "remove\n");

	ieee802154_unregister_hw(devrec->hw);
	ieee802154_free_hw(devrec->hw);
	/* TODO: Will ieee802154_free_device() wait until ->xmit() is
	 * complete? */

	return 0;
}

static const struct of_device_id mrf24j40_of_match[] = {
	{ .compatible = "microchip,mrf24j40", .data = (void *)MRF24J40 },
	{ .compatible = "microchip,mrf24j40ma", .data = (void *)MRF24J40MA },
	{ .compatible = "microchip,mrf24j40mc", .data = (void *)MRF24J40MC },
	{ },
};
MODULE_DEVICE_TABLE(of, mrf24j40_of_match);

static const struct spi_device_id mrf24j40_ids[] = {
	{ "mrf24j40", MRF24J40 },
	{ "mrf24j40ma", MRF24J40MA },
	{ "mrf24j40mc", MRF24J40MC },
	{ },
};
MODULE_DEVICE_TABLE(spi, mrf24j40_ids);

static struct spi_driver mrf24j40_driver = {
	.driver = {
		.of_match_table = of_match_ptr(mrf24j40_of_match),
		.name = "mrf24j40",
		.owner = THIS_MODULE,
	},
	.id_table = mrf24j40_ids,
	.probe = mrf24j40_probe,
	.remove = mrf24j40_remove,
};

module_spi_driver(mrf24j40_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alan Ott");
MODULE_DESCRIPTION("MRF24J40 SPI 802.15.4 Controller Driver");
