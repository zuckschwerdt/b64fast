-module(b64fast_tests).

-include_lib("eunit/include/eunit.hrl").

bad_encode_test() ->
    ?assertException(error, badarg, b64fast:encode64( 42    )),
    ?assertException(error, badarg, b64fast:encode64( foo   )),
    ?assertException(error, badarg, b64fast:encode64( "foo" )),
    ?assertException(error, badarg, b64fast:encode64( {foo} )).

bad_decode_test() ->
    ?assertException(error, badarg, b64fast:decode64( 42    )),
    ?assertException(error, badarg, b64fast:decode64( foo   )),
    ?assertException(error, badarg, b64fast:decode64( "foo" )),
    ?assertException(error, badarg, b64fast:decode64( {foo} )).

encode_test() ->
    ?assert(b64fast:encode64(<< "zany" >>) =:= << "emFueQ==" >>),
    ?assert(b64fast:encode64(<< "zan"  >>) =:= << "emFu"     >>),
    ?assert(b64fast:encode64(<< "za"   >>) =:= << "emE="     >>),
    ?assert(b64fast:encode64(<< "z"    >>) =:= << "eg=="     >>),
    ?assert(b64fast:encode64(<<        >>) =:= <<            >>).

decode_test() ->
    ?assert(b64fast:decode64(<< "emFueQ==" >>) =:= << "zany" >>),
    ?assert(b64fast:decode64(<< "emFu"     >>) =:= << "zan"  >>),
    ?assert(b64fast:decode64(<< "emE="     >>) =:= << "za"   >>),
    ?assert(b64fast:decode64(<< "eg=="     >>) =:= << "z"    >>),
    ?assert(b64fast:decode64(<<            >>) =:= <<        >>).

% TODO: skip whitespace
%padded_decode_test() ->
%    ?assert(b64fast:decode64(<< " emFu" >>) =:= << "zan" >>),
%    ?assert(b64fast:decode64(<< "em Fu" >>) =:= << "zan" >>),
%    ?assert(b64fast:decode64(<< "emFu " >>) =:= << "zan" >>),
%    ?assert(b64fast:decode64(<< "    "  >>) =:= <<       >>),
%    ?assert(b64fast:decode64(<< "   ="  >>) =:= <<       >>),
%    ?assert(b64fast:decode64(<< "  =="  >>) =:= <<       >>),
%    ?assert(b64fast:decode64(<< "=   "  >>) =:= <<       >>),
%    ?assert(b64fast:decode64(<< "==  "  >>) =:= <<       >>).

truncated_decode_test() ->
    ?assert(b64fast:decode64(<< "AAAA" >>) =:= << 0,0,0 >>),
    ?assert(b64fast:decode64(<< "AAA=" >>) =:= << 0,0   >>),
    ?assert(b64fast:decode64(<< "AAA"  >>) =:= << 0,0   >>),
    ?assert(b64fast:decode64(<< "AA==" >>) =:= << 0     >>),
    ?assert(b64fast:decode64(<< "AA="  >>) =:= << 0     >>),
    ?assert(b64fast:decode64(<< "AA"   >>) =:= << 0     >>),
    ?assert(b64fast:decode64(<< "A=="  >>) =:= <<       >>),
    ?assert(b64fast:decode64(<< "A="   >>) =:= <<       >>),
    ?assert(b64fast:decode64(<< "A"    >>) =:= <<       >>),
    ?assert(b64fast:decode64(<< "=="   >>) =:= <<       >>),
    ?assert(b64fast:decode64(<< "="    >>) =:= <<       >>),
    ?assert(b64fast:decode64(<<        >>) =:= <<       >>).

backtoback_encode_test() ->
    Data = binary:copy(<<"0123456789">>, 100000), % 1 MiB of data
    ?assert(base64:encode(Data) =:= b64fast:encode64(Data)).

backtoback_decode_test() ->
    Data = binary:copy(<<"0123456789">>, 100000), % 1 MiB of data
%    Enc = b64fast:encode64(Data),
    Enc = base64:encode(Data),
    ?assert(base64:decode(Enc) =:= b64fast:decode64(Enc)).

speed_test() ->
    Data = binary:copy(<<"0123456789">>, 100000), % 1 MiB of data

    {Elapsed1, Enc1} = timer:tc(base64, encode, [Data]),
    io:fwrite(standard_error, "erlang encode ~B us ~f MiB/s~n",
      [Elapsed1, byte_size(Data) / Elapsed1]),

    {Elapsed2, _Dec1} = timer:tc(base64, decode, [Enc1]),
    io:fwrite(standard_error, "erlang decode ~B us ~f MiB/s~n",
      [Elapsed2, byte_size(Enc1) / Elapsed2]),

    {Elapsed3, Enc2} = timer:tc(b64fast, encode64, [Data]),
    io:fwrite(standard_error, "NIF encode ~B us ~f MiB/s~n",
      [Elapsed3, byte_size(Data) / Elapsed3]),

    {Elapsed4, _Dec2} = timer:tc(b64fast, decode64, [Enc2]),
    io:fwrite(standard_error, "NIF decode ~B us ~f MiB/s~n",
      [Elapsed4, byte_size(Enc2) / Elapsed4]).

speed10_test() ->
    Data = binary:copy(<<"0123456789">>, 1000000), % 10 MiB of data

    {Elapsed3, Enc2} = timer:tc(b64fast, encode64, [Data]),
    io:fwrite(standard_error, "NIF encode ~B us ~f MiB/s~n",
      [Elapsed3, byte_size(Data) / Elapsed3]),

    {Elapsed4, _Dec2} = timer:tc(b64fast, decode64, [Enc2]),
    io:fwrite(standard_error, "NIF decode ~B us ~f MiB/s~n",
      [Elapsed4, byte_size(Enc2) / Elapsed4]).

speed100_test() ->
    Data = binary:copy(<<"0123456789">>, 10000000), % 100 MiB of data

    {Elapsed3, Enc2} = timer:tc(b64fast, encode64, [Data]),
    io:fwrite(standard_error, "NIF encode ~B us ~f MiB/s~n",
      [Elapsed3, byte_size(Data) / Elapsed3]),

    {Elapsed4, _Dec2} = timer:tc(b64fast, decode64, [Enc2]),
    io:fwrite(standard_error, "NIF decode ~B us ~f MiB/s~n",
      [Elapsed4, byte_size(Enc2) / Elapsed4]).
