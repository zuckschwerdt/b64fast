-module(b64fast).
-export([encode64/1, decode64/1]).
-on_load(init/0).

% The name of the application we're writing. This is the name
% used for the Erlang .app file.

-define(APPNAME, ?MODULE).

% The name of the shared library we're going to load the NIF
% code from. Defined in rebar.config as so_name.

-define(LIBNAME, ?MODULE).

% We could fall back to pure Erlang Base64 functions here.
% Don't hide platform load errors for now, though.

encode64(Bin) when is_binary(Bin) ->
    not_loaded(?LINE).

decode64(Bin) when is_binary(Bin) ->
    not_loaded(?LINE).

% Since we used init/0 in our -on_load() preprocessor directive, this
% function will get called as the module is loaded. This is the perfect
% place to load up our NIF shared library. Handily, the response of
% erlang:load_nif/2 matches the return specification for -on_load()
% functions.

init() ->
    SoName = case code:priv_dir(?APPNAME) of
        {error, bad_name} ->
            case filelib:is_dir(filename:join(["..", priv])) of
                true ->
                    filename:join(["..", priv, ?LIBNAME]);
                _ ->
                    filename:join([priv, ?LIBNAME])
            end;
        Dir ->
            filename:join(Dir, ?LIBNAME)
    end,
    erlang:load_nif(SoName, 0).

% This is just a simple place holder. It mostly shouldn't ever be called
% unless there was an unexpected error loading the NIF shared library.

not_loaded(Line) ->
    erlang:nif_error({not_loaded, [{module, ?MODULE}, {line, Line}]}).
