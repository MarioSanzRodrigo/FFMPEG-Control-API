/*
 * Copyright (c) 2017 Rafael Antoniello
 *
 * This file is part of MediaProcessors.
 *
 * MediaProcessors is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MediaProcessors is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with MediaProcessors. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file audio_settings.c
 * @author Rafael Antoniello
 */

#include "audio_settings.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <libcjson/cJSON.h>
#include <libmediaprocsutils/uri_parser.h>
#include <libmediaprocsutils/log.h>
#include <libmediaprocsutils/stat_codes.h>
#include <libmediaprocsutils/check_utils.h>
#include <libmediaprocs/proc_if.h>

audio_settings_enc_ctx_t* audio_settings_enc_ctx_allocate()
{
	return (audio_settings_enc_ctx_t*)calloc(1, sizeof(
			audio_settings_enc_ctx_t));
}

void audio_settings_enc_ctx_release(
		audio_settings_enc_ctx_t **ref_audio_settings_enc_ctx)
{
	audio_settings_enc_ctx_t *audio_settings_enc_ctx= NULL;

	if(ref_audio_settings_enc_ctx== NULL)
		return;

	if((audio_settings_enc_ctx= *ref_audio_settings_enc_ctx)!= NULL) {

		free(audio_settings_enc_ctx);
		*ref_audio_settings_enc_ctx= NULL;
	}
}

int audio_settings_enc_ctx_init(
		volatile audio_settings_enc_ctx_t *audio_settings_enc_ctx)
{
	LOG_CTX_INIT(NULL);

	/* Check arguments */
	CHECK_DO(audio_settings_enc_ctx!= NULL, return STAT_ERROR);

	audio_settings_enc_ctx->bit_rate_output= 64000;
	audio_settings_enc_ctx->sample_rate_output= 44100;

	return STAT_SUCCESS;
}

void audio_settings_enc_ctx_deinit(
		volatile audio_settings_enc_ctx_t *audio_settings_enc_ctx)
{
	// Reserved for future use
	// Release here heap-allocated members of the structure.
}

int audio_settings_enc_ctx_cpy(
		const audio_settings_enc_ctx_t *audio_settings_enc_ctx_src,
		audio_settings_enc_ctx_t *audio_settings_enc_ctx_dst)
{
	LOG_CTX_INIT(NULL);

	/* Check arguments */
	CHECK_DO(audio_settings_enc_ctx_src!= NULL, return STAT_ERROR);
	CHECK_DO(audio_settings_enc_ctx_dst!= NULL, return STAT_ERROR);

	/* Copy simple variable values */
	memcpy(audio_settings_enc_ctx_dst, audio_settings_enc_ctx_src,
			sizeof(audio_settings_enc_ctx_t));

	// Future use: duplicate heap-allocated variables...

	return STAT_SUCCESS;
}

int audio_settings_enc_ctx_restful_put(
		volatile audio_settings_enc_ctx_t *audio_settings_enc_ctx,
		const char *str, log_ctx_t *log_ctx)
{
	int end_code= STAT_ERROR;
	int flag_is_query= 0; // 0-> JSON / 1->query string
	cJSON *cjson_rest= NULL, *cjson_aux= NULL;
	char *bit_rate_output_str= NULL, *sample_rate_output_str= NULL;
	LOG_CTX_INIT(log_ctx);

	/* Check arguments */
	CHECK_DO(audio_settings_enc_ctx!= NULL, return STAT_ERROR);
	CHECK_DO(str!= NULL, return STAT_EINVAL);

	/* Guess string representation format (JSON-REST or Query) */
	//LOGV("'%s'\n", str); //comment-me
	flag_is_query= (str[0]=='{' && str[strlen(str)-1]=='}')? 0: 1;

	/* **** Parse RESTful string to get settings parameters **** */

	if(flag_is_query== 1) {

		/* 'bit_rate_output' */
		bit_rate_output_str= uri_parser_query_str_get_value("bit_rate_output",
				str);
		if(bit_rate_output_str!= NULL)
			audio_settings_enc_ctx->bit_rate_output= atoll(bit_rate_output_str);

		/* 'sample_rate_output' */
		sample_rate_output_str= uri_parser_query_str_get_value(
				"sample_rate_output", str);
		if(sample_rate_output_str!= NULL)
			audio_settings_enc_ctx->sample_rate_output=
					atoll(sample_rate_output_str);

	} else {

		/* In the case string format is JSON-REST, parse to cJSON structure */
		cjson_rest= cJSON_Parse(str);
		CHECK_DO(cjson_rest!= NULL, goto end);

		/* 'bit_rate_output' */
		cjson_aux= cJSON_GetObjectItem(cjson_rest, "bit_rate_output");
		if(cjson_aux!= NULL)
			audio_settings_enc_ctx->bit_rate_output= cjson_aux->valuedouble;

		/* 'sample_rate_output' */
		cjson_aux= cJSON_GetObjectItem(cjson_rest, "sample_rate_output");
		if(cjson_aux!= NULL)
			audio_settings_enc_ctx->sample_rate_output= cjson_aux->valuedouble;

	}

	end_code= STAT_SUCCESS;
end:
	if(cjson_rest!= NULL)
		cJSON_Delete(cjson_rest);
	if(bit_rate_output_str!= NULL)
		free(bit_rate_output_str);
	if(sample_rate_output_str!= NULL)
		free(sample_rate_output_str);
	return end_code;
}

int audio_settings_enc_ctx_restful_get(
		volatile audio_settings_enc_ctx_t *audio_settings_enc_ctx,
		cJSON **ref_cjson_rest, log_ctx_t *log_ctx)
{
	int end_code= STAT_ERROR;
	cJSON *cjson_rest= NULL, *cjson_aux= NULL;
	LOG_CTX_INIT(log_ctx);

	CHECK_DO(audio_settings_enc_ctx!= NULL, goto end);
	CHECK_DO(ref_cjson_rest!= NULL, goto end);

	*ref_cjson_rest= NULL;

	/* Create cJSON tree-root object */
	cjson_rest= cJSON_CreateObject();
	CHECK_DO(cjson_rest!= NULL, goto end);

	/* JSON string to be returned:
	 * {
	 *     "bit_rate_output":number,
	 *     "sample_rate_output":number
	 * }
	 */

	/* 'bit_rate_output' */
	cjson_aux= cJSON_CreateNumber((double)
			audio_settings_enc_ctx->bit_rate_output);
	CHECK_DO(cjson_aux!= NULL, goto end);
	cJSON_AddItemToObject(cjson_rest, "bit_rate_output", cjson_aux);

	/* 'sample_rate_output' */
	cjson_aux= cJSON_CreateNumber((double)
			audio_settings_enc_ctx->sample_rate_output);
	CHECK_DO(cjson_aux!= NULL, goto end);
	cJSON_AddItemToObject(cjson_rest, "sample_rate_output", cjson_aux);

	*ref_cjson_rest= cjson_rest;
	cjson_rest= NULL;
	end_code= STAT_SUCCESS;
end:
	if(cjson_rest!= NULL)
		cJSON_Delete(cjson_rest);
	return end_code;
}

audio_settings_dec_ctx_t* audio_settings_dec_ctx_allocate()
{
	return (audio_settings_dec_ctx_t*)calloc(1, sizeof(
			audio_settings_dec_ctx_t));
}

void audio_settings_dec_ctx_release(
		audio_settings_dec_ctx_t **ref_audio_settings_dec_ctx)
{
	audio_settings_dec_ctx_t *audio_settings_dec_ctx= NULL;

	if(ref_audio_settings_dec_ctx== NULL)
		return;

	if((audio_settings_dec_ctx= *ref_audio_settings_dec_ctx)!= NULL) {

		free(audio_settings_dec_ctx);
		*ref_audio_settings_dec_ctx= NULL;
	}
}

int audio_settings_dec_ctx_init(
		volatile audio_settings_dec_ctx_t *audio_settings_dec_ctx)
{
	LOG_CTX_INIT(NULL);

	/* Check arguments */
	CHECK_DO(audio_settings_dec_ctx!= NULL, return STAT_ERROR);

	// Reserved for future use
	// Initialize here structure members, for example:
	// audio_settings_dec_ctx->varX= valueX;

	return STAT_SUCCESS;
}

void audio_settings_dec_ctx_deinit(
		volatile audio_settings_dec_ctx_t *audio_settings_dec_ctx)
{
	// Reserved for future use
	// Release here heap-allocated members of the structure.
}

int audio_settings_dec_ctx_cpy(
		const audio_settings_dec_ctx_t *audio_settings_dec_ctx_src,
		audio_settings_dec_ctx_t *audio_settings_dec_ctx_dst)
{
	LOG_CTX_INIT(NULL);

	/* Check arguments */
	CHECK_DO(audio_settings_dec_ctx_src!= NULL, return STAT_ERROR);
	CHECK_DO(audio_settings_dec_ctx_dst!= NULL, return STAT_ERROR);

	// Reserved for future use
	// Copy values of simple variables, duplicate heap-allocated variables.

	return STAT_SUCCESS;
}

int audio_settings_dec_ctx_restful_put(
		volatile audio_settings_dec_ctx_t *audio_settings_dec_ctx,
		const char *str, log_ctx_t *log_ctx)
{
	int end_code= STAT_ERROR;
	int flag_is_query= 0; // 0-> JSON / 1->query string
	cJSON *cjson_rest= NULL;
	LOG_CTX_INIT(log_ctx);

	/* Check arguments */
	CHECK_DO(audio_settings_dec_ctx!= NULL, return STAT_ERROR);
	CHECK_DO(str!= NULL, return STAT_EINVAL);

	/* Guess string representation format (JSON-REST or Query) */
	//LOGV("'%s'\n", str); //comment-me
	flag_is_query= (str[0]=='{' && str[strlen(str)-1]=='}')? 0: 1;

	/* **** Parse RESTful string to get settings parameters **** */

	if(flag_is_query== 1) {

		/* Reserved for future use
		 *
		 * Example: If 'var1' is a number value passed as query-string
		 * '...var1_name=number&...':
		 *
		 * char *var1_str= NULL;
		 * ...
		 * var1_str= uri_parser_query_str_get_value("var1_name", str);
		 * if(var1_str!= NULL)
		 *     audio_settings_dec_ctx->var1= atoll(var1_str);
		 * ...
		 * if(var1_str!= NULL)
		 *     free(var1_str);
		 */

	} else {

		/* In the case string format is JSON-REST, parse to cJSON structure */
		cjson_rest= cJSON_Parse(str);
		CHECK_DO(cjson_rest!= NULL, goto end);

		/* Reserved for future use
		 *
		 * Example: If 'var1' is a number value passed as JSON-string
		 * '{..., "var1_name":number, ...}':
		 *
		 * *cjson_aux= NULL;
		 * ...
		 * cjson_aux= cJSON_GetObjectItem(cjson_rest, "var1_name");
		 * if(cjson_aux!= NULL)
		 *     audio_settings_dec_ctx->var1= cjson_aux->valuedouble;
		 */
	}

	end_code= STAT_SUCCESS;
end:
	if(cjson_rest!= NULL)
		cJSON_Delete(cjson_rest);
	return end_code;
}

int audio_settings_dec_ctx_restful_get(
		volatile audio_settings_dec_ctx_t *audio_settings_dec_ctx,
		cJSON **ref_cjson_rest, log_ctx_t *log_ctx)
{
	int end_code= STAT_ERROR;
	cJSON *cjson_rest= NULL;
	LOG_CTX_INIT(log_ctx);

	CHECK_DO(audio_settings_dec_ctx!= NULL, goto end);
	CHECK_DO(ref_cjson_rest!= NULL, goto end);

	*ref_cjson_rest= NULL;

	/* Create cJSON tree-root object */
	cjson_rest= cJSON_CreateObject();
	CHECK_DO(cjson_rest!= NULL, goto end);

	/* JSON string to be returned:
	 * {
	 *     //Reserved for future use: add settings to the REST string
	 *     // e.g.: "var1_name":number, ...
	 * }
	 */

	/* Reserved for future use
	 *
	 * Example: If 'var1' is a number value:
	 *
	 * *cjson_aux= NULL;
	 * ...
	 * 	cjson_aux= cJSON_CreateNumber((double)audio_settings_dec_ctx->var1);
	 * 	CHECK_DO(cjson_aux!= NULL, goto end);
	 * 	cJSON_AddItemToObject(cjson_rest, "var1_name", cjson_aux);
	 */

	*ref_cjson_rest= cjson_rest;
	cjson_rest= NULL;
	end_code= STAT_SUCCESS;
end:
	if(cjson_rest!= NULL)
		cJSON_Delete(cjson_rest);
	return end_code;
}
