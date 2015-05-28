/*
 * AEAD: Authenticated Encryption with Associated Data
 * 
 * Copyright (c) 2007 Herbert Xu <herbert@gondor.apana.org.au>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option) 
 * any later version.
 *
 */

#ifndef _CRYPTO_INTERNAL_AEAD_H
#define _CRYPTO_INTERNAL_AEAD_H

#include <crypto/aead.h>
#include <crypto/algapi.h>
#include <linux/types.h>

struct rtattr;

struct aead_instance {
	struct aead_alg alg;
};

struct crypto_aead_spawn {
	struct crypto_spawn base;
};

extern const struct crypto_type crypto_aead_type;
extern const struct crypto_type crypto_nivaead_type;

static inline void *crypto_aead_ctx(struct crypto_aead *tfm)
{
	return crypto_tfm_ctx(&tfm->base);
}

static inline struct crypto_instance *crypto_aead_alg_instance(
	struct crypto_aead *aead)
{
	return crypto_tfm_alg_instance(&aead->base);
}

static inline struct crypto_instance *aead_crypto_instance(
	struct aead_instance *inst)
{
	return container_of(&inst->alg.base, struct crypto_instance, alg);
}

static inline struct aead_instance *aead_instance(struct crypto_instance *inst)
{
	return container_of(&inst->alg, struct aead_instance, alg.base);
}

static inline struct aead_instance *aead_alg_instance(struct crypto_aead *aead)
{
	return aead_instance(crypto_aead_alg_instance(aead));
}

static inline void *aead_instance_ctx(struct aead_instance *inst)
{
	return crypto_instance_ctx(aead_crypto_instance(inst));
}

static inline void *aead_request_ctx(struct aead_request *req)
{
	return req->__ctx;
}

static inline void aead_request_complete(struct aead_request *req, int err)
{
	req->base.complete(&req->base, err);
}

static inline u32 aead_request_flags(struct aead_request *req)
{
	return req->base.flags;
}

static inline void crypto_set_aead_spawn(
	struct crypto_aead_spawn *spawn, struct crypto_instance *inst)
{
	crypto_set_spawn(&spawn->base, inst);
}

struct crypto_alg *crypto_lookup_aead(const char *name, u32 type, u32 mask);

int crypto_grab_aead(struct crypto_aead_spawn *spawn, const char *name,
		     u32 type, u32 mask);

static inline void crypto_drop_aead(struct crypto_aead_spawn *spawn)
{
	crypto_drop_spawn(&spawn->base);
}

static inline struct crypto_alg *crypto_aead_spawn_alg(
	struct crypto_aead_spawn *spawn)
{
	return spawn->base.alg;
}

static inline struct aead_alg *crypto_spawn_aead_alg(
	struct crypto_aead_spawn *spawn)
{
	return container_of(spawn->base.alg, struct aead_alg, base);
}

static inline struct crypto_aead *crypto_spawn_aead(
	struct crypto_aead_spawn *spawn)
{
	return crypto_spawn_tfm2(&spawn->base);
}

struct aead_instance *aead_geniv_alloc(struct crypto_template *tmpl,
				       struct rtattr **tb, u32 type, u32 mask);
void aead_geniv_free(struct aead_instance *inst);
int aead_geniv_init(struct crypto_tfm *tfm);
void aead_geniv_exit(struct crypto_tfm *tfm);

static inline struct crypto_aead *aead_geniv_base(struct crypto_aead *geniv)
{
	return geniv->child;
}

static inline void *aead_givcrypt_reqctx(struct aead_givcrypt_request *req)
{
	return aead_request_ctx(&req->areq);
}

static inline void aead_givcrypt_complete(struct aead_givcrypt_request *req,
					  int err)
{
	aead_request_complete(&req->areq, err);
}

static inline void crypto_aead_set_reqsize(struct crypto_aead *aead,
					   unsigned int reqsize)
{
	crypto_aead_crt(aead)->reqsize = reqsize;
}

static inline unsigned int crypto_aead_alg_maxauthsize(struct aead_alg *alg)
{
	return alg->base.cra_aead.encrypt ? alg->base.cra_aead.maxauthsize :
					    alg->maxauthsize;
}

static inline unsigned int crypto_aead_maxauthsize(struct crypto_aead *aead)
{
	return crypto_aead_alg_maxauthsize(crypto_aead_alg(aead));
}

int crypto_register_aead(struct aead_alg *alg);
int crypto_unregister_aead(struct aead_alg *alg);
int aead_register_instance(struct crypto_template *tmpl,
			   struct aead_instance *inst);

#endif	/* _CRYPTO_INTERNAL_AEAD_H */

