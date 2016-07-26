#include <sys/time.h>
#include "erl_nif.h"

#include "naive.h"

/*
 * decode64_chunk is an "internal NIF" scheduled by decode64 below. It takes
 * the binary argument, same as the other functions here, but also
 * takes a count of the max number of bytes to process per timeslice, the
 * offset into the binary at which to start processing, the resource type
 * holding the resulting data, and it's size.
 */
static ERL_NIF_TERM
decode64_chunk(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    ErlNifResourceType* res_type = (ErlNifResourceType*)enif_priv_data(env);
    unsigned long offset, i, end, max_per_slice, res_size;
    struct timeval start, stop, slice;
    int pct, total = 0;
    ERL_NIF_TERM newargv[5];
    ErlNifBinary bin;
    void* res;

    if (argc != 5 || !enif_inspect_binary(env, argv[0], &bin) ||
        !enif_get_ulong(env, argv[1], &max_per_slice) ||
        !enif_get_ulong(env, argv[2], &offset) ||
        !enif_get_resource(env, argv[3], res_type, &res) ||
        !enif_get_ulong(env, argv[4], &res_size))
        return enif_make_badarg(env);
    end = offset + max_per_slice;
    if (end > bin.size) end = bin.size;
    i = offset;
    while (i < bin.size) {
        gettimeofday(&start, NULL);
        unbase64(bin.data+i, end-i, res+i*3/4, res_size-i*3/4);
        i = end;
        if (i == bin.size) break;
        gettimeofday(&stop, NULL);
        /* determine how much of the timeslice was used */
        timersub(&stop, &start, &slice);
        pct = (int)((slice.tv_sec*1000000+slice.tv_usec)/10);
        total += pct;
        if (pct > 100) pct = 100;
        else if (pct == 0) pct = 1;
        if (enif_consume_timeslice(env, pct)) {
            /* the timeslice has been used up, so adjust our max_per_slice byte count based on
             * the processing we've done, then reschedule to run again */
            max_per_slice = i - offset;
            if (total > 100) {
                int m = (int)(total/100);
                if (m == 1)
                    max_per_slice -= (unsigned long)(max_per_slice*(total-100)/100);
                else
                    max_per_slice = (unsigned long)(max_per_slice/m);
            }
            max_per_slice = max_per_slice / 4;
            max_per_slice = max_per_slice * 4;
            newargv[0] = argv[0];
            newargv[1] = enif_make_ulong(env, max_per_slice);
            newargv[2] = enif_make_ulong(env, i);
            newargv[3] = argv[3];
            newargv[4] = argv[4];
            return enif_schedule_nif(env, "decode64_chunk", 0, decode64_chunk, argc, newargv);
        }
        end += max_per_slice;
        if (end > bin.size) end = bin.size;
    }
    return enif_make_resource_binary(env, res, res, res_size);
}

/*
 * decode64 just schedules decode64_chunk for execution, providing an initial
 * guess of 30KB for the max number of bytes to process before yielding the
 * scheduler thread.
 */
static ERL_NIF_TERM
decode64(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    ErlNifResourceType* res_type = (ErlNifResourceType*)enif_priv_data(env);
    ERL_NIF_TERM newargv[5];
    ErlNifBinary bin;
    unsigned res_size;
    void* res;

    if (argc != 1 || !enif_inspect_binary(env, argv[0], &bin))
        return enif_make_badarg(env);
    if (bin.size == 0)
        return argv[0];
    res_size = unbase64_size(bin.data, bin.size);
    newargv[0] = argv[0];
    newargv[1] = enif_make_ulong(env, 30720); // MOD4
    newargv[2] = enif_make_ulong(env, 0);
    res = enif_alloc_resource(res_type, res_size);
    newargv[3] = enif_make_resource(env, res);
    newargv[4] = enif_make_ulong(env, res_size);
    enif_release_resource(res);
    return enif_schedule_nif(env, "decode64_chunk", 0, decode64_chunk, 5, newargv);
}

/*
 * encode64_chunk is an "internal NIF" scheduled by encode64 below. It takes
 * the binary argument, same as the other functions here, but also
 * takes a count of the max number of bytes to process per timeslice, the
 * offset into the binary at which to start processing, the resource type
 * holding the resulting data, and it's size.
 */
static ERL_NIF_TERM
encode64_chunk(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    ErlNifResourceType* res_type = (ErlNifResourceType*)enif_priv_data(env);
    unsigned long offset, i, end, max_per_slice, res_size;
    struct timeval start, stop, slice;
    int pct, total = 0;
    ERL_NIF_TERM newargv[5];
    ErlNifBinary bin;
    void* res;

    if (argc != 5 || !enif_inspect_binary(env, argv[0], &bin) ||
        !enif_get_ulong(env, argv[1], &max_per_slice) ||
        !enif_get_ulong(env, argv[2], &offset) ||
        !enif_get_resource(env, argv[3], res_type, &res) ||
        !enif_get_ulong(env, argv[4], &res_size))
        return enif_make_badarg(env);
    end = offset + max_per_slice;
    if (end > bin.size) end = bin.size;
    i = offset;
    while (i < bin.size) {
        gettimeofday(&start, NULL);
        base64(bin.data+i, end-i, res+i*4/3, res_size-i*4/3);
        i = end;
        if (i == bin.size) break;
        gettimeofday(&stop, NULL);
        /* determine how much of the timeslice was used */
        timersub(&stop, &start, &slice);
        pct = (int)((slice.tv_sec*1000000+slice.tv_usec)/10);
        total += pct;
        if (pct > 100) pct = 100;
        else if (pct == 0) pct = 1;
        if (enif_consume_timeslice(env, pct)) {
            /* the timeslice has been used up, so adjust our max_per_slice byte count based on
             * the processing we've done, then reschedule to run again */
            max_per_slice = i - offset;
            if (total > 100) {
                int m = (int)(total/100);
                if (m == 1)
                    max_per_slice -= (unsigned long)(max_per_slice*(total-100)/100);
                else
                    max_per_slice = (unsigned long)(max_per_slice/m);
            }
            max_per_slice = max_per_slice / 3;
            max_per_slice = max_per_slice * 3;
            newargv[0] = argv[0];
            newargv[1] = enif_make_ulong(env, max_per_slice);
            newargv[2] = enif_make_ulong(env, i);
            newargv[3] = argv[3];
            newargv[4] = argv[4];
            return enif_schedule_nif(env, "encode64_chunk", 0, encode64_chunk, argc, newargv);
        }
        end += max_per_slice;
        if (end > bin.size) end = bin.size;
    }
    return enif_make_resource_binary(env, res, res, res_size);
}

/*
 * encode64 just schedules encode64_chunk for execution, providing an initial
 * guess of 30KB for the max number of bytes to process before yielding the
 * scheduler thread.
 */
static ERL_NIF_TERM
encode64(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    ErlNifResourceType* res_type = (ErlNifResourceType*)enif_priv_data(env);
    ERL_NIF_TERM newargv[5];
    ErlNifBinary bin;
    unsigned res_size;
    void* res;

    if (argc != 1 || !enif_inspect_binary(env, argv[0], &bin))
        return enif_make_badarg(env);
    if (bin.size == 0)
        return argv[0];
    res_size = base64_size(bin.size);
    newargv[0] = argv[0];
    newargv[1] = enif_make_ulong(env, 30720); // MOD3
    newargv[2] = enif_make_ulong(env, 0);
    res = enif_alloc_resource(res_type, res_size);
    newargv[3] = enif_make_resource(env, res);
    newargv[4] = enif_make_ulong(env, res_size);
    enif_release_resource(res);
    return enif_schedule_nif(env, "encode64_chunk", 0, encode64_chunk, 5, newargv);
}

static int
nifload(ErlNifEnv* env, void** priv_data, ERL_NIF_TERM load_info)
{
    *priv_data = enif_open_resource_type(env,
                                         NULL,
                                         "b64fast",
                                         NULL,
                                         ERL_NIF_RT_CREATE|ERL_NIF_RT_TAKEOVER,
                                         NULL);
    return *priv_data == NULL;
}

static int
nifupgrade(ErlNifEnv* env, void** priv_data, void** old_priv_data, ERL_NIF_TERM load_info)
{
    *priv_data = enif_open_resource_type(env,
                                         NULL,
                                         "b64fast",
                                         NULL,
                                         ERL_NIF_RT_TAKEOVER,
                                         NULL);
    return *priv_data == NULL;
}

static ErlNifFunc funcs[] = {
    {"encode64", 1, encode64},
    {"decode64", 1, decode64},
};
ERL_NIF_INIT(b64fast, funcs, nifload, NULL, nifupgrade, NULL);
