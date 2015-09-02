#include "erl_nif.h"

#include "naive.h"

static ERL_NIF_TERM
encode64(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    if(argc != 1)
    {
        return enif_make_badarg(env);
    }

    ErlNifBinary bin;
    if(!enif_inspect_binary(env, argv[0], &bin))
    {
        return enif_make_badarg(env);
    }

    unsigned int size = base64_size(bin.size);
    ERL_NIF_TERM term;
    unsigned char *buf = enif_make_new_binary(env, size, &term);

    base64(bin.data, bin.size, buf, size);
    return term;
}

static ERL_NIF_TERM
decode64(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    if(argc != 1)
    {
        return enif_make_badarg(env);
    }

    ErlNifBinary bin;
    if(!enif_inspect_binary(env, argv[0], &bin))
    {
        return enif_make_badarg(env);
    }

    unsigned int size = unbase64_size(bin.data, bin.size);
    ERL_NIF_TERM term;
    unsigned char *buf = enif_make_new_binary(env, size, &term);

    unbase64(bin.data, bin.size, buf, size);
    return term;
}

static ErlNifFunc nif_funcs[] = {
    {"encode64", 1, encode64},
    {"decode64", 1, decode64}
};

// Initialize this NIF library.
//
// Args: (MODULE, ErlNifFunc funcs[], load, reload, upgrade, unload)
// Docs: http://erlang.org/doc/man/erl_nif.html#ERL_NIF_INIT

ERL_NIF_INIT(b64fast, nif_funcs, NULL, NULL, NULL, NULL);
