Fast Base64 encoding/decoding NIF for Erlang
============================================

This NIF complements Erlang with a fast and optimized way of encoding and
decoding bulk Base64 data.
Compliant with RFC4648 - The Base16, Base32, and Base64 Data Encodings.

Erlang is not well suited for fast sequential processing (s.a. Type B in
http://jlouisramblings.blogspot.de/2013/07/problematic-traits-in-erlang.html).

The NIF is well behaved and won't block the Erlang scheduler. It also doesn't
need a dirty scheduler by breaking large amounts of data into chunks to limit
the processing time to short timeslices. Big thanks to Steve Vinoski for making
this easy (see https://github.com/vinoski/bitwise).

Please note that the decoding doesn't handle whitespace, yet.

Use an erl shell to quickly measure Base64 speed in native Erlang.

```erlang
Data = binary:copy(<<"0123456789">>, 100000). % Create 1 MiB of data

{Elapsed, Enc} = timer:tc(base64, encode, [Data]).
Throughput_Encode = byte_size(Data) / Elapsed. % Throughput in MiB/s

{Elapsed2, Dec} = timer:tc(base64, decode, [Enc]).
Throughput_Decode = byte_size(Enc) / Elapsed2. % Throughput in MiB/s
```

And now try the (naive) NIF variant.

```erlang
Data = binary:copy(<<"0123456789">>, 1000000). % 10 MiB of data

{Elapsed, Enc} = timer:tc(b64fast, encode64, [Data]).
Throughput_Encode = byte_size(Data) / Elapsed. % Throughput in MiB/s

{Elapsed2, Dec} = timer:tc(b64fast, decode64, [Enc]).
Throughput_Decode = byte_size(Enc) / Elapsed2. % Throughput in MiB/s
```

Quick comparison gives a speedup of x24 on encode and x23 on decode.
