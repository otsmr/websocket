


Connect with websocket in the terminal
```bash
cd cpp/corpus
cat con_small - | nc localhost 8080



# Run Autobahn|Fuzzing

RUST_LOG=debug cargo run --bin wsserver_autobah
docker run -it --rm --net=host \
  -v "${PWD}/tests:/config" \
  -v "${PWD}/tests/reports:/reports" \
  --name fuzzingclient \
  crossbario/autobahn-testsuite \
  wstest -m fuzzingclient --spec /config/fuzzingclient.json
