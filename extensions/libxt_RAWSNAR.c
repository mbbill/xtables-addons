/*
 *	"RAWNAT" target extension for iptables
 *	Copyright © Jan Engelhardt, 2008 - 2009
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License; either
 *	version 2 of the License, or any later version, as published by the
 *	Free Software Foundation.
 */
#include <netinet/in.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <xtables.h>
#include <linux/netfilter.h>
#include "xt_RAWNAT.h"
#include "compat_user.h"

enum {
	FLAGS_TO = 1 << 0,
};

static const struct option rawsnat_tg_opts[] = {
	{.name = "to-source", .has_arg = true, .val = 't'},
        {.name = "to-port",   .has_arg = true, .val = 'p'},
	{},
};

static void rawsnat_tg_help(void)
{
	printf(
"RAWSNAR target options:\n"
"    --to-source addr[/mask]    Address or network to map to\n"
"    --to-port               0 - random port;\n"
);
}

static int
rawsnat_tg4_parse(int c, char **argv, int invert, unsigned int *flags,
                  const void *entry, struct xt_entry_target **target)
{
	struct xt_rawnat_tginfo *info = (void *)(*target)->data;
	struct in_addr *a;
	unsigned int mask, port;
	char *end;

	switch (c) {
	case 't':
		info->mask = 32;
		end = strchr(optarg, '/');
		if (end != NULL) {
			*end++ = '\0';
			if (!xtables_strtoui(end, NULL, &mask, 0, 32))
				xtables_param_act(XTF_BAD_VALUE, "RAWSNAR",
					"--to-source", optarg);
			info->mask = mask;
		}
		a = xtables_numeric_to_ipaddr(optarg);
		if (a == NULL)
			xtables_param_act(XTF_BAD_VALUE, "RAWSNAR",
				"--to-source", optarg);
		memcpy(&info->addr.in, a, sizeof(*a));
		*flags |= FLAGS_TO;
                info->has_addr = 1;
		return true;
        case 'p':
                info->port = 0;
                if (!xtables_strtoui(optarg, NULL, &port, 0, 65535))
                        xtables_param_act(XTF_BAD_VALUE, "RAWSNAR", "--to-port", optarg);
                info->port = port;
                info->has_port = 1;
                return true;
	}
	return false;
}

static int
rawsnat_tg6_parse(int c, char **argv, int invert, unsigned int *flags,
                  const void *entry, struct xt_entry_target **target)
{
	struct xt_rawnat_tginfo *info = (void *)(*target)->data;
	struct in6_addr *a;
	unsigned int mask;
	char *end;

	switch (c) {
	case 't':
		info->mask = 128;
		end = strchr(optarg, '/');
		if (end != NULL) {
			*end++ = '\0';
			if (!xtables_strtoui(end, NULL, &mask, 0, 128))
				xtables_param_act(XTF_BAD_VALUE, "RAWSNAR",
					"--to-source", optarg);
			info->mask = mask;
		}
		a = xtables_numeric_to_ip6addr(optarg);
		if (a == NULL)
			xtables_param_act(XTF_BAD_VALUE, "RAWSNAR",
				"--to-source", optarg);
		memcpy(&info->addr.in6, a, sizeof(*a));
		*flags |= FLAGS_TO;
		return true;
	}
	return false;
}

static void rawsnat_tg_check(unsigned int flags)
{
}

static void
rawsnat_tg4_save(const void *entry, const struct xt_entry_target *target)
{
	const struct xt_rawnat_tginfo *info = (const void *)target->data;

	if (info->has_addr)
	   printf(" --to-source %s/%u ",
	       xtables_ipaddr_to_numeric(&info->addr.in),
	       info->mask);
        if (info->has_port)
           printf(" --to-port %u ", info->port);
}

static void
rawsnat_tg6_save(const void *entry, const struct xt_entry_target *target)
{
	const struct xt_rawnat_tginfo *info = (const void *)target->data;

	printf(" --to-source %s/%u ",
	       xtables_ip6addr_to_numeric(&info->addr.in6),
	       info->mask);
}

static void
rawsnat_tg4_print(const void *entry, const struct xt_entry_target *target,
                  int numeric)
{
	printf(" -j RAWSNAR");
	rawsnat_tg4_save(entry, target);
}

static void
rawsnat_tg6_print(const void *entry, const struct xt_entry_target *target,
                  int numeric)
{
	printf(" -j RAWSNAR");
	rawsnat_tg6_save(entry, target);
}

static struct xtables_target rawsnat_tg_reg[] = {
	{
		.version       = XTABLES_VERSION,
		.name          = "RAWSNAR",
		.revision      = 0,
		.family        = NFPROTO_IPV4,
		.size          = XT_ALIGN(sizeof(struct xt_rawnat_tginfo)),
		.userspacesize = XT_ALIGN(sizeof(struct xt_rawnat_tginfo)),
		.help          = rawsnat_tg_help,
		.parse         = rawsnat_tg4_parse,
		.final_check   = rawsnat_tg_check,
		.print         = rawsnat_tg4_print,
		.save          = rawsnat_tg4_save,
		.extra_opts    = rawsnat_tg_opts,
	},
	{
		.version       = XTABLES_VERSION,
		.name          = "RAWSNAR",
		.revision      = 0,
		.family        = NFPROTO_IPV6,
		.size          = XT_ALIGN(sizeof(struct xt_rawnat_tginfo)),
		.userspacesize = XT_ALIGN(sizeof(struct xt_rawnat_tginfo)),
		.help          = rawsnat_tg_help,
		.parse         = rawsnat_tg6_parse,
		.final_check   = rawsnat_tg_check,
		.print         = rawsnat_tg6_print,
		.save          = rawsnat_tg6_save,
		.extra_opts    = rawsnat_tg_opts,
	},
};

static void _init(void)
{
	xtables_register_targets(rawsnat_tg_reg,
		sizeof(rawsnat_tg_reg) / sizeof(*rawsnat_tg_reg));
}
