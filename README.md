Fast Base64 encoding/decoding NIF for Erlang
============================================

RFC4648 - The Base16, Base32, and Base64 Data Encodings


Use an erl shell to quickly measure Base64 speed in native Erlang.

```
Data = binary:copy(<<"0123456789">>, 100000). % Create 1 MiB of data

{Elapsed, Enc} = timer:tc(base64, encode, [Data]).
Throughput_Encode = byte_size(Data) / Elapsed. % Throughput in MiB/s

{Elapsed2, Dec} = timer:tc(base64, decode, [Enc]).
Throughput_Decode = byte_size(Enc) / Elapsed2. % Throughput in MiB/s
```

And now try the (naive) NIF variant.

```
Data = binary:copy(<<"0123456789">>, 1000000). % 10 MiB of data

{Elapsed, Enc} = timer:tc(b64fast, encode64, [Data]).
Throughput_Encode = byte_size(Data) / Elapsed. % Throughput in MiB/s

{Elapsed2, Dec} = timer:tc(b64fast, decode64, [Enc]).
Throughput_Decode = byte_size(Enc) / Elapsed2. % Throughput in MiB/s
```

Quick comparison gives a speedup of x24 on encode and x23 on decode.
